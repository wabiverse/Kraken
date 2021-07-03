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

#include "UNI_api.h"
#include "UNI_area.h"
#include "UNI_factory.h"
#include "UNI_region.h"
#include "UNI_screen.h"
#include "UNI_userpref.h"
#include "UNI_window.h"

#include "CLI_icons.h"

#include "CKE_context.h"

#include <wabi/usd/usd/stage.h>
#include <wabi/usd/usdGeom/cube.h>
#include <wabi/usd/usdUI/tokens.h>


/**
 * file format defaults. */


WABI_NAMESPACE_BEGIN

/* clang-format off */


void UNI_default_table_main_window(cContext *C)
{
  wmWindow *win = CTX_wm_window(C);

  FormFactory(PROP_IFACE(win->title),         VALUE_IFACE(TfToken("Kraken")));
  FormFactory(PROP_IFACE(win->icon),          VALUE_IFACE(SdfAssetPath(CLI_icon(ICON_KRAKEN))));
  FormFactory(PROP_IFACE(win->state),         VALUE_IFACE(UsdUITokens->maximized));
  FormFactory(PROP_IFACE(win->cursor),        VALUE_IFACE(UsdUITokens->default_));
  FormFactory(PROP_IFACE(win->alignment),     VALUE_IFACE(UsdUITokens->alignAbsolute));
  FormFactory(PROP_IFACE(win->type),          VALUE_IFACE(UsdUITokens->normal));
  FormFactory(PROP_IFACE(win->pos),           VALUE_IFACE(GfVec2f(0.0, 0.0)));
  FormFactory(PROP_IFACE(win->size),          VALUE_IFACE(GfVec2f(1920, 1080)));
  FormFactory(PROP_IFACE(win->dpi),           VALUE_IFACE(float(1.0)));
  FormFactory(PROP_IFACE(win->widgetunit),    VALUE_IFACE(float(20.0)));
  FormFactory(PROP_IFACE(win->scale),         VALUE_IFACE(float(1.0)));
  FormFactory(PROP_IFACE(win->linewidth),     VALUE_IFACE(float(1.0)));
  FormFactory(PROP_IFACE(win->pixelsz),       VALUE_IFACE(float(1.0)));
}


void UNI_default_table_user_prefs(cContext *C)
{
  UserDef *prefs = CTX_data_prefs(C);

  FormFactory(PROP_IFACE(prefs->showsave), VALUE_IFACE(bool(true)));
  FormFactory(PROP_IFACE(prefs->dpifac),   VALUE_IFACE(float(1.0)));
}


void UNI_default_table_area_v3d(cContext *C)
{
  ScrArea *v3d = CTX_wm_area(C);

  FormFactory(PROP_IFACE(v3d->name),      VALUE_IFACE(TfToken("View3D")));
  FormFactory(PROP_IFACE(v3d->spacetype), VALUE_IFACE(UsdUITokens->spaceView3D));
  FormFactory(PROP_IFACE(v3d->icon),      VALUE_IFACE(SdfAssetPath(CLI_icon(ICON_HYDRA))));
  FormFactory(PROP_IFACE(v3d->pos),       VALUE_IFACE(GfVec2f(0, 0)));
  FormFactory(PROP_IFACE(v3d->size),      VALUE_IFACE(GfVec2f(1800, 1080)));  
}


void UNI_default_table_area_outliner(cContext *C)
{
  ScrArea *outliner = CTX_wm_area(C);

  FormFactory(PROP_IFACE(outliner->name),      VALUE_IFACE(TfToken("Outliner")));
  FormFactory(PROP_IFACE(outliner->spacetype), VALUE_IFACE(UsdUITokens->spaceOutliner));
  FormFactory(PROP_IFACE(outliner->icon),      VALUE_IFACE(SdfAssetPath(CLI_icon(ICON_LUXO))));
  FormFactory(PROP_IFACE(outliner->pos),       VALUE_IFACE(GfVec2f(1800, 0)));
  FormFactory(PROP_IFACE(outliner->size),      VALUE_IFACE(GfVec2f(120, 1080)));
}


void UNI_default_table_area_screen(cContext *C)
{
  cScreen *screen = CTX_wm_screen(C);

  UNIVERSE_FOR_ALL(area, screen->areas)
  {
    /** Add UI Areas to Screen's Collection of Areas. */
    FormFactory(PROP_IFACE(screen->areas_rel), VALUE_IFACE(SdfPath(area->path)));
  }

  FormFactory(PROP_IFACE(screen->align), VALUE_IFACE(UsdUITokens->verticalSplit));
}


void UNI_default_table_area_workspace(cContext *C)
{
  WorkSpace *workspace = CTX_wm_workspace(C);
  cScreen *screen = CTX_wm_screen(C);

  /** Add this screen to our default 'Layout' WorkSpace. */
  FormFactory(PROP_IFACE(workspace->name),       VALUE_IFACE(TfToken("Layout")));
  FormFactory(PROP_IFACE(workspace->screen_rel), VALUE_IFACE(screen->path));
}


void UNI_default_table_scene_data(cContext *C)
{
  Stage stage = CTX_data_stage(C);

  UsdGeomCube cube = UsdGeomCube::Define(stage, SdfPath("/Cube"));

  UsdGeomXformOp location = cube.AddTranslateOp();
  location.Set(VtValue(GfVec3d(0.0f, 0.0f, 0.0f)));
  UsdGeomXformOp rotation = cube.AddRotateXYZOp();
  rotation.Set(VtValue(GfVec3f(0.0f, 0.0f, 0.0f)));
  UsdGeomXformOp scale = cube.AddScaleOp();
  scale.Set(VtValue(GfVec3f(1.0f, 1.0f, 1.0f)));
}


/* clang-format on */

WABI_NAMESPACE_END