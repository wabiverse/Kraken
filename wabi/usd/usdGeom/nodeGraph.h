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
#ifndef USDGEOM_GENERATED_NODEGRAPH_H
#define USDGEOM_GENERATED_NODEGRAPH_H

/// \file usdGeom/nodeGraph.h

#include "wabi/usd/usd/prim.h"
#include "wabi/usd/usd/stage.h"
#include "wabi/usd/usd/typed.h"
#include "wabi/usd/usdGeom/api.h"
#include "wabi/wabi.h"

#include "wabi/usd/usd/editTarget.h"
#include "wabi/usd/usd/relationship.h"
#include "wabi/usd/usdGeom/connectableAPIBehavior.h"
#include "wabi/usd/usdGeom/gprim.h"
#include "wabi/usd/usdGeom/imageable.h"
#include "wabi/usd/usdGeom/input.h"
#include "wabi/usd/usdGeom/output.h"
#include <utility>

#include "wabi/base/vt/value.h"

#include "wabi/base/gf/matrix4d.h"
#include "wabi/base/gf/vec3d.h"
#include "wabi/base/gf/vec3f.h"

#include "wabi/base/tf/token.h"
#include "wabi/base/tf/type.h"

WABI_NAMESPACE_BEGIN

class SdfAssetPath;

// -------------------------------------------------------------------------- //
// NODEGRAPH                                                                  //
// -------------------------------------------------------------------------- //

/// \class UsdGeomNodeGraph
///
/// A node-graph is a container for geom nodes, as well as other
/// node-graphs. It has a public input interface and provides a list of public
/// outputs.
///
/// <b>Node Graph Interfaces</b>
///
/// One of the most important functions of a node-graph is to host the "interface"
/// with which clients of already-built geom networks will interact.  Please
/// see \ref UsdGeomNodeGraph_Interfaces "Interface Inputs" for a detailed
/// explanation of what the interface provides, and how to construct and
/// use it, to effectively share/instance geom networks.
///
/// <b>Node Graph Outputs</b>
///
/// These behave like outputs on a geom and are typically connected to an
/// output on a geom inside the node-graph.
///
///
class UsdGeomNodeGraph : public UsdTyped {
 public:
  /// Compile time constant representing what kind of schema this class is.
  ///
  /// \sa UsdSchemaKind
  static const UsdSchemaKind schemaKind = UsdSchemaKind::ConcreteTyped;

  /// \deprecated
  /// Same as schemaKind, provided to maintain temporary backward
  /// compatibility with older generated schemas.
  static const UsdSchemaKind schemaType = UsdSchemaKind::ConcreteTyped;

  /// Construct a UsdGeomNodeGraph on UsdPrim \p prim .
  /// Equivalent to UsdGeomNodeGraph::Get(prim.GetStage(), prim.GetPath())
  /// for a \em valid \p prim, but will not immediately throw an error for
  /// an invalid \p prim
  explicit UsdGeomNodeGraph(const UsdPrim &prim = UsdPrim()) : UsdTyped(prim)
  {}

  /// Construct a UsdGeomNodeGraph on the prim held by \p schemaObj .
  /// Should be preferred over UsdGeomNodeGraph(schemaObj.GetPrim()),
  /// as it preserves SchemaBase state.
  explicit UsdGeomNodeGraph(const UsdSchemaBase &schemaObj) : UsdTyped(schemaObj)
  {}

  /// Destructor.
  USDGEOM_API
  virtual ~UsdGeomNodeGraph();

  /// Return a vector of names of all pre-declared attributes for this schema
  /// class and all its ancestor classes.  Does not include attributes that
  /// may be authored by custom/extended methods of the schemas involved.
  USDGEOM_API
  static const TfTokenVector &GetSchemaAttributeNames(bool includeInherited = true);

  /// Return a UsdGeomNodeGraph holding the prim adhering to this
  /// schema at \p path on \p stage.  If no prim exists at \p path on
  /// \p stage, or if the prim at that path does not adhere to this schema,
  /// return an invalid schema object.  This is shorthand for the following:
  ///
  /// \code
  /// UsdGeomNodeGraph(stage->GetPrimAtPath(path));
  /// \endcode
  ///
  USDGEOM_API
  static UsdGeomNodeGraph Get(const UsdStagePtr &stage, const SdfPath &path);

  /// Attempt to ensure a \a UsdPrim adhering to this schema at \p path
  /// is defined (according to UsdPrim::IsDefined()) on this stage.
  ///
  /// If a prim adhering to this schema at \p path is already defined on this
  /// stage, return that prim.  Otherwise author an \a SdfPrimSpec with
  /// \a specifier == \a SdfSpecifierDef and this schema's prim type name for
  /// the prim at \p path at the current EditTarget.  Author \a SdfPrimSpec s
  /// with \p specifier == \a SdfSpecifierDef and empty typeName at the
  /// current EditTarget for any nonexistent, or existing but not \a Defined
  /// ancestors.
  ///
  /// The given \a path must be an absolute prim path that does not contain
  /// any variant selections.
  ///
  /// If it is impossible to author any of the necessary PrimSpecs, (for
  /// example, in case \a path cannot map to the current UsdEditTarget's
  /// namespace) issue an error and return an invalid \a UsdPrim.
  ///
  /// Note that this method may return a defined prim whose typeName does not
  /// specify this schema class, in case a stronger typeName opinion overrides
  /// the opinion at the current EditTarget.
  ///
  USDGEOM_API
  static UsdGeomNodeGraph Define(const UsdStagePtr &stage, const SdfPath &path);

