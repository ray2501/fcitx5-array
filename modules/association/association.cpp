/*
 * License: LGPL-2.1-or-later
 *
 */

#include "association.h"
#include "notifications_public.h"
#include <fcitx-config/iniparser.h>
#include <fcitx-utils/i18n.h>
#include <fcitx-utils/standardpath.h>
#include <fcitx-utils/utf8.h>
#include <fcitx/addonfactory.h>
#include <fcitx/inputcontext.h>
#include <fcitx/inputmethodentry.h>
#include <fcitx/statusarea.h>
#include <fcitx/userinterfacemanager.h>
#include <fcntl.h>

using namespace fcitx;

const fcitx::KeyList &selectionKeys() {
    static const fcitx::KeyList selectionKeys{
        fcitx::Key{FcitxKey_1}, fcitx::Key{FcitxKey_2}, fcitx::Key{FcitxKey_3},
        fcitx::Key{FcitxKey_4}, fcitx::Key{FcitxKey_5}, fcitx::Key{FcitxKey_6},
        fcitx::Key{FcitxKey_7}, fcitx::Key{FcitxKey_8}, fcitx::Key{FcitxKey_9},
        fcitx::Key{FcitxKey_0}};
    return selectionKeys;
}

Association::Association(Instance *instance) : instance_(instance) {
    instance_->userInterfaceManager().registerAction("association",
                                                     &toggleAction_);

    auto reset = [this](Event &event) {
        auto &icEvent = static_cast<InputContextEvent &>(event);
        auto *inputContext = icEvent.inputContext();
        if (enabled_) {
            isNeedUpdate = false;
            commitstr_.clear();
            this->updateUI(inputContext);
        }
    };
    eventHandlers_.emplace_back(instance_->watchEvent(
        EventType::InputContextFocusOut, EventWatcherPhase::Default, reset));
    eventHandlers_.emplace_back(instance_->watchEvent(
        EventType::InputContextReset, EventWatcherPhase::Default, reset));
    eventHandlers_.emplace_back(
        instance_->watchEvent(EventType::InputContextSwitchInputMethod,
                              EventWatcherPhase::Default, reset));

    eventHandlers_.emplace_back(instance->watchEvent(
        EventType::InputContextKeyEvent, EventWatcherPhase::Default,
        [this](Event &event) {
            auto &keyEvent = static_cast<KeyEvent &>(event);
            if (keyEvent.isRelease()) {
                return;
            }
            if (!inWhiteList(keyEvent.inputContext())) {
                return;
            }

            if (keyEvent.key().checkKeyList(config_.hotkey.value())) {
                setEnabled(!enabled_, keyEvent.inputContext());
                if (notifications()) {
                    notifications()->call<INotifications::showTip>(
                        "fcitx-association-toggle", _("Associated Phraes"),
                        enabled_ ? "Enable Associated Phrases"
                                 : "Disable Associated Phrases",
                        _("Associated Phraes"),
                        enabled_ ? _("Associated Phraes is enabled.")
                                 : _("Associated Phraes is disabled."),
                        -1);
                }
                keyEvent.filterAndAccept();
                return;
            }

            return;
        }));

    eventHandlers_.emplace_back(instance_->watchEvent(
        EventType::InputContextKeyEvent, EventWatcherPhase::PreInputMethod,
        [this](Event &event) {
            auto &keyEvent = static_cast<KeyEvent &>(event);
            auto *inputContext = keyEvent.inputContext();
            if (!enabled_ || !inWhiteList(inputContext)) {
                return;
            }

            if (keyEvent.isRelease()) {
                return;
            }

            if (keyEvent.key().isModifier() || keyEvent.key().hasModifier()) {
                return;
            }

            if (!commitstr_.empty()) {
                if (keyEvent.key().check(FcitxKey_Escape)) {
                    keyEvent.filterAndAccept();
                    this->reset(inputContext);
                    return;
                }

                if (keyEvent.key().check(FcitxKey_Page_Up) ||
                    keyEvent.key().check(FcitxKey_Up) ||
                    keyEvent.key().check(FcitxKey_Left)) {
                    if (auto candidateList =
                            inputContext->inputPanel().candidateList()) {
                        if (candidateList->toPageable()->totalPages() > 1) {
                            if (candidateList->toPageable()->hasPrev()) {
                                candidateList->toPageable()->prev();
                            } else {
                                candidateList->toPageable()->setPage(
                                    candidateList->toPageable()->totalPages() -
                                    1);
                            }

                            inputContext->updateUserInterface(
                                fcitx::UserInterfaceComponent::InputPanel);
                        }
                    }

                    return keyEvent.filterAndAccept();
                }

                if (keyEvent.key().check(FcitxKey_space) ||
                    keyEvent.key().check(FcitxKey_Page_Down) ||
                    keyEvent.key().check(FcitxKey_Down) ||
                    keyEvent.key().check(FcitxKey_Right)) {
                    if (auto candidateList =
                            inputContext->inputPanel().candidateList()) {
                        if (candidateList->toPageable()->totalPages() > 1) {
                            if (candidateList->toPageable()->hasNext()) {
                                candidateList->toPageable()->next();
                            } else {
                                candidateList->toPageable()->setPage(0);
                            }

                            inputContext->updateUserInterface(
                                fcitx::UserInterfaceComponent::InputPanel);
                        }
                    }

                    return keyEvent.filterAndAccept();
                }

                if (keyEvent.key().isDigit()) {
                    if (auto candidateList =
                            inputContext->inputPanel().candidateList()) {
                        int idx = keyEvent.key().keyListIndex(selectionKeys());
                        if (idx >= 0 && idx < candidateList->size()) {
                            keyEvent.accept();
                            candidateList->candidate(idx).select(inputContext);
                        }

                        keyEvent.filterAndAccept();
                        return;
                    }
                } else {
                    this->reset(inputContext);
                    return;
                }
            } else {
                return;
            }
        }));

    outputFilterConn_ = instance_->connect<Instance::OutputFilter>(
        [this](InputContext *inputContext, Text &text) {
            if (!enabled_ || !inWhiteList(inputContext)) {
                return;
            }

            if (!commitstr_.empty())
                inputContext->updateUserInterface(
                    fcitx::UserInterfaceComponent::InputPanel);

            if (!commitstr_.empty() && text.size() == 0 &&
                isNeedUpdate == true) {
                updateUI(inputContext);
                isNeedUpdate = false;
            }
        });

    commitFilterConn_ = instance_->connect<Instance::CommitFilter>(
        [this](InputContext *inputContext, std::string &str) {
            if (!enabled_ || !inWhiteList(inputContext)) {
                return;
            }

            commitstr_.clear();
            for (auto chr : fcitx::utf8::MakeUTF8CharRange(str)) {
                commitstr_ = utf8::UCS4ToUTF8(chr);
            }

            isNeedUpdate = true;
        });

    reloadConfig();
}

