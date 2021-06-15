/*
 * Copyright 2021 Pixar. All Rights Reserved.
 *
 * Portions of this file are derived from original work by Pixar
 * distributed with Universal Scene Description, a project of the
 * Academy Software Foundation (ASWF). https://www.aswf.io/
 *
 * Licensed under the Apache License, Version 2.0 (the "Apache License")
 * with the following modification; you may not use this file except in
 * compliance with the Apache License and the following modification:
 * Section 6. Trademarks. is deleted and replaced with:
 *
 * 6. Trademarks. This License does not grant permission to use the trade
 *    names, trademarks, service marks, or product names of the Licensor
 *    and its affiliates, except as required to comply with Section 4(c)
 *    of the License and to reproduce the content of the NOTICE file.
 *
 * You may obtain a copy of the Apache License at:
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the Apache License with the above modification is
 * distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF
 * ANY KIND, either express or implied. See the Apache License for the
 * specific language governing permissions and limitations under the
 * Apache License.
 *
 * Modifications copyright (C) 2020-2021 Wabi.
 */
#include "wabi/usdImaging/usdAppUtils/camera.h"
#include "wabi/wabi.h"

#include "wabi/base/tf/diagnostic.h"
#include "wabi/usd/sdf/path.h"
#include "wabi/usd/usd/prim.h"
#include "wabi/usd/usd/primFlags.h"
#include "wabi/usd/usd/primRange.h"
#include "wabi/usd/usd/stage.h"
#include "wabi/usd/usdGeom/camera.h"

WABI_NAMESPACE_BEGIN

UsdGeomCamera UsdAppUtilsGetCameraAtPath(const UsdStagePtr &stage, const SdfPath &cameraPath)
{
  if (!stage) {
    TF_CODING_ERROR("Invalid stage");
    return UsdGeomCamera();
  }

  if (!cameraPath.IsPrimPath()) {
    // A non-prim path cannot be a camera.
    return UsdGeomCamera();
  }

  SdfPath usdCameraPath = cameraPath;

  if (!cameraPath.IsAbsolutePath()) {
    if (cameraPath.GetPathElementCount() > 1u) {
      // XXX: Perhaps we should error here? For now we coerce the camera
      // path to be absolute using the absolute root path and print a
      // warning.
      usdCameraPath = cameraPath.MakeAbsolutePath(SdfPath::AbsoluteRootPath());
      TF_WARN(
        "Camera path \"%s\" is not absolute. Using absolute path "
        "instead: \"%s\"",
        cameraPath.GetText(),
        usdCameraPath.GetText());
    }
    else {
      // Search for the camera by name.
      UsdPrimRange primRange = UsdPrimRange::Stage(stage, UsdTraverseInstanceProxies());

      for (const UsdPrim &usdPrim : primRange) {
        if (usdPrim.GetName() == cameraPath.GetNameToken()) {
          const UsdGeomCamera usdCamera(usdPrim);
          if (usdCamera) {
            return usdCamera;
          }
        }
      }
    }
  }

  return UsdGeomCamera::Get(stage, usdCameraPath);
}

WABI_NAMESPACE_END
