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
#include "UNI_context.h"
#include "UNI_scene.h"
#include "UNI_screen.h"
#include "UNI_window.h"
#include "UNI_workspace.h"

#include "CKE_context.h"
#include "CKE_main.h"

#include "CLI_icons.h"

#include <wabi/wabi.h>

#include <wabi/base/vt/value.h>

#include <wabi/usd/usdUI/area.h>
#include <wabi/usd/usdUI/screen.h>
#include <wabi/usd/usdUI/tokens.h>
#include <wabi/usd/usdUI/window.h>
#include <wabi/usd/usdUI/workspace.h>

#include <wabi/usd/usdGeom/cube.h>
#include <wabi/usd/usdGeom/gprim.h>

#include <wabi/usd/usd/collectionAPI.h>
#include <wabi/usd/usd/stage.h>

WABI_NAMESPACE_BEGIN

void UNI_create_stage(cContext &C)
{
  Main main = CTX_data_main(C);

  main->stage_id = TfStringCatPaths(main->temp_dir, "startup.usda");

  CTX_data_scene_set(C, TfCreateRefPtr(new CovahScene(main->stage_id)));
}

void UNI_destroy(cContext &C)
{
  Stage stage = CTX_data_stage(C);
  stage->~UsdStage();
}

void UNI_open_stage(cContext &C)
{
  Main main = CTX_data_stage(C);
  Stage stage = CTX_data_stage(C);

  stage->Open(main->stage_id);
}

void UNI_save_stage(cContext &C)
{
  Stage stage = CTX_data_stage(C);
  stage->GetRootLayer()->Save();
}

void UNI_set_defaults(cContext &C)
{
  /* ----- */

  Stage stage = CTX_data_stage(C);
  wmWindow win = TfCreateRefPtr(new CovahWindow(C));

  /* ----- */

  /** Default Window. */
  win->title.Set(TfToken("Covah"));
  win->dpi.Set(1.0f);
  win->dpifac.Set(1.0f);
  win->widgetunit.Set(20.0f);
  win->scale.Set(1.0f);
  win->linewidth.Set(1.0f);
  win->pixelsz.Set(1.0f);
  win->icon.Set(SdfAssetPath(CLI_icon(ICON_COVAH)));
  win->state.Set(UsdUITokens->maximized);
  win->cursor.Set(UsdUITokens->default_);
  win->alignment.Set(UsdUITokens->alignAbsolute);
  win->pos.Set(GfVec2f(0.0f, 0.0f));
  win->size.Set(GfVec2f(1920, 1080));
  win->type.Set(TfToken(UsdUITokens->normal));
  win->workspace_rel.AddTarget(win->prims.workspace->path);

  /* ----- */

  /** Default Viewport. */
  Area v3d = TfCreateRefPtr(new CovahArea(C, win->prims.screen, SdfPath("View3D")));
  v3d->name.Set(TfToken("View3D"));
  v3d->icon.Set(SdfAssetPath(CLI_icon(ICON_HYDRA)));
  v3d->pos.Set(GfVec2f(0, 0));
  v3d->size.Set(GfVec2f(1800, 1080));

  /* ----- */

  /** Default Outliner. */
  Area outliner = TfCreateRefPtr(new CovahArea(C, win->prims.screen, SdfPath("Outliner")));
  v3d->name.Set(TfToken("Outliner"));
  v3d->icon.Set(SdfAssetPath(CLI_icon(ICON_LUXO)));
  v3d->pos.Set(GfVec2f(1800, 0));
  v3d->size.Set(GfVec2f(120, 1080));

  /* ----- */

  /** Add UI Areas to Screen's Collection of Areas. */
  win->prims.screen->align.Set(UsdUITokens->verticalSplit);
  win->prims.screen->areas_rel.AddTarget(v3d->path);
  win->prims.screen->areas_rel.AddTarget(outliner->path);

  /** Add this screen to our default 'Layout' WorkSpace. */
  win->prims.workspace->name.Set(TfToken("Layout"));
  win->prims.workspace->screen_rel.AddTarget(win->prims.screen->path);

  /* ----- */

  CTX_wm_window_set(C, win);

  wmWindowManager wm = TfCreateRefPtr(new CovahWindowManager);
  wm->windows.insert(std::make_pair(win->path, win));
  CTX_wm_manager_set(C, wm);
}

void UNI_author_default_scene(cContext &C)
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

WABI_NAMESPACE_END