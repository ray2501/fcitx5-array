/*
 * License: LGPL-2.1-or-later
 *
 */

#ifndef _ASSOCIATION_ASSOCIATION_H_
#define _ASSOCIATION_ASSOCIATION_H_

#include <fcitx-config/configuration.h>
#include <fcitx-config/iniparser.h>
#include <fcitx-utils/i18n.h>
#include <fcitx/action.h>
#include <fcitx/addoninstance.h>
#include <fcitx/addonmanager.h>
#include <fcitx/candidatelist.h>
#include <fcitx/instance.h>

namespace fcitx {
FCITX_CONFIG_ENUM_NAME_WITH_I18N(CandidateLayoutHint, N_("Not set"),
                                 N_("Vertical"), N_("Horizontal"));
} // namespace fcitx

FCITX_CONFIGURATION(
    AssociationConfig,
    fcitx::Option<fcitx::KeyList> hotkey{this, "Hotkey", _("Toggle key")};
    fcitx::OptionWithAnnotation<fcitx::CandidateLayoutHint,
                                fcitx::CandidateLayoutHintI18NAnnotation>
        candidateLayoutHint{this, "CandidateLayoutHint",
                            _("Candidate List orientation"),
                            fcitx::CandidateLayoutHint::NotSet};)

class ToggleAction;

class Association final : public fcitx::AddonInstance {
    class ToggleAction : public fcitx::Action {
    public:
        ToggleAction(Association *parent) : parent_(parent) {}

        std::string shortText(fcitx::InputContext *) const override {
            return parent_->enabled_ ? _("Enable Associated Phrases")
                                     : _("Disable Associated Phrases");
        }
        std::string icon(fcitx::InputContext *) const override {
            return parent_->enabled_ ? "fcitx-association-active"
                                     : "fcitx-association-inactive";
        }

        void activate(fcitx::InputContext *ic) override {
            return parent_->setEnabled(!parent_->enabled_, ic);
        }

    private:
        Association *parent_;
    };

public:
    Association(fcitx::Instance *instance);

    void reloadConfig() override;
    const fcitx::Configuration *getConfig() const override { return &config_; }
    void setConfig(const fcitx::RawConfig &config) override {
        config_.load(config, true);
        fcitx::safeSaveAsIni(config_, "conf/association.conf");
    }

    FCITX_ADDON_DEPENDENCY_LOADER(notifications, instance_->addonManager());

    bool inWhiteList(fcitx::InputContext *inputContext) const;

    void setEnabled(bool enabled, fcitx::InputContext *ic) {
        if (enabled != enabled_) {
            enabled_ = enabled;
            toggleAction_.update(ic);
        }
    }

    void reset(fcitx::InputContext *inputContext);
    void updateUI(fcitx::InputContext *inputContext);

private:
#ifdef USE_FCITX5_LEGACY_API_STANDARDPATH
    void load(fcitx::StandardPathFile &file);
#else
    void load(int fd);
#endif
    std::multimap<std::string, std::string> map_;
    std::string commitstr_;
    bool isNeedUpdate = false;

    bool enabled_ = true;
    fcitx::Instance *instance_;
    AssociationConfig config_;
    std::vector<std::unique_ptr<fcitx::HandlerTableEntry<fcitx::EventHandler>>>
        eventHandlers_;
    fcitx::ScopedConnection commitFilterConn_;
    fcitx::ScopedConnection outputFilterConn_;
    std::unordered_set<std::string> whiteList_;
    ToggleAction toggleAction_{this};
};

#endif // _ASSOCIATION_ASSOCIATION_H_
