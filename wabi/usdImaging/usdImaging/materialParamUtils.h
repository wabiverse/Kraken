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
#ifndef WABI_USD_IMAGING_MATERIAL_PARAM_UTILS_H
#define WABI_USD_IMAGING_MATERIAL_PARAM_UTILS_H

#include "wabi/wabi.h"

#include "wabi/base/tf/token.h"

WABI_NAMESPACE_BEGIN

struct HdMaterialNetworkMap;
class UsdAttribute;
class UsdPrim;
class UsdTimeCode;
class VtValue;

/// Get the value from the usd attribute at given time. If it is an
/// SdfAssetPath containing a UDIM pattern, e.g., //SHOW/myImage.<UDIM>.exr,
/// the resolved path of the SdfAssetPath will be updated to a file path
/// with a UDIM pattern, e.g., /filePath/myImage.<UDIM>.exr.
/// There might be support for different patterns, e.g., myImage._MAPID_.exr,
/// but this function always normalizes it to myImage.<UDIM>.exr.
///
/// The function assumes that the correct ArResolverContext is bound.
///
VtValue UsdImaging_ResolveMaterialParamValue(const UsdAttribute &attr, const UsdTimeCode &time);

/// Builds an HdMaterialNetwork for the usdTerminal prim and
/// populates it in the materialNetworkMap under the terminalIdentifier.
/// This shared implementation is usable for populating material networks for
/// any connectable source including lights and light filters in addition to
/// materials.
void UsdImaging_BuildHdMaterialNetworkFromTerminal(UsdPrim const &usdTerminal,
                                                   TfToken const &terminalIdentifier,
                                                   TfTokenVector const &shaderSourceTypes,
                                                   HdMaterialNetworkMap *materialNetworkMap,
                                                   UsdTimeCode time);

/// Returns whether the material network built by
/// UsdImaging_BuildHdMaterialNetworkFromTerminal for the given usdTerminal
/// prim is time varying.
bool UsdImaging_IsHdMaterialNetworkTimeVarying(UsdPrim const &usdTerminal);

WABI_NAMESPACE_END

#endif  // WABI_USD_IMAGING_MATERIAL_PARAM_UTILS_H
