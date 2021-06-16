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

/**
 * @file
 * COVAH Library.
 * Gadget Vault.
 */

#pragma once

#include "CLI_api.h"

enum {
  ICON_COVAH = 0,
  ICON_WINDOW_CLOSE,
  ICON_WINDOW_MINIMIZE,
  ICON_WINDOW_FULLSCREEN,
  ICON_BACKGROUND_DOTS,
  ICON_HYDRA,
  ICON_NODES,
  ICON_PROPERTIES,
  ICON_HIERARCHY,
  ICON_ANALYSIS,
  ICON_DOPE_SHEET,
  ICON_SIM,
  ICON_IMAGING,
  ICON_ASSETS,
  ICON_SHELL,
  ICON_LUXO,
  ICON_STAGE,
  ICON_UNI,
  ICON_SIMULATION,
  ICON_ANIMATION,
  ICON_SHADING,
  ICON_SCRIPTING,
  ICON_ROOT,
  ICON_XFORM,
  ICON_GEOM,
  ICON_CAMERA,
  ICON_LIGHT,
  ICON_HIDE,
  ICON_SHOW,
  ICON_DRAW_WIREFRAME,
  ICON_DRAW_SOLID,
  ICON_DRAW_SHADED,
  ICON_PROPS,
  ICON_PROP_XYZ,
  ICON_PROP_CUBE,
  ICON_PROP_RENDER,
  ICON_RENDER,
  ICON_VIEW_LAYER,
  ICON_XYZ,
  ICON_FILE_NEW,
  ICON_FILE_OPEN,
  ICON_FILE_PREVIOUS,
  ICON_URL,
  ICON_SETTINGS,
  ICON_SEARCH,
  ICON_FILTER,
  ICON_HEART,
  ICON_CUBE,
  ICON_SPHERE,
  ICON_GRID,
  ICON_CONE,
  ICON_CYLINDER,
  ICON_CAPSULE,
  ICON_SUZANNE,
  ICON_ARROW_UP,
  ICON_ARROW_DOWN,
  ICON_ARROW_LEFT,
  ICON_ARROW_RIGHT,
  ICON_CURSOR_POINTER,
  ICON_CURSOR_POINTER_HAND,
  ICON_CLICK_LEFT,
  ICON_CLICK_RIGHT,
  ICON_CLICK_MIDDLE,
};

COVAH_LIB_API
std::string CLI_icon(int icon_id);
