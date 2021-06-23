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
#include "UNI_default_tables.h"
#include "UNI_factory.h"
#include "UNI_scene.h"
#include "UNI_screen.h"
#include "UNI_userpref.h"
#include "UNI_window.h"
#include "UNI_workspace.h"

#include "CKE_context.h"
#include "CKE_main.h"
#include "CKE_screen.h"

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

void UNI_create_stage(cContext *C)
{
  Main *main = CTX_data_main(C);

  main->stage_id = TfStringCatPaths(main->temp_dir, "startup.usda");

  CTX_data_scene_set(C, new Scene(main->stage_id));
}

void UNI_destroy(cContext *C)
{
  Stage stage = CTX_data_stage(C);
  stage->~UsdStage();
}

void UNI_open_stage(cContext *C)
{
  Main *main = CTX_data_main(C);
  Stage stage = CTX_data_stage(C);

  stage->Open(main->stage_id);
}

void UNI_save_stage(cContext *C)
{
  Stage stage = CTX_data_stage(C);
  stage->GetRootLayer()->Save();
}

void UNI_set_defaults(cContext *C)
{
  /* ----- */

  /** Pixar Stage Initiated. */
  Stage stage = CTX_data_stage(C);

  /* ----- */

  /** WindowManager Creation. */
  wmWindowManager *wm = new wmWindowManager();

  /** CTX set WindowManager. */
  CTX_wm_manager_set(C, wm);

  /* ----- */

  /** Window Creation. */
  wmWindow *win = new wmWindow(C, SdfPath(COVAH_PATH_DEFAULTS::COVAH_WINDOW));

  /** CTX set Window. */
  CTX_wm_window_set(C, win);

  /** Insert into WindowManager. */
  UNIVERSE_INSERT_WINDOW(wm, win->path, win);

  /** Load the defaults. */
  UNI_default_table_main_window(C);

  /* ----- */

  /** User Preferences Creation. */
  UserDef *uprefs = new UserDef(C);

  /** CTX set User Preferences. */
  CTX_data_prefs_set(C, uprefs);

  /** Load the defaults. */
  UNI_default_table_user_prefs(C);

  /* ----- */

  /** CTX get cScreen. */
  cScreen *screen = CTX_wm_screen(C);

  /* ----- */

  /** View3D Creation. */
  ScrArea *v3d = new ScrArea(C, screen, SdfPath("View3D"));

  /** Insert into cScreen. */
  screen->areas.push_back(v3d);

  /** CTX set View3D. */
  CTX_wm_area_set(C, v3d);

  /** Load the defaults. */
  UNI_default_table_area_v3d(C);

  /* ----- */

  /** Outliner Creation. */
  ScrArea *outliner = new ScrArea(C, screen, SdfPath("Outliner"));

  /** Insert into cScreen. */
  screen->areas.push_back(outliner);

  /** CTX set Outliner. */
  CTX_wm_area_set(C, outliner);

  /** Load the defaults. */
  UNI_default_table_area_outliner(C);

  /* ----- */

  /** Load the cScreen defaults. */
  UNI_default_table_area_screen(C);

  /** Load the WorkSpace defaults. */
  UNI_default_table_area_workspace(C);

  /* ----- */

  /** Load the Default Scene. */
  UNI_default_table_scene_data(C);
}

WABI_NAMESPACE_END