 protected:
  /// Returns the kind of schema this class belongs to.
  ///
  /// \sa UsdSchemaKind
  USDGEOM_API
  UsdSchemaKind _GetSchemaKind() const override;

  /// \deprecated
  /// Same as _GetSchemaKind, provided to maintain temporary backward
  /// compatibility with older generated schemas.
  USDGEOM_API
  UsdSchemaKind _GetSchemaType() const override;

 private:
  // needs to invoke _GetStaticTfType.
  friend class UsdSchemaRegistry;
  USDGEOM_API
  static const TfType &_GetStaticTfType();

  static bool _IsTypedSchema();

  // override SchemaBase virtuals.
  USDGEOM_API
  const TfType &_GetTfType() const override;

 public:
  // ===================================================================== //
  // Feel free to add custom code below this line, it will be preserved by
  // the code generator.
  //
  // Just remember to:
  //  - Close the class declaration with };
  //  - Close the namespace with WABI_NAMESPACE_END
  //  - Close the include guard with #endif
  // ===================================================================== //
  // --(BEGIN CUSTOM CODE)--

  /// Constructor that takes a GeomConnectableAPI object.
  /// Allow implicit (auto) conversion of UsdGeomNodeGraph to
  /// UsdGeomConnectableAPI, so that a NodeGraph can be passed into any
  /// function that accepts a GeomConnectableAPI.
  USDGEOM_API
  UsdGeomNodeGraph(const UsdGeomConnectableAPI &connectable);

  /// Contructs and returns a UsdGeomConnectableAPI object with this
  /// node-graph.
  ///
  /// Note that most tasks can be accomplished without explicitly constructing
  /// a UsdGeomConnectable API, since connection-related API such as
  /// UsdGeomConnectableAPI::ConnectToSource() are static methods, and
  /// UsdGeomNodeGraph will auto-convert to a UsdGeomConnectableAPI when
  /// passed to functions that want to act generically on a connectable
  /// UsdGeomConnectableAPI object.
  USDGEOM_API
  UsdGeomConnectableAPI ConnectableAPI() const;

  /// \anchor UsdGeomNodeGraph_Output
  /// \name Outputs of a node-graph. These typically connect to outputs of
  /// geoms or nested node-graphs within the node-graph.
  ///
  /// @{

  /// Create an output which can either have a value or can be connected.
  /// The attribute representing the output is created in the "outputs:"
  /// namespace.
  ///
  USDGEOM_API
  UsdGeomOutput CreateOutput(const TfToken &name, const SdfValueTypeName &typeName) const;

  /// Return the requested output if it exists.
  ///
  USDGEOM_API
  UsdGeomOutput GetOutput(const TfToken &name) const;

  /// Outputs are represented by attributes in the "outputs:" namespace.
  /// If \p onlyAuthored is true (the default), then only return authored
  /// attributes; otherwise, this also returns un-authored builtins.
  ///
  USDGEOM_API
  std::vector<UsdGeomOutput> GetOutputs(bool onlyAuthored = true) const;

  /// \deprecated in favor of GetValueProducingAttributes on UsdGeomOutput
  /// Resolves the connection source of the requested output, identified by
  /// \p outputName to a geom output.
  ///
  /// \p sourceName is an output parameter that is set to the name of the
  /// resolved output, if the node-graph output is connected to a valid
  /// geom source.
  ///
  /// \p sourceType is an output parameter that is set to the type of the
  /// resolved output, if the node-graph output is connected to a valid
  /// geom source.
  ///
  /// \return Returns a valid geom object if the specified output exists and
  /// is connected to one. Return an empty geom object otherwise.
  /// The python version of this method returns a tuple containing three
  /// elements (the source geom, sourceName, sourceType).
  USDGEOM_API
  UsdGeomImageable ComputeOutputSource(const TfToken &outputName,
                                       TfToken *sourceName,
                                       UsdGeomAttributeType *sourceType) const;

  /// @}

