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

#ifndef FCITX5_ARRAY_CONFIG_H
#define FCITX5_ARRAY_CONFIG_H

#include <fcitx-config/configuration.h>
#include <fcitx-config/enum.h>
#include <fcitx-utils/i18n.h>
#include <fcitx/candidatelist.h>
#include <quickphrase_public.h>

namespace fcitx {

FCITX_CONFIG_ENUM_NAME_WITH_I18N(CandidateLayoutHint, N_("Not set"),
                                 N_("Vertical"), N_("Horizontal"));
} // namespace fcitx

namespace Array {

FCITX_CONFIGURATION(
    FcitxArrayConfig,
    fcitx::Option<bool> useFullWidth{this, "UseFullWidth", _("Use full width"),
                                     true};
    fcitx::Option<bool> useChttrans{this, "UseChttrans", _("Use chttrans"),
                                     false};
    fcitx::Option<fcitx::Key, fcitx::KeyConstrain> quickphraseKey{
        this, "QuickPhraseKey", _("QuickPhrase Trigger Key"),
        fcitx::Key(FcitxKey_grave), fcitx::KeyConstrain{fcitx::KeyConstrainFlag::AllowModifierLess}};
    fcitx::OptionWithAnnotation<fcitx::CandidateLayoutHint,
                                fcitx::CandidateLayoutHintI18NAnnotation>
        candidateLayoutHint{this, "CandidateLayoutHint",
                            _("Candidate List orientation"),
                            fcitx::CandidateLayoutHint::NotSet};
    fcitx::Option<bool> SpecialNotify{this, "SpecialNotify", _("Notify special code"),
                                     false};);
} // namespace Array
#endif // FCITX5_ARRAY_CONFIG_H
