/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Copyright 2022, Wabi Animation Studios, Ltd. Co.
 */

/**
 * @file
 * Editors.
 * Tools for Artists.
 */

#include "USD_api.h"
#include "ED_userpref.h"

eUserPrefTokensType::eUserPrefTokensType()
  : KRAKEN_DEFINE_STATIC_TOKEN(PREFERENCES_OT_reset_default_theme),
    KRAKEN_DEFINE_STATIC_TOKEN(PREFERENCES_OT_autoexec_path_add),
    KRAKEN_DEFINE_STATIC_TOKEN(PREFERENCES_OT_autoexec_path_remove),
    KRAKEN_DEFINE_STATIC_TOKEN(PREFERENCES_OT_associate_usd),
    allTokens({PREFERENCES_OT_reset_default_theme,
               PREFERENCES_OT_autoexec_path_add,
               PREFERENCES_OT_autoexec_path_remove,
               PREFERENCES_OT_associate_usd})
{}

TfStaticData<eUserPrefTokensType> eUserPrefTokens;