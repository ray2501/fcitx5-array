/*
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2, or
 any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.
*/

#include "engine.h"

#include <fcitx-config/iniparser.h>
#include <fcitx/candidatelist.h>
#include <fcitx/inputpanel.h>
#include <fcitx/instance.h>
#include <fcitx/userinterfacemanager.h>
#include <fmt/format.h>
#include <notifications_public.h>
#include <quickphrase_public.h>
#include <utility>

#define ARRAY_SHORT_CODE_EMPTY_STRING "⎔"

namespace Array {

const fcitx::KeyList &selectionKeys() {
    static const fcitx::KeyList selectionKeys{
        fcitx::Key{FcitxKey_1}, fcitx::Key{FcitxKey_2}, fcitx::Key{FcitxKey_3},
        fcitx::Key{FcitxKey_4}, fcitx::Key{FcitxKey_5}, fcitx::Key{FcitxKey_6},
        fcitx::Key{FcitxKey_7}, fcitx::Key{FcitxKey_8}, fcitx::Key{FcitxKey_9},
        fcitx::Key{FcitxKey_0}};
    return selectionKeys;
}

class ArrayCandidate : public fcitx::CandidateWord {
public:
    ArrayCandidate(ArrayEngine *engine, int idx, std::string text)
        : engine_(engine), idx_(idx) {
        setText(fcitx::Text(std::move(text)));
    }

