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
 * Universe.
 * Set the Stage.
 */

#include <wabi/wabi.h>

#include <wabi/base/tf/hashmap.h>

#include <wabi/usd/sdf/path.h>
#include <wabi/usd/usd/property.h>
#include <wabi/usd/usdUI/window.h>

struct Scene;

struct wmWindow : public wabi::UsdUIWindow {
  /** Anchor system backend pointer. */
  void *anchorwin;

  wabi::UsdProperty winCoords;
  wabi::UsdProperty workspace;
  wabi::UsdProperty screen;

  /** Active scene for this window. */
  Scene *scene;

  /** Active session layer display name. */
  char view_layer_name[64];
};

struct wmWindowManager {
  /** All windows this manager controls. */
  wabi::TfHashMap<wabi::TfToken, wmWindow *, wabi::TfHash> windows;
};