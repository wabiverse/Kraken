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
 * COVAH Kernel.
 * Purple Underground.
 */

#pragma once

#include "CKE_main.h"

#include "UNI_scene.h"

#include <wabi/base/gf/vec2i.h>

enum SpaceWindowType {
  SPACE_WINDOW_DEFAULT = 0,
  SPACE_WINDOW_VIEW3D,
  SPACE_WINDOW_CODE,
  SPACE_WINDOW_ETCHER,
  SPACE_WINDOW_NODES,
  SPACE_WINDOW_TIMELINE,
  SPACE_WINDOW_USERPREF,
  SPACE_WINDOW_SPLASH,

  /** For custom layouts. */
  SPACE_WINDOW_CUSTOM1 = 50,
  SPACE_WINDOW_CUSTOM2,
  SPACE_KEYMAP_CUSTOM3
};

enum class SpaceKeymapType {
  SPACE_KEYMAP_VIEW3D,
  SPACE_KEYMAP_NODES,
  SPACE_KEYMAP_TIMELINE,
  SPACE_KEYMAP_USERPREF,

  /** For custom keymaps. */
  SPACE_KEYMAP_CUSTOM1,
  SPACE_KEYMAP_CUSTOM2,
  SPACE_KEYMAP_CUSTOM3,
};

struct SpaceArea {
  /** Coordinates of region. */
  wabi::GfVec2i win_coords;
  /** Size. */
  int width, height;

  /** Region is currently visible on screen. */
  bool visible;
  /** Window, header, etc. identifier for drawing. */
  int regiontype;
  /** How it should split. */
  int alignment;

  /** Fullscreen state of region. */
  bool fullscreen;
};

struct SpaceWindow {
  /** Window identifier, for which to subclass this window from. */
  int spaceid;
  /** Keymap identifier, to add keymaps to this window. */
  SpaceKeymapType keymap_id;

  /** Name of Window, displayed in menus and up-top. */
  char name[64];
  /** Icon identifier for this window. */
  int icon_id;

  void (*init)(Scene *scene);
};

typedef int wmWindowFlags;