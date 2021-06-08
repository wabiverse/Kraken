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
#ifndef WABI_USD_IMAGING_USD_APP_UTILS_CAMERA_H
#define WABI_USD_IMAGING_USD_APP_UTILS_CAMERA_H

/// \file usdAppUtils/camera.h
///
/// Collection of module-scoped utilities for applications that operate using
/// USD cameras.

#include "wabi/usdImaging/usdAppUtils/api.h"
#include "wabi/wabi.h"

#include "wabi/usd/sdf/path.h"
#include "wabi/usd/usd/stage.h"
#include "wabi/usd/usdGeom/camera.h"

WABI_NAMESPACE_BEGIN

/// Gets the UsdGeomCamera matching \p cameraPath from the USD stage \p stage.
///
/// If \p cameraPath is an absolute path, this is equivalent to
/// UsdGeomCamera::Get(). Otherwise, if \p cameraPath is a single-element path
/// representing just the name of a camera prim, then \p stage will be searched
/// looking for a UsdGeomCamera matching that name. The UsdGeomCamera schema
/// for that prim will be returned if found, or an invalid UsdGeomCamera will
/// be returned if not.
///
/// Note that if \p cameraPath is a multi-element path, a warning is issued and
/// it is just made absolute using the absolute root path before searching. In
/// the future, this could potentially be changed to use a suffix-based match.
USDAPPUTILS_API
UsdGeomCamera UsdAppUtilsGetCameraAtPath(const UsdStagePtr &stage, const SdfPath &cameraPath);

WABI_NAMESPACE_END

#endif
