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

#ifndef _FCITX5_ARRAY_ARRAY_H_
#define _FCITX5_ARRAY_ARRAY_H_

#include <fcitx-utils/i18n.h>
#include <fcitx-utils/inputbuffer.h>
#include <fcitx/addonfactory.h>
#include <fcitx/addonmanager.h>
#include <fcitx/inputcontext.h>
#include <fcitx/inputcontextproperty.h>
#include <fcitx/inputmethodengine.h>
#include <fcitx/inputpanel.h>
#include <fcitx/instance.h>

#include "array.h"
#include "arrayconfig.h"

namespace Array {

class ArrayEngine;

class ArrayState : public fcitx::InputContextProperty {
public:
    ArrayState(ArrayEngine *engine, fcitx::InputContext *ic)
        : engine_(engine), ic_(ic) {}

    void keyEvent(fcitx::KeyEvent &keyEvent);
    void reset();
    void setLookupTable(bool isMain);
    void setSymLookupTable();
    void setPhraseLookupTable();
    void updatePreedit();
    void updateUI();

    std::string getInput() const { return buffer_.userInput(); }

private:
    ArrayEngine *engine_;
    fcitx::InputContext *ic_;

    ArrayContext *array_context;
    fcitx::InputBuffer buffer_{{fcitx::InputBufferOption::AsciiOnly,
                                fcitx::InputBufferOption::FixedCursor}};
    unsigned int space_press_count = 0;
    unsigned int wildcard_char_count = 0;
};

class ArrayEngine : public fcitx::InputMethodEngineV2 {
public:
    ArrayEngine(fcitx::Instance *instance);

    void activate(const fcitx::InputMethodEntry &,
                  fcitx::InputContextEvent &) override;

    void keyEvent(const fcitx::InputMethodEntry &entry,
                  fcitx::KeyEvent &keyEvent) override;

    void reset(const fcitx::InputMethodEntry &,
               fcitx::InputContextEvent &event) override;

    const fcitx::Configuration *getConfig() const override;
    void setConfig(const fcitx::RawConfig &) override;
    void reloadConfig() override;

    auto instance() const { return instance_; }
    auto factory() const { return &factory_; }
    auto context() const { return &context_; }
    auto config() const { return &config_; }
    auto is_special_notify() const { return is_special_notify_; }

    FCITX_ADDON_DEPENDENCY_LOADER(fullwidth, instance_->addonManager());
    FCITX_ADDON_DEPENDENCY_LOADER(chttrans, instance_->addonManager());
    FCITX_ADDON_DEPENDENCY_LOADER(quickphrase, instance_->addonManager());
    FCITX_ADDON_DEPENDENCY_LOADER(notifications, instance_->addonManager());

private:
    fcitx::Instance *instance_;
    fcitx::FactoryFor<ArrayState> factory_;
    std::unique_ptr<ArrayContext> context_;
    FcitxArrayConfig config_;
    bool is_special_notify_ = false;
};

class ArrayEngineFactory final : public fcitx::AddonFactory {
public:
    fcitx::AddonInstance *create(fcitx::AddonManager *manager) override {
        fcitx::registerDomain("fcitx5-array", FCITX_INSTALL_LOCALEDIR);
        return new ArrayEngine(manager->instance());
    }
};

} // namespace Array

#endif // _FCITX5_ARRAY_ARRAY_H_