void Association::reloadConfig() {
    map_.clear();
    auto file = StandardPath::global().open(
        StandardPath::Type::PkgData, "data/AssociatedPhrases.mb", O_RDONLY);

    if (file.fd() >= 0) {
        load(file);
    }

    readAsIni(config_, "conf/association.conf");
}

bool Association::inWhiteList(InputContext *inputContext) const {
    return toggleAction_.isParent(&inputContext->statusArea());
}

void Association::load(StandardPathFile &file) {
    UniqueFilePtr fp = fs::openFD(file, "rb");
    if (!fp) {
        return;
    }

    UniqueCPtr<char> buf;
    size_t len = 0;
    while (getline(buf, &len, fp.get()) != -1) {
        std::string strBuf(buf.get());

        auto [start, end] = stringutils::trimInplace(strBuf);
        if (start == end) {
            continue;
        }
        std::string_view text(strBuf.data() + start, end - start);
        if (!utf8::validate(text)) {
            continue;
        }

        auto pos = text.find_first_of(FCITX_WHITESPACE);
        if (pos == std::string::npos) {
            continue;
        }

        auto word = text.find_first_not_of(FCITX_WHITESPACE, pos);
        if (word == std::string::npos) {
            continue;
        }

        std::string key(text.begin(), text.begin() + pos);
        auto wordString =
            stringutils::unescapeForValue(std::string_view(text.substr(word)));

        if (!wordString) {
            continue;
        }

        map_.emplace(std::move(key), std::move(*wordString));
    }
}

void Association::reset(InputContext *inputContext) {
    isNeedUpdate = false;
    commitstr_.clear();
    updateUI(inputContext);
}

class AssociationCandidateWord : public CandidateWord {
public:
    AssociationCandidateWord(Association *q, int idx, std::string text)
        : q_(q), idx_(idx) {
        setText(fcitx::Text(std::move(text)));
    }

    void select(InputContext *inputContext) const override {
        std::string choiceword = text().toString();

        q_->reset(inputContext);
        inputContext->commitString(choiceword);
    }

private:
    Association *q_;
    int idx_;
};

class AssociationModuleFactory : public AddonFactory {
    AddonInstance *create(AddonManager *manager) override {
        return new Association(manager->instance());
    }
};

void Association::updateUI(InputContext *inputContext) {
    inputContext->inputPanel().reset();

    if (!commitstr_.empty()) {

        auto candidateList = std::make_unique<CommonCandidateList>();
        candidateList->setSelectionKey(selectionKeys());
        candidateList->setCursorPositionAfterPaging(
            fcitx::CursorPositionAfterPaging::ResetToFirst);
        candidateList->setPageSize(10);

        auto start = map_.lower_bound(commitstr_);
        auto end = map_.end();

        if (map_.count(commitstr_)) {
            unsigned int index = 0;
            for (; start != end; start++) {
                if (!stringutils::startsWith(start->first, commitstr_)) {
                    break;
                }

                candidateList->append<AssociationCandidateWord>(this, index,
                                                                start->second);
                index++;
            }

            candidateList->setGlobalCursorIndex(0);
            inputContext->inputPanel().setCandidateList(
                std::move(candidateList));

            Text auxUp(_("Associated Phrases: "));
            inputContext->inputPanel().setAuxUp(auxUp);
            inputContext->updatePreedit();
        } else {
            /*
             * I think it is good to clear.
             */
            commitstr_.clear();
        }
    }

    inputContext->updateUserInterface(UserInterfaceComponent::InputPanel);
}

FCITX_ADDON_FACTORY(AssociationModuleFactory)
