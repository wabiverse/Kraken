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
#ifndef WABI_USD_USD_GEOM_CONNECTABLE_BEHAVIOR_H
#define WABI_USD_USD_GEOM_CONNECTABLE_BEHAVIOR_H

/// \file usdGeom/connectableAPIBehavior.h

#include "wabi/usd/usdGeom/api.h"
#include "wabi/wabi.h"

#include "wabi/base/gf/vec3f.h"
#include "wabi/base/vt/array.h"

#include "wabi/base/tf/type.h"

WABI_NAMESPACE_BEGIN

class UsdAttribute;
class UsdGeomInput;
class UsdGeomOutput;

/// UsdGeomConnectableAPIBehavior defines the compatibilty and behavior
/// UsdGeomConnectableAPIof when applied to a particular prim type.
///
/// This enables schema libraries to enable UsdGeomConnectableAPI for
/// their prim types and define its behavior.
class UsdGeomConnectableAPIBehavior {
 public:
  /// An enum describing the types of connectable nodes which will govern what
  /// connectibility rule is invoked for these.
  enum ConnectableNodeTypes {
    BasicNodes,             // Geom, NodeGraph
    DerivedContainerNodes,  // Material, etc
  };

  USDGEOM_API
  virtual ~UsdGeomConnectableAPIBehavior();

  /// The prim owning the input is guaranteed to be of the type this
  /// behavior was registered with. The function must be thread-safe.
  ///
  /// It should return true if the connection is allowed, false
  /// otherwise. If the connection is prohibited and \p reason is
  /// non-NULL, it should be set to a user-facing description of the
  /// reason the connection is prohibited.
  ///
  /// The base implementation checks that the input is defined; that
  /// the source attribute exists; and that the connectability metadata
  /// on the input allows a connection from the attribute -- see
  /// UsdGeomInput::GetConnectability().
  ///
  USDGEOM_API
  virtual bool CanConnectInputToSource(const UsdGeomInput &,
                                       const UsdAttribute &,
                                       std::string *reason);

  /// The prim owning the output is guaranteed to be of the type this
  /// behavior was registered with. The function must be thread-safe.
  ///
  /// It should return true if the connection is allowed, false
  /// otherwise. If the connection is prohibited and \p reason is
  /// non-NULL, it should be set to a user-facing description of the
  /// reason the connection is prohibited.
  ///
  /// The base implementation returns false. Outputs of most prim
  /// types will be defined by the underlying node definition (see
  /// UsdGeomNodeDefAPI), not a connection.
  ///
  USDGEOM_API
  virtual bool CanConnectOutputToSource(const UsdGeomOutput &,
                                        const UsdAttribute &,
                                        std::string *reason);

  /// The prim owning the output is guaranteed to be of the type this
  /// behavior was registered with. The function must be thread-safe.
  ///
  /// It should return true if the associated prim type is considered
  /// a "container" for connected nodes.
  USDGEOM_API
  virtual bool IsContainer() const;

 protected:
  /// Helper function to separate and share special connectivity logic for
  /// specialized, NodeGraph-derived nodes, like Material (and other in other
  /// domains) that allow their inputs to be connected to an output of a
  /// source that they directly contain/encapsulate. The default behavior is
  /// for Geom Nodes or NodeGraphs which allow their input connections to
  /// output of a sibling source, both encapsulated by the same container
  /// node.
  USDGEOM_API
  bool _CanConnectInputToSource(const UsdGeomInput &,
                                const UsdAttribute &,
                                std::string *reason,
                                ConnectableNodeTypes nodeType = ConnectableNodeTypes::BasicNodes);

  USDGEOM_API
  bool _CanConnectOutputToSource(const UsdGeomOutput &,
                                 const UsdAttribute &,
                                 std::string *reason,
                                 ConnectableNodeTypes nodeType = ConnectableNodeTypes::BasicNodes);
};

/// Registers \p behavior to define connectability of attributes for \p PrimType.
///
/// Plugins should call this function in a TF_REGISTRY_FUNCTION.  For example:
///
/// \code
/// class MyBehavior : public UsdGeomConnectableAPIBehavior { ... }
///
/// TF_REGISTRY_FUNCTION(UsdGeomConnectableAPI)
/// {
///     UsdGeomRegisterConnectableAPIBehavior<MyPrim, MyBehavior>();
/// }
/// \endcode
///
/// Plugins must also note that UsdGeomConnectableAPI behavior is implemented
/// for a prim type in that type's schema definnition.  For example:
///
/// \code
/// class "MyPrim" (
///     ...
///     customData = {
///         dictionary extraPlugInfo = {
///             bool implementsUsdGeomConnectableAPIBehavior = true
///         }
///     }
///     ...
/// )
/// { ... }
/// \endcode
///
/// This allows the plugin system to discover this behavior dynamically
/// and load the plugin if needed.
template<class PrimType, class BehaviorType = UsdGeomConnectableAPIBehavior>
inline void UsdGeomRegisterConnectableAPIBehavior()
{
  UsdGeomRegisterConnectableAPIBehavior(
      TfType::Find<PrimType>(), std::shared_ptr<UsdGeomConnectableAPIBehavior>(new BehaviorType));
}

/// Registers \p behavior to define connectability of attributes for
/// \p PrimType.
USDGEOM_API
void UsdGeomRegisterConnectableAPIBehavior(
    const TfType &connectablePrimType,
    const std::shared_ptr<UsdGeomConnectableAPIBehavior> &behavior);

WABI_NAMESPACE_END

#endif  // WABI_USD_USD_GEOM_CONNECTABLE_BEHAVIOR_H
