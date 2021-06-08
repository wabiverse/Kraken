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
#ifndef WABI_USD_USD_GEOM_TYPES_H
#define WABI_USD_USD_GEOM_TYPES_H

#include "wabi/usd/usd/attribute.h"
#include "wabi/usd/usdGeom/api.h"
#include "wabi/wabi.h"

#include "wabi/base/tf/smallVector.h"

WABI_NAMESPACE_BEGIN

/// \enum UsdGeomAttributeType
///
/// Specifies the type of a shading attribute.
///
enum class UsdGeomAttributeType {
  Invalid,
  Input,
  Output,
};

/// \enum UsdGeomConnectionModification
///
/// Choice when creating a single connection with the \p ConnectToSource method
/// for a shading attribute. The new connection can replace any existing
/// connections or be added to the list of existing connections. In which case
/// there is a choice between prepending and appending to said list, which will
/// be represented by Usd's list editing operations.
///
enum class UsdGeomConnectionModification { Replace, Prepend, Append };

/// \typedef UsdGeomAttributeVector
///
/// For performance reasons we want to be extra careful when reporting
/// attributes. It is possible to have multiple connections for a shading
/// attribute, but by far the more common cases are one or no connection. So we
/// use a small vector that can be stack allocated that holds space for a single
/// attributes, but that can "spill" to the heap in the case of multiple
/// upstream attributes.
using UsdGeomAttributeVector = TfSmallVector<UsdAttribute, 1>;

/// \typedef UsdGeomSourceInfoVector
///
/// For performance reasons we want to be extra careful when reporting
/// connections. It is possible to have multiple connections for a shading
/// attribute, but by far the more common cases are one or no connection.
/// So we use a small vector that can be stack allocated that holds space
/// for a single source, but that can "spill" to the heap in the case
/// of a multi-connection.
///
/// /sa UsdGeomConnectionSourceInfo in connectableAPI.h
struct UsdGeomConnectionSourceInfo;
using UsdGeomSourceInfoVector = TfSmallVector<UsdGeomConnectionSourceInfo, 1>;

WABI_NAMESPACE_END

#endif
