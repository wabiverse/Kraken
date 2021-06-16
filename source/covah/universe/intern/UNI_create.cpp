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

#include "CKE_main.h"

#include "CLI_icons.h"

#include <wabi/usd/usd/stage.h>
#include <wabi/wabi.h>

#include <wabi/usd/usdGeom/cube.h>
#include <wabi/usd/usdGeom/gprim.h>

#include <wabi/usd/usdUI/screenAPI.h>
#include <wabi/usd/usdUI/tokens.h>
#include <wabi/usd/usdUI/window.h>
#include <wabi/usd/usdUI/workspace.h>

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

void UNI_author_gui()
{
  /* ----- */

  /** Default Window. */
  SdfPath main_window_path = SdfPath("/Covah/Windows/MainWindow");
  UsdUIWindow window = UsdUIWindow::Define(UNI.stage, main_window_path);
  window.CreateWindowTitleAttr(VtValue(TfToken("Covah")));
  window.CreateWindowIconAttr(VtValue(SdfAssetPath(CLI_icon(ICON_COVAH))));
  window.CreateWindowPosAttr(VtValue(GfVec2f(0, 0)));
  window.CreateWindowSizeAttr(VtValue(GfVec2f(1920, 1080)));
  window.CreateWindowTypeAttr(VtValue(TfToken(UsdUITokens->attached)));

  /* ----- */

  /** Default Workspace :: Layout. */
  SdfPath layout_workspace = SdfPath(main_window_path.AppendPath(SdfPath("Workspaces/Layout")));
  UsdUIWorkspace layout = UsdUIWorkspace::Define(UNI.stage, layout_workspace);
  layout.CreateWorkspaceNameAttr(VtValue(TfToken("Layout")));

  /** Default Workspace :: Layout's Region. */
  UsdUIScreenAPI layout_region = UsdUIScreenAPI::Apply(layout.GetPrim());
  layout_region.CreatePurposeAttr(VtValue(UsdUITokens->region));

  /* ----- */

  /** View3D Region. */
  SdfPath v3d_path = layout_workspace.AppendPath(SdfPath("View3D"));
  SdfPath v3d_header_path = v3d_path.AppendPath(SdfPath("Header"));

  /** Outliner Region. */
  SdfPath outliner_path = layout_workspace.AppendPath(SdfPath("Outliner"));
  SdfPath outliner_header_path = outliner_path.AppendPath(SdfPath("Header"));

  /* ----- */

  /** Viewport Defaults */
  UsdPrim view3d_prim = UNI.stage->DefinePrim(v3d_path);
  UsdUIScreenAPI v3d = UsdUIScreenAPI::Apply(view3d_prim);
  v3d.CreateNameAttr(VtValue(TfToken("View3D")));
  v3d.CreatePosAttr(VtValue(GfVec2f(0, 0)));
  v3d.CreateSizeAttr(VtValue(GfVec2f(1800, 1080)));

  /** Viewport Header Bar. */
  UsdPrim view3d_header_prim = UNI.stage->DefinePrim(v3d_header_path);
  UsdUIScreenAPI v3d_header = UsdUIScreenAPI::Apply(view3d_header_prim);
  /** Header -> TopBar */
  v3d_header.CreateTypeAttr(VtValue(UsdUITokens->topBar));
  v3d_header.CreateIconAttr(VtValue(SdfAssetPath(CLI_icon(ICON_HYDRA))));
  v3d_header.CreateNameAttr(VtValue(TfToken("Hydra View3D")));

  /** Set Type and Purpose. */
  v3d.CreateTypeAttr(VtValue(UsdUITokens->view3D));
  v3d.CreatePurposeAttr(VtValue(UsdUITokens->region));

  /** Set Header Relationship. */
  v3d.CreateLayoutAttr(VtValue(UsdUITokens->horizontalSplit));
  v3d.CreateUiScreenAreaRegionRel().AddTarget(v3d_header_path);

  /* ----- */

  /** Outliner Defaults. */
  UsdPrim outliner_prim = UNI.stage->DefinePrim(outliner_path);
  UsdUIScreenAPI outliner = UsdUIScreenAPI::Apply(outliner_prim);
  outliner.CreateNameAttr(VtValue(TfToken("Outliner")));
  outliner.CreatePosAttr(VtValue(GfVec2f(1800, 0)));
  outliner.CreateSizeAttr(VtValue(GfVec2f(120, 1080)));

  /** Outliner Header Bar. */
  UsdPrim outliner_header_prim = UNI.stage->DefinePrim(outliner_header_path);
  UsdUIScreenAPI outliner_header = UsdUIScreenAPI::Apply(outliner_header_prim);
  /** Header -> TopBar */
  outliner_header.CreateTypeAttr(VtValue(UsdUITokens->topBar));
  outliner_header.CreateIconAttr(VtValue(SdfAssetPath(CLI_icon(ICON_LUXO))));
  outliner_header.CreateNameAttr(VtValue(TfToken("Stage Inspector")));

  /** Set Type and Purpose. */
  outliner.CreateTypeAttr(VtValue(UsdUITokens->outliner));
  outliner.CreatePurposeAttr(VtValue(UsdUITokens->region));

  /** Set Header Relationship. */
  outliner.CreateLayoutAttr(VtValue(UsdUITokens->horizontalSplit));
  outliner.CreateUiScreenAreaRegionRel().AddTarget(outliner_header_path);

  /* ----- */

  /**
   * And add these to our Layout Workspace. */

  /** Add Viewport. */
  v3d.CreateWorkspaceRel().AddTarget(layout_workspace);

  /** Vertical Split Between View3D and Outliner. */
  layout_region.CreateLayoutAttr(VtValue(UsdUITokens->verticalSplit));

  /** Add Outliner. */
  outliner.CreateWorkspaceRel().AddTarget(layout_workspace);
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