  /// \anchor UsdGeomNodeGraph_Interfaces
  /// \name Interface inputs of a node-graph.
  ///
  /// In addition to serving as the "head" for all of the geom networks
  /// that describe each render target's particular node-graph, the node-graph
  /// prim provides a unified "interface" that allows node-graphs to share
  /// geom networks while retaining the ability for each to specify its own
  /// set of unique values for the interface inputs that users may need to
  /// modify.
  ///
  /// A "Node-graph Interface" is a combination of:
  /// \li a flat collection of attributes, of arbitrary names
  /// \li for each such attribute, a list of UsdGeomInput targets
  /// whose attributes on Geom prims should be driven by the interface
  /// input.
  ///
  /// A single interface input can drive multiple geom inputs and be
  /// consumed by multiple render targets. The set of interface inputs itself
  /// is intentionally flat, to encourage sharing of the interface between
  /// render targets.  Clients are always free to create interface inputs with
  /// namespacing to segregate "private" attributes exclusive to the render
  /// target, but we hope this will be an exception.
  ///
  /// To facilitate connecting, qualifying, and interrogating interface
  /// attributes, we use the attribute schema UsdGeomInput, which also
  /// serves as an abstraction for geom inputs.
  ///
  /// <b>Scoped Interfaces</b>
  ///
  /// \todo describe scoped interfaces and fix bug/108940 to account for them.
  ///
  /// @{

  /// Create an Input which can either have a value or can be connected.
  /// The attribute representing the input is created in the "inputs:"
  /// namespace.
  ///
  /// \todo clarify error behavior if typeName does not match existing,
  /// defined attribute - should match UsdPrim::CreateAttribute - bug/108970
  ///
  USDGEOM_API
  UsdGeomInput CreateInput(const TfToken &name, const SdfValueTypeName &typeName) const;

  /// Return the requested input if it exists.
  ///
  USDGEOM_API
  UsdGeomInput GetInput(const TfToken &name) const;

  /// Returns all inputs present on the node-graph. These are represented by
  /// attributes in the "inputs:" namespace.
  /// If \p onlyAuthored is true (the default), then only return authored
  /// attributes; otherwise, this also returns un-authored builtins.
  ///
  USDGEOM_API
  std::vector<UsdGeomInput> GetInputs(bool onlyAuthored = true) const;

  /// @}

  // Provide custom hash and equality comparison function objects for
  // UsdGeomNodeGraph until bug 143077 is resolved.

  /// Hash functor for UsdGeomNodeGraph objects.
  struct NodeGraphHasher {
    inline size_t operator()(const UsdGeomNodeGraph &nodeGraph) const
    {
      return hash_value(nodeGraph.GetPrim());
    }
  };
  /// Equality comparator for UsdGeomNodeGraph objects.
  struct NodeGraphEqualFn {
    inline bool operator()(UsdGeomNodeGraph const &s1, UsdGeomNodeGraph const &s2) const
    {
      return s1.GetPrim() == s2.GetPrim();
    }
  };

  // ---------------------------------------------------------------------- //
  /// \anchor UsdGeomNodeGraph_InterfaceInputs
  /// \name Interface Inputs
  ///
  /// API to query the inputs that form the interface of the node-graph and
  /// their connections.
  ///
  /// @{

  /// Returns all the "Interface Inputs" of the node-graph. This is the same
  /// as GetInputs(), but is provided  as a convenience, to allow clients to
  /// distinguish between inputs on geoms vs. interface-inputs on
  /// node-graphs.
  USDGEOM_API
  std::vector<UsdGeomInput> GetInterfaceInputs() const;

  /// Map of interface inputs to corresponding vectors of inputs that
  /// consume their values.
  typedef std::unordered_map<UsdGeomInput, std::vector<UsdGeomInput>, UsdGeomInput::Hash>
      InterfaceInputConsumersMap;

  /// Map of node-graphs to their associated input-consumers map.
  typedef std::unordered_map<UsdGeomNodeGraph,
                             InterfaceInputConsumersMap,
                             NodeGraphHasher,
                             NodeGraphEqualFn>
      NodeGraphInputConsumersMap;

  /// Walks the namespace subtree below the node-graph and computes a map
  /// containing the list of all inputs on the node-graph and the associated
  /// vector of consumers of their values. The consumers can be inputs on
  /// geoms within the node-graph or on nested node-graphs).
  ///
  /// If \p computeTransitiveConsumers is true, then value consumers
  /// belonging to <b>node-graphs</b> are resolved transitively to compute the
  /// transitive mapping from inputs on the node-graph to inputs on geoms
  /// inside the material. Note that inputs on node-graphs that don't have
  /// value consumers will continue to be included in the result.
  ///
  /// This API is provided for use by DCC's that want to present node-graph
  /// interface / geom connections in the opposite direction than they are
  /// encoded in USD.
  ///
  USDGEOM_API
  InterfaceInputConsumersMap ComputeInterfaceInputConsumersMap(
      bool computeTransitiveConsumers = false) const;

  /// @}

  /// UsdGeomNodeGraph provides its own connectability behavior,
  /// to support nesting of node graphs.
  class ConnectableAPIBehavior : public UsdGeomConnectableAPIBehavior {
    USDGEOM_API
    bool CanConnectOutputToSource(const UsdGeomOutput &output,
                                  const UsdAttribute &source,
                                  std::string *reason) override;

    USDGEOM_API
    bool IsContainer() const override;
  };
};

WABI_NAMESPACE_END

#endif
