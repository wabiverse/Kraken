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
#ifndef WABI_USD_USD_GEOM_UTILS_H
#define WABI_USD_USD_GEOM_UTILS_H

#include "wabi/usd/usd/attribute.h"
#include "wabi/usd/usdGeom/api.h"
#include "wabi/usd/usdGeom/types.h"
#include "wabi/wabi.h"

#include "wabi/base/tf/smallVector.h"
#include "wabi/base/tf/token.h"

#include <string>
#include <utility>

WABI_NAMESPACE_BEGIN

class UsdGeomInput;
class UsdGeomOutput;

/// \class UsdGeomUtils
///
/// This class contains a set of utility functions used when authoring and
/// querying shading networks.
///
class UsdGeomUtils {
 public:
  /// Returns the namespace prefix of the USD attribute associated with the
  /// given shading attribute type.
  USDGEOM_API
  static std::string GetPrefixForAttributeType(UsdGeomAttributeType sourceType);

  /// Given the full name of a shading attribute, returns it's base name and
  /// shading attribute type.
  USDGEOM_API
  static std::pair<TfToken, UsdGeomAttributeType> GetBaseNameAndType(const TfToken &fullName);

  /// Given the full name of a shading attribute, returns its shading
  /// attribute type.
  USDGEOM_API
  static UsdGeomAttributeType GetType(const TfToken &fullName);

  /// Returns the full shading attribute name given the basename and the
  /// shading attribute type. \p baseName is the name of the input or output
  /// on the shading node. \p type is the \ref UsdGeomAttributeType of the
  /// shading attribute.
  USDGEOM_API
  static TfToken GetFullName(const TfToken &baseName, const UsdGeomAttributeType type);

  /// \brief Find what is connected to an Input or Output recursively
  ///
  /// GetValueProducingAttributes implements the UsdGeom connectivity rules
  /// described in \ref UsdGeomAttributeResolution .
  ///
  /// When tracing connections within networks that contain containers like
  /// UsdGeomNodeGraph nodes, the actual output(s) or value(s) at the end of
  /// an input or output might be multiple connections removed. The methods
  /// below resolves this across multiple physical connections.
  ///
  /// An UsdGeomInput is getting its value from one of these sources:
  /// - If the input is not connected the UsdAttribute for this input is
  /// returned, but only if it has an authored value. The input attribute
  /// itself carries the value for this input.
  /// - If the input is connected we follow the connection(s) until we reach
  /// a valid output of a UsdGeomImageable node or if we reach a valid
  /// UsdGeomInput attribute of a UsdGeomNodeGraph or UsdGeomGprim that
  /// has an authored value.
  ///
  /// An UsdGeomOutput on a container can get its value from the same
  /// type of sources as a UsdGeomInput on either a UsdGeomImageable or
  /// UsdGeomNodeGraph. Outputs on non-containers (UsdGeomImageables) cannot be
  /// connected.
  ///
  /// This function returns a vector of UsdAttributes. The vector is empty if
  /// no valid attribute was found. The type of each attribute can be
  /// determined with the \p UsdGeomUtils::GetType function.
  ///
  /// If \p geomOutputsOnly is true, it will only report attributes that are
  /// outputs of non-containers (UsdGeomImageables). This is a bit faster and
  /// what is need when determining the connections for Material terminals.
  ///
  /// \note This will return the last attribute along the connection chain
  /// that has an authored value, which might not be the last attribute in the
  /// chain itself.
  /// \note When the network contains multi-connections, this function can
  /// return multiple attributes for a single input or output. The list of
  /// attributes is build by a depth-first search, following the underlying
  /// connection paths in order. The list can contain both UsdGeomOutput and
  /// UsdGeomInput attributes. It is up to the caller to decide how to
  /// process such a mixture.
  USDGEOM_API
  static UsdGeomAttributeVector GetValueProducingAttributes(UsdGeomInput const &input,
                                                            bool geomOutputsOnly = false);
  /// \overload
  USDGEOM_API
  static UsdGeomAttributeVector GetValueProducingAttributes(UsdGeomOutput const &output,
                                                            bool geomOutputsOnly = false);
};

WABI_NAMESPACE_END

#endif
