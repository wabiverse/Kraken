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

#ifndef KRAKEN_EDITORS_USERPREF_H
#define KRAKEN_EDITORS_USERPREF_H

/**
 * @file
 * Editors.
 * Tools for Artists.
 */

#include "kraken/kraken.h"

#ifdef __cplusplus
#  include "USD_api.h"
#  include <wabi/base/tf/token.h>
#endif /* __cplusplus */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * -------------------------------------------------------------------------
 * @API: For Kraken user preferences,
 * which define the system options & themes.
 */

/* To register all userpref operators. */
void ED_operatortypes_userpref(void);

/**
 * -------------------------------------------------------------------------
 */

/**
 * -------------------------------------------------------------------------
 * @TOKENS: For quick lookups on commonly used types,
 * such as the suite of operators.
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#ifdef __cplusplus
#  define UPREF_ID_(t) eUserPrefTokens->t
struct eUserPrefTokensType
{
  eUserPrefTokensType();

  KRAKEN_DECLARE_STATIC_TOKEN(PREFERENCES_OT_reset_default_theme);
  KRAKEN_DECLARE_STATIC_TOKEN(PREFERENCES_OT_autoexec_path_add);
  KRAKEN_DECLARE_STATIC_TOKEN(PREFERENCES_OT_autoexec_path_remove);
  KRAKEN_DECLARE_STATIC_TOKEN(PREFERENCES_OT_associate_usd);

  const std::vector<TfToken> allTokens;
};
extern TfStaticData<eUserPrefTokensType> eUserPrefTokens;
#endif /* __cplusplus */

/**
 * -------------------------------------------------------------------------
 */

#endif /* KRAKEN_EDITORS_USERPREF_H */