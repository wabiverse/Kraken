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

#include <wabi/usd/usd/stage.h>
#include <wabi/wabi.h>

#include <wabi/usd/usdGeom/cube.h>
#include <wabi/usd/usdGeom/gprim.h>

#include <wabi/usd/usdUI/window.h>

WABI_NAMESPACE_USING

Universe UNI;

void UNI_create_stage(std::string project_file)
{
  UNI.stage = UsdStage::CreateNew(project_file);
  UNI.stage->GetRootLayer()->SetDocumentation(std::string("COVAH v") +
                                              UNI.system.version.covah_version);
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

  TF_CODING_ERROR("Attempted to access invalid UsdStage.");
  exit(COVAH_ERROR);
}

void UNI_author_gui()
{
  // UsdUIWindow window = UsdUIWindow::Define(UNI.stage, SdfPath("/Window"));
  // window.CreateTitleAttr(VtValue(TfToken("COVAH")));
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
