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
 * KRAKEN Library.
 * Gadget Vault.
 */

#include "KLI_icons.h"

#include "KKE_main.h"

#include <wabi/base/arch/systemInfo.h>
#include <wabi/base/tf/stringUtils.h>

WABI_NAMESPACE_BEGIN

/**
 * Refactor and hash these in so we're not creating 19 billion strings. */

std::string KLI_icon(eKrakenIcon icon_id)
{
  switch (icon_id) {
    case ICON_NONE:
      return std::string();
    case ICON_KRAKEN:
      return STRCAT(G.main->icons_path, "kraken-desktop.png");
    case ICON_WINDOW_CLOSE:
      return STRCAT(G.main->icons_path, "win_close.png");
    case ICON_WINDOW_MINIMIZE:
      return STRCAT(G.main->icons_path, "win_min.png");
    case ICON_WINDOW_FULLSCREEN:
      return STRCAT(G.main->icons_path, "win_full.png");
    case ICON_BACKGROUND_DOTS:
      return STRCAT(G.main->icons_path, "tex_bg.png");
    case ICON_HYDRA:
      return STRCAT(G.main->icons_path, "hydra.png");
    case ICON_NODES:
      return STRCAT(G.main->icons_path, "universe.png");
    case ICON_PROPERTIES:
      return STRCAT(G.main->icons_path, "props.png");
    case ICON_HIERARCHY:
      return STRCAT(G.main->icons_path, "hierarchy.png");
    case ICON_ANALYSIS:
      return STRCAT(G.main->icons_path, "dna.png");
    case ICON_SIM:
      return STRCAT(G.main->icons_path, "simulate.png");
    case ICON_IMAGING:
      return STRCAT(G.main->icons_path, "imaging.png");
    case ICON_DOPE_SHEET:
      return STRCAT(G.main->icons_path, "dopesheet.png");
    case ICON_ASSETS:
      return STRCAT(G.main->icons_path, "assets.png");
    case ICON_SHELL:
      return STRCAT(G.main->icons_path, "shell.png");
    case ICON_LUXO:
      return STRCAT(G.main->icons_path, "luxo.png");
    case ICON_STAGE:
      return STRCAT(G.main->icons_path, "stage.png");
    case ICON_UNI:
      return STRCAT(G.main->icons_path, "world.png");
    case ICON_SIMULATION:
      return STRCAT(G.main->icons_path, "simulation.png");
    case ICON_ANIMATION:
      return STRCAT(G.main->icons_path, "animation.png");
    case ICON_SHADING:
      return STRCAT(G.main->icons_path, "shading.png");
    case ICON_SCRIPTING:
      return STRCAT(G.main->icons_path, "scripting.png");
    case ICON_ROOT:
      return STRCAT(G.main->icons_path, "collection.png");
    case ICON_XFORM:
      return STRCAT(G.main->icons_path, "xform.png");
    case ICON_GEOM:
      return STRCAT(G.main->icons_path, "geom.png");
    case ICON_CAMERA:
      return STRCAT(G.main->icons_path, "camera.png");
    case ICON_LIGHT:
      return STRCAT(G.main->icons_path, "light.png");
    case ICON_HIDE:
      return STRCAT(G.main->icons_path, "hide.png");
    case ICON_SHOW:
      return STRCAT(G.main->icons_path, "show.png");
    case ICON_DRAW_WIREFRAME:
      return STRCAT(G.main->icons_path, "draw_wireframe.png");
    case ICON_DRAW_SOLID:
      return STRCAT(G.main->icons_path, "draw_solid.png");
    case ICON_DRAW_SHADED:
      return STRCAT(G.main->icons_path, "draw_shaded.png");
    case ICON_PROPS:
      return STRCAT(G.main->icons_path, "properties.png");
    case ICON_PROP_XYZ:
      return STRCAT(G.main->icons_path, "prop_xyz.png");
    case ICON_PROP_CUBE:
      return STRCAT(G.main->icons_path, "prop_cube.png");
    case ICON_PROP_RENDER:
      return STRCAT(G.main->icons_path, "prop_render.png");
    case ICON_RENDER:
      return STRCAT(G.main->icons_path, "render.png");
    case ICON_VIEW_LAYER:
      return STRCAT(G.main->icons_path, "view_layer.png");
    case ICON_XYZ:
      return STRCAT(G.main->icons_path, "xyz.png");
    case ICON_FILE_NEW:
      return STRCAT(G.main->icons_path, "file_new.png");
    case ICON_FILE_OPEN:
      return STRCAT(G.main->icons_path, "file_open.png");
    case ICON_FILE_PREVIOUS:
      return STRCAT(G.main->icons_path, "file_previous.png");
    case ICON_URL:
      return STRCAT(G.main->icons_path, "url.png");
    case ICON_SETTINGS:
      return STRCAT(G.main->icons_path, "settings.png");
    case ICON_SEARCH:
      return STRCAT(G.main->icons_path, "search.png");
    case ICON_FILTER:
      return STRCAT(G.main->icons_path, "filter.png");
    case ICON_HEART:
      return STRCAT(G.main->icons_path, "heart.png");
    case ICON_CUBE:
      return STRCAT(G.main->icons_path, "cube.png");
    case ICON_SPHERE:
      return STRCAT(G.main->icons_path, "sphere.png");
    case ICON_GRID:
      return STRCAT(G.main->icons_path, "grid.png");
    case ICON_CONE:
      return STRCAT(G.main->icons_path, "cone.png");
    case ICON_CYLINDER:
      return STRCAT(G.main->icons_path, "cylinder.png");
    case ICON_CAPSULE:
      return STRCAT(G.main->icons_path, "pill.png");
    case ICON_SUZANNE:
      return STRCAT(G.main->icons_path, "suzanne.png");
    case ICON_ARROW_UP:
      return STRCAT(G.main->icons_path, "arrow_closed_up.png");
    case ICON_ARROW_DOWN:
      return STRCAT(G.main->icons_path, "arrow_closed_down.png");
    case ICON_ARROW_LEFT:
      return STRCAT(G.main->icons_path, "arrow_closed_left.png");
    case ICON_ARROW_RIGHT:
      return STRCAT(G.main->icons_path, "arrow_closed_right.png");
    case ICON_CURSOR_POINTER:
      return STRCAT(G.main->icons_path, "cursors/pointer_arrow.png");
    case ICON_CURSOR_POINTER_HAND:
      return STRCAT(G.main->icons_path, "cursors/pointer_hand.png");
    case ICON_CLICK_LEFT:
      return STRCAT(G.main->icons_path, "click_left.png");
    case ICON_CLICK_RIGHT:
      return STRCAT(G.main->icons_path, "click_right.png");
    case ICON_CLICK_MIDDLE:
      return STRCAT(G.main->icons_path, "click_middle.png");
    default:
      return std::string();
  }
}

WABI_NAMESPACE_END