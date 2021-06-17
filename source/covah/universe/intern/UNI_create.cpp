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
#include "UNI_context.h"
#include "UNI_scene.h"
#include "UNI_window.h"

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

WABI_NAMESPACE_USING

Universe UNI;

void UNI_create_stage(std::string project_file)
{
  UNI.stage = UsdStage::CreateNew(project_file);
  UNI.stage->GetRootLayer()->SetDocumentation(std::string("Covah v") + UNI.system.version.covah_version);
  strcpy(UNI.system.paths.stage_path, project_file.c_str());
}

void UNI_destroy()
{
  UNI.stage->~UsdStage();
}

void UNI_open_stage(std::string project_file)
{
  UNI.stage = UsdStage::Open(project_file);
  strcpy(UNI.system.paths.stage_path, project_file.c_str());
}

void UNI_save_stage()
{
  UNI.stage->GetRootLayer()->Save();
}

const SdfPath &UNI_stage_root()
{
  if (TF_VERIFY(UNI.stage)) {
    static const SdfPath &path = UNI.stage->GetPseudoRoot().GetPath();
    return path;
  }

  /**
   * Create startup stage if user's stage is invalid. */
  TF_CODING_ERROR("Attempted to access invalid UsdStage.");
  UNI_create_stage(TfStringCatPaths(G.main->temp_dir, "startup.usda"));
  return UNI.stage->GetPseudoRoot().GetPath();
}

void UNI_on_ctx(cContext *C)
{
  /* ----- */

  /** Store Properties for C. */
  Scene *cscene = new Scene();
  wmWindow *win = new wmWindow();
  wmWindowManager *wm = new wmWindowManager();

  /* ----- */

  /** Default Stage Paths. */
  SdfPath window_path = SdfPath("/Covah/Windows/MainWindow");
  SdfPath workspace_path = window_path.AppendPath(SdfPath("WorkSpaces/Layout"));
  SdfPath screen_path = workspace_path.AppendPath(SdfPath("Screen"));
  SdfPath v3d_path = screen_path.AppendPath(SdfPath("View3D"));
  SdfPath outliner_path = screen_path.AppendPath(SdfPath("Outliner"));

  /* ----- */

  /** Default Window. */
  UsdUIWindow window = UsdUIWindow::Define(UNI.stage, window_path);
  win->title = window.CreateTitleAttr(VtValue(TfToken("Covah")));
  win->dpi = window.CreateDpiAttr(VtValue(float(1)));
  win->dpifac = window.CreateDpifacAttr(VtValue(float(1)));
  win->widgetunit = window.CreateWidgetunitAttr(VtValue(float(20)));
  win->scale = window.CreateScaleAttr(VtValue(float(1)));
  win->linewidth = window.CreateLinewidthAttr(VtValue(float(1)));
  win->pixelsz = window.CreatePixelszAttr(VtValue(float(1)));
  win->icon = window.CreateIconAttr(VtValue(SdfAssetPath(CLI_icon(ICON_COVAH))));
  win->state = window.CreateStateAttr(VtValue(UsdUITokens->maximized));
  win->cursor = window.CreateCursorAttr(VtValue(UsdUITokens->default_));
  win->pos = window.CreatePosAttr(VtValue(GfVec2f(0, 0)));
  win->size = window.CreateSizeAttr(VtValue(GfVec2f(1920, 1080)));
  win->type = window.CreateTypeAttr(VtValue(TfToken(UsdUITokens->normal)));
  window.CreateUiWindowWorkspaceRel().AddTarget(workspace_path);

  /* ----- */

  /** Default 'Layout' WorkSpace. */
  UsdUIWorkspace workspace = UsdUIWorkspace::Define(UNI.stage, workspace_path);
  win->workspace = workspace.CreateNameAttr(VtValue(TfToken("Layout")));
  workspace.CreateScreenRel().AddTarget(screen_path);

  /** Default Viewport. */
  UsdUIArea v3d = UsdUIArea::Define(UNI.stage, v3d_path);
  v3d.CreateNameAttr(VtValue(TfToken("View3D")));
  v3d.CreateIconAttr(VtValue(SdfAssetPath(CLI_icon(ICON_HYDRA))));
  v3d.CreatePosAttr(VtValue(GfVec2f(0, 0)));
  v3d.CreateSizeAttr(VtValue(GfVec2f(1800, 1080)));

  /* ----- */

  /** Default Outliner. */
  UsdUIArea outliner = UsdUIArea::Define(UNI.stage, outliner_path);
  outliner.CreateNameAttr(VtValue(TfToken("Outliner")));
  outliner.CreateIconAttr(VtValue(SdfAssetPath(CLI_icon(ICON_LUXO))));
  outliner.CreatePosAttr(VtValue(GfVec2f(1800, 0)));
  outliner.CreateSizeAttr(VtValue(GfVec2f(120, 1080)));

  /* ----- */

  /** Add UI Areas to Screen's Collection of Areas. */
  UsdUIScreen screen = UsdUIScreen::Define(UNI.stage, screen_path);
  win->align = screen.CreateAlignmentAttr(VtValue(UsdUITokens->verticalSplit));
  win->screen = screen.CreateAreasRel();
  win->screen.AddTarget(v3d_path);
  win->screen.AddTarget(outliner_path);

  /* ----- */

  /** Setup C */

  cscene->stage = UNI.stage;
  CTX_data_scene_set(C, cscene);

  CTX_wm_window_set(C, win);

  /** Hash the stage path to this window. */
  wm->windows.insert(std::make_pair(TfToken(UNI.system.paths.stage_path), win));
  CTX_wm_manager_set(C, wm);
}

void UNI_author_default_scene()
{
  UsdGeomCube cube = UsdGeomCube::Define(UNI.stage, SdfPath("/Cube"));

  UsdGeomXformOp location = cube.AddTranslateOp();
  location.Set(VtValue(GfVec3d(0.0f, 0.0f, 0.0f)));
  UsdGeomXformOp rotation = cube.AddRotateXYZOp();
  rotation.Set(VtValue(GfVec3f(0.0f, 0.0f, 0.0f)));
  UsdGeomXformOp scale = cube.AddScaleOp();
  scale.Set(VtValue(GfVec3f(1.0f, 1.0f, 1.0f)));
}

void UNI_create(std::string exe_path,
                std::string temp_dir,
                std::string styles_path,
                std::string icons_path,
                std::string datafiles_path,
                std::filesystem::path stage_id,
                uint64_t build_commit_timestamp,
                std::string build_hash,
                std::string covah_version)
{
  /** Create the system paths. */
  strcpy(UNI.system.paths.exe_path, exe_path.c_str());
  strcpy(UNI.system.paths.temp_dir, temp_dir.c_str());
  strcpy(UNI.system.paths.datafiles_path, datafiles_path.c_str());
  strcpy(UNI.system.paths.styles_path, styles_path.c_str());
  strcpy(UNI.system.paths.icons_path, icons_path.c_str());
  strcpy(UNI.system.paths.stage_path, stage_id.string().c_str());

  /** Create the system info. */
  std::string pixar_version = std::string(std::to_string(WABI_VERSION_MINOR) + "." +
                                          std::to_string(WABI_VERSION_PATCH));
  UNI.system.version.build_commit_timestamp = build_commit_timestamp;
  strcpy(UNI.system.version.build_hash, build_hash.c_str());
  strcpy(UNI.system.version.covah_version, covah_version.c_str());
  strcpy(UNI.system.version.pixar_version, pixar_version.c_str());
}
