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
 * Copyright 2021, Wabi.
 */

#pragma once

/**
 * @file
 * Universe.
 * Set the Stage.
 */

#include "UNI_context.h"
#include "UNI_screen.h"
#include "UNI_workspace.h"

#include "CKE_context.h"

#include <wabi/usd/sdf/path.h>
#include <wabi/usd/usdUI/userPref.h>

WABI_NAMESPACE_BEGIN

enum eUserprefUIFlag
{
  USER_UIFLAG_UNUSED_0 = (1 << 0),
  USER_UIFLAG_UNUSED_1 = (1 << 1),
  USER_WHEELZOOMDIR = (1 << 2),
  USER_FILTERFILEEXTS = (1 << 3),
  USER_DRAWVIEWINFO = (1 << 4),
  USER_PLAINMENUS = (1 << 5),
  USER_LOCK_CURSOR_ADJUST = (1 << 6),
  USER_HEADER_BOTTOM = (1 << 7),
  USER_HEADER_FROM_PREF = (1 << 8),
  USER_MENUOPENAUTO = (1 << 9),
  USER_DEPTH_CURSOR = (1 << 10),
  USER_AUTOPERSP = (1 << 11),
  USER_UIFLAG_UNUSED_12 = (1 << 12),
  USER_GLOBALUNDO = (1 << 13),
  USER_ORBIT_SELECTION = (1 << 14),
  USER_DEPTH_NAVIGATE = (1 << 15),
  USER_HIDE_DOT = (1 << 16),
  USER_SHOW_GIZMO_NAVIGATE = (1 << 17),
  USER_SHOW_VIEWPORTNAME = (1 << 18),
  USER_UIFLAG_UNUSED_3 = (1 << 19),
  USER_ZOOM_TO_MOUSEPOS = (1 << 20),
  USER_SHOW_FPS = (1 << 21),
  USER_UIFLAG_UNUSED_22 = (1 << 22),
  USER_MENUFIXEDORDER = (1 << 23),
  USER_CONTINUOUS_MOUSE = (1 << 24),
  USER_ZOOM_INVERT = (1 << 25),
  USER_ZOOM_HORIZ = (1 << 26),
  USER_SPLASH_DISABLE = (1 << 27),
  USER_HIDE_RECENT = (1 << 28),
  USER_SAVE_PROMPT = (1 << 30),
  USER_HIDE_SYSTEM_BOOKMARKS = (1u << 31),
};

struct UserDef : public UsdUIUserPref, public UniverseObject
{

  SdfPath path;

  UsdAttribute showsave;
  UsdAttribute dpifac;

  int uiflag;

  inline UserDef(cContext *C,
                 const SdfPath &stagepath = SdfPath(KRAKEN_PATH_DEFAULTS::KRAKEN_USERPREFS));
};

UserDef::UserDef(cContext *C, const SdfPath &stagepath)
  : UsdUIUserPref(KRAKEN_UNIVERSE_CREATE(C)),
    path(stagepath),
    showsave(CreateShowSavePromptAttr()),
    dpifac(CreateDpifacAttr()),
    uiflag(VALUE_ZERO)
{}

WABI_NAMESPACE_END