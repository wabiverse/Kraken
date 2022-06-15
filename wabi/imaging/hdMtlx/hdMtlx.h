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

#ifndef WABI_IMAGING_HD_MTLX_HDMTLX_H
#define WABI_IMAGING_HD_MTLX_HDMTLX_H

#include "wabi/imaging/hdMtlx/api.h"
#include "wabi/wabi.h"
#include <memory>
#include <set>
#include <unordered_map>

namespace MaterialX
{
  using DocumentPtr = std::shared_ptr<class Document>;
  using StringMap = std::unordered_map<std::string, std::string>;
}  // namespace MaterialX

WABI_NAMESPACE_BEGIN

class SdfPath;
class VtValue;
struct HdMaterialNetwork2;
struct HdMaterialNode2;

/**
 * Converts the HdParameterValue to a string MaterialX can understand. */
HDMTLX_API
std::string HdMtlxConvertToString(VtValue const &hdParameterValue);

/**
 * Creates and returns a MaterialX Document from the given HdMaterialNetwork2
 * Collecting the hdTextureNodes as the HdMaterialNetwork2 is traversed as
 * well as the Texture name mapping between MaterialX and Hydra. */
HDMTLX_API
MaterialX::DocumentPtr HdMtlxCreateMtlxDocumentFromHdNetwork(
  HdMaterialNetwork2 const &hdNetwork,
  HdMaterialNode2 const &hdMaterialXNode,
  SdfPath const &materialPath,
  MaterialX::DocumentPtr const &libraries,
  std::set<SdfPath> *hdTextureNodes,
  MaterialX::StringMap *mxHdTextureMap);

WABI_NAMESPACE_END

#endif /* WABI_IMAGING_HD_MTLX_HDMTLX_H */