    void select(fcitx::InputContext *inputContext) const override {
        std::string choiceword = text().toString();

        if (choiceword.compare(ARRAY_SHORT_CODE_EMPTY_STRING) != 0) {
            if (engine_->is_special_notify()) {
                auto ctx = engine_->context();
                auto *state = inputContext->propertyFor(engine_->factory());
                bool check_special = (ctx->get())
                                         ->input_key_is_not_special(
                                             state->getInput(), choiceword);

                if (check_special) {
                    std::vector<std::string> candidates =
                        (ctx->get())
                            ->get_reverted_key_candidates_from_special(
                                choiceword);
                    if (candidates.size() == 1) {
                        std::string keystr =
                            (ctx->get())->get_preedit_string(candidates[0]);
                        std::string msg =
                            fmt::format(_("{} : {}"), choiceword, keystr);

                        std::vector<std::string> actions = {"Ok", _("Ok")};
                        engine_->notifications()
                            ->call<fcitx::INotifications::sendNotification>(
                                _("fcitx5-array"), 0, "fcitx-ibusarray",
                                _("Notify special code"), msg, actions, 2500,
                                [this](const std::string &action) {
                                    FCITX_UNUSED(action);
                                },
                                nullptr);
                    }
                }
            }

            inputContext->commitString(choiceword);
        }

        auto state = inputContext->propertyFor(engine_->factory());
        state->reset();
    }

private:
    ArrayEngine *engine_;
    int idx_;
};

/*
 * ArrayState
 */

void ArrayState::keyEvent(fcitx::KeyEvent &event) {
    if (event.isRelease()) {
        return;
    }

    /*
     * Support Fcitx 5 QuickPhrase (triggered by hot key)
     */
    if (buffer_.empty()) {
        const auto &config = *(engine_->config());

        if (event.key().check(*config.quickphraseKey) &&
            engine_->quickphrase()) {
            engine_->quickphrase()->call<fcitx::IQuickPhrase::trigger>(
                ic_, "", "", "", "", fcitx::Key());

            event.filterAndAccept();
            return;
        }
    }

    if (event.key().isModifier() || event.key().hasModifier()) {
        return;
    }

    if (event.key().check(FcitxKey_space)) {
        if (buffer_.empty()) {
            return;
        } else {
            if (auto candidateList = ic_->inputPanel().candidateList()) {
                if (candidateList->toPageable()->totalPages() > 1) {
                    if (candidateList->toPageable()->hasNext()) {
                        candidateList->toPageable()->next();
                    } else {
                        candidateList->toPageable()->setPage(0);
                    }

                    ic_->updateUserInterface(
                        fcitx::UserInterfaceComponent::InputPanel);
                    return event.filterAndAccept();
                }
            }

            if (space_press_count == 0) {
                space_press_count++;

                updateUI();

                if (auto candidateList = ic_->inputPanel().candidateList()) {
                    if (candidateList->size() == 1) {
                        event.accept();
                        candidateList->candidate(0).select(ic_);
                    }
                }
            } else if (space_press_count == 1) {
                if (auto candidateList = ic_->inputPanel().candidateList()) {
                    event.accept();
                    candidateList->candidate(0).select(ic_);
                } else {
                    reset();
                }
            }

            return event.filterAndAccept();
        }
    }

    /*
     * For Array 30 phrase input key
     */
    if (event.key().check(FcitxKey_apostrophe)) {
        if (buffer_.empty()) {
            return;
        } else {
            updatePreedit();
            setPhraseLookupTable();
            return event.filterAndAccept();
        }
    }

    if (event.key().check(FcitxKey_BackSpace) ||
        event.key().check(FcitxKey_Delete)) {
        if (buffer_.empty()) {
            return;
        } else {
            std::string currentstr = buffer_.userInput();
            if (currentstr.back() == '?')
                wildcard_char_count--;

            buffer_.backspace();
            updateUI();
            return event.filterAndAccept();
        }
    }

    if (event.key().check(FcitxKey_Return)) {
        if (buffer_.empty()) {
            return;
        } else {
            return event.filterAndAccept();
        }
    }

    if (event.key().check(FcitxKey_Page_Up) || event.key().check(FcitxKey_Up) ||
        event.key().check(FcitxKey_Left)) {
        if (buffer_.empty()) {
            return;
        } else {
            if (auto candidateList = ic_->inputPanel().candidateList()) {
                if (candidateList->toPageable()->totalPages() > 1) {
                    if (candidateList->toPageable()->hasPrev()) {
                        candidateList->toPageable()->prev();
                    } else {
                        candidateList->toPageable()->setPage(
                            candidateList->toPageable()->totalPages() - 1);
                    }

                    ic_->updateUserInterface(
                        fcitx::UserInterfaceComponent::InputPanel);
                    return event.filterAndAccept();
                }
            }
        }
    }

    if (event.key().check(FcitxKey_Page_Down) ||
        event.key().check(FcitxKey_Down) || event.key().check(FcitxKey_Right)) {
        if (buffer_.empty()) {
            return;
        } else {
            if (auto candidateList = ic_->inputPanel().candidateList()) {
                if (candidateList->toPageable()->totalPages() > 1) {
                    if (candidateList->toPageable()->hasNext()) {
                        candidateList->toPageable()->next();
                    } else {
                        candidateList->toPageable()->setPage(0);
                    }

                    ic_->updateUserInterface(
                        fcitx::UserInterfaceComponent::InputPanel);
                    return event.filterAndAccept();
                }
            }
        }
    }

    if (event.key().check(FcitxKey_Escape)) {
        if (buffer_.empty()) {
            return;
        } else {
            reset();
            return event.filterAndAccept();
        }
    }

    if (buffer_.size() == 1 && buffer_.userInput().at(0) == 'w' &&
        event.key().isDigit()) {
        buffer_.type(event.key().sym());

        updatePreedit();
        setSymLookupTable();
        return event.filterAndAccept();
    } else if (event.key().isDigit()) {
        if (auto candidateList = ic_->inputPanel().candidateList()) {
            int idx = event.key().keyListIndex(selectionKeys());
            if (idx >= 0 && idx < candidateList->size()) {
                event.accept();
                candidateList->candidate(idx).select(ic_);
            }

            return event.filterAndAccept();
        }
    }

    if (event.key().isLAZ() || event.key().check(FcitxKey_period) ||
        event.key().check(FcitxKey_comma) ||
        event.key().check(FcitxKey_slash) ||
        event.key().check(FcitxKey_semicolon) ||
        event.key().check(FcitxKey_question)) {

        if (space_press_count == 1) {
            if (auto candidateList = ic_->inputPanel().candidateList()) {
                event.accept();
                candidateList->candidate(0).select(ic_);

                /*
                 * Still need to handle press key after space.
                 */
                buffer_.clear();
                buffer_.type(event.key().sym());
                updateUI();
                return event.filterAndAccept();
            }
        }

        if (buffer_.size() >= 5)
            return event.filterAndAccept();

        if (event.key().check(FcitxKey_question)) {
            wildcard_char_count++;
        }

        buffer_.type(event.key().sym());
    } else {
        return;
    }

    updateUI();
    return event.filterAndAccept();
}

void ArrayState::reset() {
    space_press_count = 0;
    wildcard_char_count = 0;

    buffer_.clear();
    updateUI();
}

void ArrayState::setLookupTable(bool isMain) {
    auto ctx = engine_->context();
    const auto &config = *(engine_->config());
    std::vector<std::string> words;

    if (isMain)
        words = (ctx->get())
                    ->get_candidates_from_main(buffer_.userInput(),
                                               wildcard_char_count);
    else
        words = (ctx->get())->get_candidates_from_simple(buffer_.userInput());

    if (words.size() > 0) {
        auto candidate = std::make_unique<fcitx::CommonCandidateList>();
        candidate->setSelectionKey(selectionKeys());
        candidate->setCursorPositionAfterPaging(
            fcitx::CursorPositionAfterPaging::ResetToFirst);
        candidate->setPageSize(10);
        for (unsigned int i = 0; i < words.size(); i++) {
            std::string value = words[i];
            candidate->append<ArrayCandidate>(engine_, i, value);
        }

        candidate->setLayoutHint(*config.candidateLayoutHint);
        candidate->setGlobalCursorIndex(0);
        ic_->inputPanel().setCandidateList(std::move(candidate));
    }
}

void ArrayState::setSymLookupTable() {
    auto ctx = engine_->context();
    const auto &config = *(engine_->config());
    std::vector<std::string> words;

    words = (ctx->get())->get_candidates_from_main(buffer_.userInput(), 0);
    if (words.size() > 0) {
        auto candidate = std::make_unique<fcitx::CommonCandidateList>();
        candidate->setSelectionKey(selectionKeys());
        candidate->setCursorPositionAfterPaging(
            fcitx::CursorPositionAfterPaging::ResetToFirst);
        candidate->setPageSize(10);
        for (unsigned int i = 0; i < words.size(); i++) {
            std::string value = words[i];
            candidate->append<ArrayCandidate>(engine_, i, value);
        }

        candidate->setLayoutHint(*config.candidateLayoutHint);
        candidate->setGlobalCursorIndex(0);
        ic_->inputPanel().setCandidateList(std::move(candidate));
    }
}

void ArrayState::setPhraseLookupTable() {
    auto ctx = engine_->context();
    std::vector<std::string> words;

    words = (ctx->get())->get_candidates_from_phrase(buffer_.userInput());
    if (words.size() > 0) {
        auto candidate = std::make_unique<fcitx::CommonCandidateList>();
        candidate->setSelectionKey(selectionKeys());
        candidate->setCursorPositionAfterPaging(
            fcitx::CursorPositionAfterPaging::ResetToFirst);
        candidate->setPageSize(10);
        for (unsigned int i = 0; i < words.size(); i++) {
            std::string value = words[i];
            candidate->append<ArrayCandidate>(engine_, i, value);
        }

        candidate->setGlobalCursorIndex(0);
        ic_->inputPanel().setCandidateList(std::move(candidate));
    }
}

void ArrayState::updatePreedit() {
    auto ctx = engine_->context();

    auto &inputPanel = ic_->inputPanel();
    inputPanel.reset();

    std::string input = buffer_.userInput();
    std::string preeditstring = (ctx->get())->get_preedit_string(input);
    fcitx::Text preedit(preeditstring);
    inputPanel.setPreedit(preedit);

    if (input.compare("w") == 0) {
        ic_->inputPanel().setAuxDown(
            fcitx::Text(_("1.comma 2.bracket 3.symbol 4.math 5.arrow 6.unit "
                          "7.table 8.roman 9.greek 0.bopomo")));
    }

    ic_->updateUserInterface(fcitx::UserInterfaceComponent::InputPanel);
    ic_->updatePreedit();
}

void ArrayState::updateUI() {
    updatePreedit();

    if (buffer_.size() <= 2 && space_press_count == 0) {
        setLookupTable(0);
    } else {
        setLookupTable(1);
    }
}

/*
 * ArrayEngine
 */

ArrayEngine::ArrayEngine(fcitx::Instance *instance)
    : instance_(instance), factory_([this](fcitx::InputContext &ic) {
          return new ArrayState(this, &ic);
      }),
      context_(std::make_unique<ArrayContext>()) {
    instance->inputContextManager().registerProperty("arrayState", &factory_);
    reloadConfig();
}

void ArrayEngine::activate(const fcitx::InputMethodEntry &entry,
                           fcitx::InputContextEvent &event) {
    FCITX_UNUSED(entry);
    auto *inputContext = event.inputContext();

    if (*config_.useChttrans && chttrans()) {
        if (auto *action =
                instance_->userInterfaceManager().lookupAction("chttrans")) {
            inputContext->statusArea().addAction(
                fcitx::StatusGroup::InputMethod, action);
        }
    }

    if (*config_.useFullWidth && fullwidth()) {
        if (auto *action =
                instance_->userInterfaceManager().lookupAction("fullwidth")) {
            inputContext->statusArea().addAction(
                fcitx::StatusGroup::InputMethod, action);
        }
    }

    is_special_notify_ = *config_.SpecialNotify;
}

void ArrayEngine::keyEvent(const fcitx::InputMethodEntry &entry,
                           fcitx::KeyEvent &keyEvent) {
    FCITX_UNUSED(entry);

    auto ic = keyEvent.inputContext();
    auto *state = ic->propertyFor(&factory_);
    state->keyEvent(keyEvent);
}

void ArrayEngine::reset(const fcitx::InputMethodEntry &,
                        fcitx::InputContextEvent &event) {
    auto *state = event.inputContext()->propertyFor(&factory_);
    state->reset();
}

const fcitx::Configuration *ArrayEngine::getConfig() const { return &config_; }

void ArrayEngine::setConfig(const fcitx::RawConfig &rawConfig) {
    config_.load(rawConfig, true);
    safeSaveAsIni(config_, "conf/array.conf");
    reloadConfig();
}

void ArrayEngine::reloadConfig() { readAsIni(config_, "conf/array.conf"); }

FCITX_ADDON_FACTORY(ArrayEngineFactory);

} // namespace Array
