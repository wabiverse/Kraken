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

#pragma once

/**
 * @file
 * Universe.
 * Set the Stage.
 */

#include "USD_wm_types.h"
#include "USD_context.h"
#include "USD_screen.h"
#include "USD_workspace.h"
#include "USD_userdef_types.h"

#include "KKE_context.h"

#include <wabi/usd/sdf/path.h>
#include <wabi/usd/usdUI/userPref.h>

typedef struct uiStyle uiStyle;
typedef struct kTheme kTheme;
typedef struct UserDef UserDef;

struct kUserDef : public wabi::UsdUIUserPref, public UserDef
{
  wabi::SdfPath path;

  wabi::UsdAttribute showsave;
  wabi::UsdAttribute dpifac;

  int uiflag;

  inline kUserDef(
    kContext *C,
    const wabi::SdfPath &stagepath = SdfPath(KRAKEN_PATH_DEFAULTS::KRAKEN_USERPREFS));
};

kUserDef::kUserDef(kContext *C, const wabi::SdfPath &stagepath)
  : UsdUIUserPref(KRAKEN_STAGE_CREATE(C)),
    path(GetPath()),
    showsave(CreateShowSavePromptAttr()),
    dpifac(CreateDpifacAttr()),
    uiflag(VALUE_ZERO)
{}
