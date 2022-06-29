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

#include "KLI_icons.h"

#include "KKE_context.h"
#include "KKE_kraken_prim.h"
#include "KKE_main.h"
#include "KKE_scene.h"
#include "KKE_screen.h"

#include "LUXO_runtime.h"

#include "USD_api.h"
#include "USD_area.h"
#include "USD_context.h"
#include "USD_default_tables.h"
#include "USD_factory.h"
#include "USD_scene.h"
#include "USD_screen.h"
#include "USD_userpref.h"
#include "USD_window.h"
#include "USD_workspace.h"

#include <wabi/wabi.h>

#include <wabi/base/vt/value.h>

// #include <wabi/usd/usdUI/area.h>
// #include <wabi/usd/usdUI/screen.h>
#include <wabi/usd/usdUI/tokens.h>
// #include <wabi/usd/usdUI/window.h>
// #include <wabi/usd/usdUI/workspace.h>

#include <wabi/usd/usdGeom/cube.h>
#include <wabi/usd/usdGeom/gprim.h>

#include <wabi/usd/usd/collectionAPI.h>
#include <wabi/usd/usd/stage.h>

#include <wabi/base/plug/plugin.h>
#include <wabi/base/plug/registry.h>

WABI_NAMESPACE_BEGIN

void USD_create_stage(kContext *C)
{
  Main *main = CTX_data_main(C);

  main->stage_id = TfStringCatPaths(main->temp_dir, "startup.usda");

  Scene *scene = new Scene(main->stage_id.string());
  CTX_data_scene_set(C, scene);
}

void USD_destroy(kContext *C)
{
  Stage stage = CTX_data_stage(C);
  stage->~UsdStage();
}

void USD_open_stage(kContext *C)
{
  Main *main = CTX_data_main(C);
  Stage stage = CTX_data_stage(C);

  stage->Open(main->stage_id.string());
}

void USD_save_stage(kContext *C)
{
  Stage stage = CTX_data_stage(C);
  stage->GetRootLayer()->Save();
}

void USD_set_defaults(kContext *C)
{
  /* ----- */

  /** Pixar Stage Initiated. */
  Stage stage = CTX_data_stage(C);

  /* ----- */

  /** Insert into kScreen. */
  // ScrArea *v3d = new ScrArea(C, screen, SdfPath("View3D"));
  // screen->areas.push_back(v3d);

  /** CTX set View3D. */
  // CTX_wm_area_set(C, v3d);

  /** Load the defaults. */
  // USD_default_table_area_v3d(C);

  /* ----- */

  /** Outliner Creation. */
  // ScrArea *outliner = new ScrArea(C, screen, SdfPath("Outliner"));

  /** Insert into kScreen. */
  // screen->areas.push_back(outliner);

  /** CTX set Outliner. */
  // CTX_wm_area_set(C, outliner);

  /** Load the defaults. */
  // USD_default_table_area_outliner(C);

  /* ----- */

  /** Load the kScreen defaults. */
  // USD_default_table_area_screen(C);

  /** Load the WorkSpace defaults. */
  // USD_default_table_area_workspace(C);

  /* ----- */

  /** Load the Default Scene. */
  // USD_default_table_scene_data(C);
}

WABI_NAMESPACE_END