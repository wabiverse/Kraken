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

#include "CLI_icons.h"

#include <wabi/base/arch/systemInfo.h>
#include <wabi/base/tf/stringUtils.h>

WABI_NAMESPACE_USING

/**
 * Refactor and hash these in so we're not creating 19 billion strings. */

std::string CLI_icon(int icon_id)
{
  const std::string exe_path = TfGetPathName(ArchGetExecutablePath());
  const std::string icons_path = TfStringCatPaths(exe_path, "../datafiles/icons");

  switch (icon_id) {
    case ICON_COVAH:
      return TfStringCatPaths(icons_path, "covah-desktop.png");
    case ICON_WINDOW_CLOSE:
      return TfStringCatPaths(icons_path, "win_close.png");
    case ICON_WINDOW_MINIMIZE:
      return TfStringCatPaths(icons_path, "win_min.png");
    case ICON_WINDOW_FULLSCREEN:
      return TfStringCatPaths(icons_path, "win_full.png");
    case ICON_BACKGROUND_DOTS:
      return TfStringCatPaths(icons_path, "tex_bg.png");
    case ICON_HYDRA:
      return TfStringCatPaths(icons_path, "hydra.png");
    case ICON_NODES:
      return TfStringCatPaths(icons_path, "universe.png");
    case ICON_PROPERTIES:
      return TfStringCatPaths(icons_path, "props.png");
    case ICON_HIERARCHY:
      return TfStringCatPaths(icons_path, "hierarchy.png");
    case ICON_ANALYSIS:
      return TfStringCatPaths(icons_path, "dna.png");
    case ICON_SIM:
      return TfStringCatPaths(icons_path, "simulate.png");
    case ICON_IMAGING:
      return TfStringCatPaths(icons_path, "imaging.png");
    case ICON_DOPE_SHEET:
      return TfStringCatPaths(icons_path, "dopesheet.png");
    case ICON_ASSETS:
      return TfStringCatPaths(icons_path, "assets.png");
    case ICON_SHELL:
      return TfStringCatPaths(icons_path, "shell.png");
    case ICON_LUXO:
      return TfStringCatPaths(icons_path, "luxo.png");
    case ICON_STAGE:
      return TfStringCatPaths(icons_path, "stage.png");
    case ICON_UNI:
      return TfStringCatPaths(icons_path, "world.png");
    case ICON_SIMULATION:
      return TfStringCatPaths(icons_path, "simulation.png");
    case ICON_ANIMATION:
      return TfStringCatPaths(icons_path, "animation.png");
    case ICON_SHADING:
      return TfStringCatPaths(icons_path, "shading.png");
    case ICON_SCRIPTING:
      return TfStringCatPaths(icons_path, "scripting.png");
    case ICON_ROOT:
      return TfStringCatPaths(icons_path, "collection.png");
    case ICON_XFORM:
      return TfStringCatPaths(icons_path, "xform.png");
    case ICON_GEOM:
      return TfStringCatPaths(icons_path, "geom.png");
    case ICON_CAMERA:
      return TfStringCatPaths(icons_path, "camera.png");
    case ICON_LIGHT:
      return TfStringCatPaths(icons_path, "light.png");
    case ICON_HIDE:
      return TfStringCatPaths(icons_path, "hide.png");
    case ICON_SHOW:
      return TfStringCatPaths(icons_path, "show.png");
    case ICON_DRAW_WIREFRAME:
      return TfStringCatPaths(icons_path, "draw_wireframe.png");
    case ICON_DRAW_SOLID:
      return TfStringCatPaths(icons_path, "draw_solid.png");
    case ICON_DRAW_SHADED:
      return TfStringCatPaths(icons_path, "draw_shaded.png");
    case ICON_PROPS:
      return TfStringCatPaths(icons_path, "properties.png");
    case ICON_PROP_XYZ:
      return TfStringCatPaths(icons_path, "prop_xyz.png");
    case ICON_PROP_CUBE:
      return TfStringCatPaths(icons_path, "prop_cube.png");
    case ICON_PROP_RENDER:
      return TfStringCatPaths(icons_path, "prop_render.png");
    case ICON_RENDER:
      return TfStringCatPaths(icons_path, "render.png");
    case ICON_VIEW_LAYER:
      return TfStringCatPaths(icons_path, "view_layer.png");
    case ICON_XYZ:
      return TfStringCatPaths(icons_path, "xyz.png");
    case ICON_FILE_NEW:
      return TfStringCatPaths(icons_path, "file_new.png");
    case ICON_FILE_OPEN:
      return TfStringCatPaths(icons_path, "file_open.png");
    case ICON_FILE_PREVIOUS:
      return TfStringCatPaths(icons_path, "file_previous.png");
    case ICON_URL:
      return TfStringCatPaths(icons_path, "url.png");
    case ICON_SETTINGS:
      return TfStringCatPaths(icons_path, "settings.png");
    case ICON_SEARCH:
      return TfStringCatPaths(icons_path, "search.png");
    case ICON_FILTER:
      return TfStringCatPaths(icons_path, "filter.png");
    case ICON_HEART:
      return TfStringCatPaths(icons_path, "heart.png");
    case ICON_CUBE:
      return TfStringCatPaths(icons_path, "cube.png");
    case ICON_SPHERE:
      return TfStringCatPaths(icons_path, "sphere.png");
    case ICON_GRID:
      return TfStringCatPaths(icons_path, "grid.png");
    case ICON_CONE:
      return TfStringCatPaths(icons_path, "cone.png");
    case ICON_CYLINDER:
      return TfStringCatPaths(icons_path, "cylinder.png");
    case ICON_CAPSULE:
      return TfStringCatPaths(icons_path, "pill.png");
    case ICON_SUZANNE:
      return TfStringCatPaths(icons_path, "suzanne.png");
    case ICON_ARROW_UP:
      return TfStringCatPaths(icons_path, "arrow_closed_up.png");
    case ICON_ARROW_DOWN:
      return TfStringCatPaths(icons_path, "arrow_closed_down.png");
    case ICON_ARROW_LEFT:
      return TfStringCatPaths(icons_path, "arrow_closed_left.png");
    case ICON_ARROW_RIGHT:
      return TfStringCatPaths(icons_path, "arrow_closed_right.png");
    case ICON_CURSOR_POINTER:
      return TfStringCatPaths(icons_path, "cursors/pointer_arrow.png");
    case ICON_CURSOR_POINTER_HAND:
      return TfStringCatPaths(icons_path, "cursors/pointer_hand.png");
    case ICON_CLICK_LEFT:
      return TfStringCatPaths(icons_path, "click_left.png");
    case ICON_CLICK_RIGHT:
      return TfStringCatPaths(icons_path, "click_right.png");
    case ICON_CLICK_MIDDLE:
      return TfStringCatPaths(icons_path, "click_middle.png");
    default:
      return TfStringCatPaths(icons_path, "covah-desktop.png");
  }
}