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
 * Universe.
 * Set the Stage.
 */

#include "USD_api.h"
#include "USD_area.h"
#include "USD_factory.h"
#include "USD_region.h"
#include "USD_screen.h"
#include "USD_userpref.h"
#include "USD_window.h"

#include "KLI_icons.h"

#include "KKE_context.h"

#include <wabi/usd/usd/stage.h>
#include <wabi/usd/usdGeom/cube.h>
#include <wabi/usd/usdUI/tokens.h>


/**
 * file format defaults. */


KRAKEN_NAMESPACE_BEGIN

/* clang-format off */


void USD_default_table_main_window(kContext *C)
{
  wmWindow *win = CTX_wm_window(C);

  FormFactory(PROP_IFACE(win->title),         VALUE_IFACE(TfToken("Kraken")));
  // FormFactory(PROP_IFACE(win->icon),          VALUE_IFACE(SdfAssetPath(KLI_icon(ICON_KRAKEN))));
  FormFactory(PROP_IFACE(win->state),         VALUE_IFACE(UsdUITokens->normal));
  FormFactory(PROP_IFACE(win->cursor),        VALUE_IFACE(UsdUITokens->default_));
  FormFactory(PROP_IFACE(win->alignment),     VALUE_IFACE(UsdUITokens->alignAbsolute));
  FormFactory(PROP_IFACE(win->type),          VALUE_IFACE(UsdUITokens->normal));
  FormFactory(PROP_IFACE(win->pos),           VALUE_IFACE(GfVec2f(0, 0)));
  FormFactory(PROP_IFACE(win->size),          VALUE_IFACE(GfVec2f(1920, 1080)));
  FormFactory(PROP_IFACE(win->dpi),           VALUE_IFACE(float(1.0)));
  FormFactory(PROP_IFACE(win->widgetunit),    VALUE_IFACE(float(20.0)));
  FormFactory(PROP_IFACE(win->scale),         VALUE_IFACE(float(1.0)));
  FormFactory(PROP_IFACE(win->linewidth),     VALUE_IFACE(float(1.0)));
  FormFactory(PROP_IFACE(win->pixelsz),       VALUE_IFACE(float(1.0)));
}


void USD_default_table_user_prefs(kContext *C)
{
  UserDef *prefs = CTX_data_prefs(C);

  FormFactory(PROP_IFACE(prefs->showsave), VALUE_IFACE(bool(true)));
  FormFactory(PROP_IFACE(prefs->dpifac),   VALUE_IFACE(float(1.0)));
}


void USD_default_table_area_v3d(kContext *C)
{
  ScrArea *v3d = CTX_wm_area(C);

  FormFactory(PROP_IFACE(v3d->name),      VALUE_IFACE(TfToken("View3D")));
  FormFactory(PROP_IFACE(v3d->spacetype), VALUE_IFACE(UsdUITokens->spaceView3D));
  // FormFactory(PROP_IFACE(v3d->icon),      VALUE_IFACE(SdfAssetPath(KLI_icon(ICON_HYDRA))));
  FormFactory(PROP_IFACE(v3d->pos),       VALUE_IFACE(GfVec2f(0, 0)));
  FormFactory(PROP_IFACE(v3d->size),      VALUE_IFACE(GfVec2f(1800, 1080)));  
  FormFactory(PROP_IFACE(v3d->coords),    VALUE_IFACE(GfVec4i(0, 1800, 0, 1080)));
}


void USD_default_table_area_outliner(kContext *C)
{
  ScrArea *outliner = CTX_wm_area(C);

  FormFactory(PROP_IFACE(outliner->name),      VALUE_IFACE(TfToken("Outliner")));
  FormFactory(PROP_IFACE(outliner->spacetype), VALUE_IFACE(UsdUITokens->spaceOutliner));
  // FormFactory(PROP_IFACE(outliner->icon),      VALUE_IFACE(SdfAssetPath(KLI_icon(ICON_LUXO))));
  FormFactory(PROP_IFACE(outliner->pos),       VALUE_IFACE(GfVec2f(1800, 0)));
  FormFactory(PROP_IFACE(outliner->size),      VALUE_IFACE(GfVec2f(120, 1080)));
  FormFactory(PROP_IFACE(outliner->coords),    VALUE_IFACE(GfVec4i(1800, 1920, 0, 1080)));
}


void USD_default_table_area_screen(kContext *C)
{
  kScreen *screen = CTX_wm_screen(C);

  UNIVERSE_FOR_ALL(area, screen->areas)
  {
    /** Add UI Areas to Screen's Collection of Areas. */
    FormFactory(PROP_IFACE(screen->areas_rel), VALUE_IFACE(SdfPath(area->path)));
  }

  FormFactory(PROP_IFACE(screen->align), VALUE_IFACE(UsdUITokens->none));
}


void USD_default_table_area_workspace(kContext *C)
{
  WorkSpace *workspace = CTX_wm_workspace(C);
  kScreen *screen = CTX_wm_screen(C);

  /** Add this screen to our default 'Layout' WorkSpace. */
  FormFactory(PROP_IFACE(workspace->name),       VALUE_IFACE(TfToken("Layout")));
  FormFactory(PROP_IFACE(workspace->screen_rel), VALUE_IFACE(screen->path));
}


void USD_default_table_scene_data(kContext *C)
{
  KrakenSTAGE stage = CTX_data_stage(C);

  UsdGeomCube cube = UsdGeomCube::Define(stage, SdfPath("/Cube"));

  UsdGeomXformOp location = cube.AddTranslateOp();
  location.Set(VtValue(GfVec3d(0.0f, 0.0f, 0.0f)));
  UsdGeomXformOp rotation = cube.AddRotateXYZOp();
  rotation.Set(VtValue(GfVec3f(0.0f, 0.0f, 0.0f)));
  UsdGeomXformOp scale = cube.AddScaleOp();
  scale.Set(VtValue(GfVec3f(1.0f, 1.0f, 1.0f)));
}


/* clang-format on */

KRAKEN_NAMESPACE_END