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
#ifndef USDGEOM_GENERATED_CONNECTABLEAPI_H
#define USDGEOM_GENERATED_CONNECTABLEAPI_H

/// \file usdGeom/connectableAPI.h

#include "wabi/usd/usd/apiSchemaBase.h"
#include "wabi/usd/usd/prim.h"
#include "wabi/usd/usd/stage.h"
#include "wabi/usd/usdGeom/api.h"
#include "wabi/wabi.h"

#include "wabi/usd/usd/typed.h"
#include "wabi/usd/usdGeom/input.h"
#include "wabi/usd/usdGeom/output.h"
#include "wabi/usd/usdGeom/tokens.h"
#include "wabi/usd/usdGeom/types.h"

#include "wabi/base/vt/value.h"

#include "wabi/base/gf/matrix4d.h"
#include "wabi/base/gf/vec3d.h"
#include "wabi/base/gf/vec3f.h"

#include "wabi/base/tf/token.h"
#include "wabi/base/tf/type.h"

WABI_NAMESPACE_BEGIN

class SdfAssetPath;

// -------------------------------------------------------------------------- //
// CONNECTABLEAPI                                                             //
// -------------------------------------------------------------------------- //

/// \class UsdGeomConnectableAPI
///
/// UsdGeomConnectableAPI is an API schema that provides a common
/// interface for creating outputs and making connections between shading
/// parameters and outputs. The interface is common to all UsdGeom schemas
/// that support Inputs and Outputs, which currently includes UsdGeomImageable,
/// and UsdGeomGprim .
///
/// One can construct a UsdGeomConnectableAPI directly from a UsdPrim, or
/// from objects of any of the schema classes listed above.  If it seems
/// onerous to need to construct a secondary schema object to interact with
/// Inputs and Outputs, keep in mind that any function whose purpose is either
/// to walk material/geom networks via their connections, or to create such
/// networks, can typically be written entirely in terms of
/// UsdGeomConnectableAPI objects, without needing to care what the underlying
/// prim type is.
///
/// Additionally, the most common UsdGeomConnectableAPI behaviors
/// (creating Inputs and Outputs, and making connections) are wrapped as
/// convenience methods on the prim schema classes (creation) and
/// UsdGeomInput and UsdGeomOutput.
///
///
class UsdGeomConnectableAPI : public UsdAPISchemaBase {
 public:
  /// Compile time constant representing what kind of schema this class is.
  ///
  /// \sa UsdSchemaKind
  static const UsdSchemaKind schemaKind = UsdSchemaKind::NonAppliedAPI;

  /// \deprecated
  /// Same as schemaKind, provided to maintain temporary backward
  /// compatibility with older generated schemas.
  static const UsdSchemaKind schemaType = UsdSchemaKind::NonAppliedAPI;

  /// Construct a UsdGeomConnectableAPI on UsdPrim \p prim .
  /// Equivalent to UsdGeomConnectableAPI::Get(prim.GetStage(), prim.GetPath())
  /// for a \em valid \p prim, but will not immediately throw an error for
  /// an invalid \p prim
  explicit UsdGeomConnectableAPI(const UsdPrim &prim = UsdPrim()) : UsdAPISchemaBase(prim)
  {}

  /// Construct a UsdGeomConnectableAPI on the prim held by \p schemaObj .
  /// Should be preferred over UsdGeomConnectableAPI(schemaObj.GetPrim()),
  /// as it preserves SchemaBase state.
  explicit UsdGeomConnectableAPI(const UsdSchemaBase &schemaObj) : UsdAPISchemaBase(schemaObj)
  {}

  /// Destructor.
  USDGEOM_API
  virtual ~UsdGeomConnectableAPI();

  /// Return a vector of names of all pre-declared attributes for this schema
  /// class and all its ancestor classes.  Does not include attributes that
  /// may be authored by custom/extended methods of the schemas involved.
  USDGEOM_API
  static const TfTokenVector &GetSchemaAttributeNames(bool includeInherited = true);

  /// Return a UsdGeomConnectableAPI holding the prim adhering to this
  /// schema at \p path on \p stage.  If no prim exists at \p path on
  /// \p stage, or if the prim at that path does not adhere to this schema,
  /// return an invalid schema object.  This is shorthand for the following:
  ///
  /// \code
  /// UsdGeomConnectableAPI(stage->GetPrimAtPath(path));
  /// \endcode
  ///
  USDGEOM_API
  static UsdGeomConnectableAPI Get(const UsdStagePtr &stage, const SdfPath &path);

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

 protected:
  /// Returns true if the given prim is compatible with this API schema,
  /// i.e. if it is a valid geom or a node-graph.
  USDGEOM_API
  bool _IsCompatible() const override;

 public:
  /// Returns true if the prim is a container.
  ///
  /// The underlying prim type may provide runtime behavior
  /// that defines whether it is a container.
  USDGEOM_API
  bool IsContainer() const;

  /// \name Connections
  ///
  /// Inputs and outputs on geoms and node-graphs are connectable.
  /// This section provides API for authoring and managing these connections
  /// in a shading network.
  ///
  /// @{

  /// Determines whether the given input can be connected to the given
  /// source attribute, which can be an input or an output.
  ///
  /// The result depends on the "connectability" of the input and the source
  /// attributes.  Depending on the prim type, this may require the plugin
  /// that defines connectability behavior for that prim type be loaded.
  ///
  /// \sa UsdGeomInput::SetConnectability
  /// \sa UsdGeomInput::GetConnectability
  USDGEOM_API
  static bool CanConnect(const UsdGeomInput &input, const UsdAttribute &source);

  /// \overload
  USDGEOM_API
  static bool CanConnect(const UsdGeomInput &input, const UsdGeomInput &sourceInput)
  {
    return CanConnect(input, sourceInput.GetAttr());
  }

  /// \overload
  USDGEOM_API
  static bool CanConnect(const UsdGeomInput &input, const UsdGeomOutput &sourceOutput)
  {
    return CanConnect(input, sourceOutput.GetAttr());
  }

  /// Determines whether the given output can be connected to the given
  /// source attribute, which can be an input or an output.
  ///
  /// An output is considered to be connectable only if it belongs to a
  /// node-graph. Geom outputs are not connectable.
  ///
  /// \p source is an optional argument. If a valid UsdAttribute is supplied
  /// for it, this method will return true only if the source attribute is
  /// owned by a descendant of the node-graph owning the output.
  ///
  USDGEOM_API
  static bool CanConnect(const UsdGeomOutput &output, const UsdAttribute &source = UsdAttribute());

  /// \overload
  USDGEOM_API
  static bool CanConnect(const UsdGeomOutput &output, const UsdGeomInput &sourceInput)
  {
    return CanConnect(output, sourceInput.GetAttr());
  }

  /// \overload
  USDGEOM_API
  static bool CanConnect(const UsdGeomOutput &output, const UsdGeomOutput &sourceOutput)
  {
    return CanConnect(output, sourceOutput.GetAttr());
  }

  using ConnectionModification = UsdGeomConnectionModification;

  /// Authors a connection for a given shading attribute \p shadingAttr.
  ///
  /// \p shadingAttr can represent a parameter, an input or an output.
  /// \p source is a struct that describes the upstream source attribute
  /// with all the information necessary to make a connection. See the
  /// documentation for UsdGeomConnectionSourceInfo.
  /// \p mod describes the operation that should be applied to the list of
  /// connections. By default the new connection will replace any existing
  /// connections, but it can add to the list of connections to represent
  /// multiple input connections.
  ///
  /// \return
  /// \c true if a connection was created successfully.
  /// \c false if \p shadingAttr or \p source is invalid.
  ///
  /// \note This method does not verify the connectability of the shading
  /// attribute to the source. Clients must invoke CanConnect() themselves
  /// to ensure compatibility.
  /// \note The source shading attribute is created if it doesn't exist
  /// already.
  ///
  USDGEOM_API
  static bool ConnectToSource(UsdAttribute const &shadingAttr,
                              UsdGeomConnectionSourceInfo const &source,
                              ConnectionModification const mod = ConnectionModification::Replace);

  /// \overload
  USDGEOM_API
  static bool ConnectToSource(UsdGeomInput const &input,
                              UsdGeomConnectionSourceInfo const &source,
                              ConnectionModification const mod = ConnectionModification::Replace)
  {
    return ConnectToSource(input.GetAttr(), source, mod);
  }

  /// \overload
  USDGEOM_API
  static bool ConnectToSource(UsdGeomOutput const &output,
                              UsdGeomConnectionSourceInfo const &source,
                              ConnectionModification const mod = ConnectionModification::Replace)
  {
    return ConnectToSource(output.GetAttr(), source, mod);
  }

  /// \deprecated Please use the versions that take a
  /// UsdGeomConnectionSourceInfo to describe the upstream source
  /// \overload
  USDGEOM_API
  static bool ConnectToSource(UsdAttribute const &shadingAttr,
                              UsdGeomConnectableAPI const &source,
                              TfToken const &sourceName,
                              UsdGeomAttributeType const sourceType = UsdGeomAttributeType::Output,
                              SdfValueTypeName typeName             = SdfValueTypeName());

  /// \deprecated
  /// \overload
  USDGEOM_API
  static bool ConnectToSource(UsdGeomInput const &input,
                              UsdGeomConnectableAPI const &source,
                              TfToken const &sourceName,
                              UsdGeomAttributeType const sourceType = UsdGeomAttributeType::Output,
                              SdfValueTypeName typeName             = SdfValueTypeName())
  {
    return ConnectToSource(input.GetAttr(), source, sourceName, sourceType, typeName);
  }

  /// \deprecated
  /// \overload
  USDGEOM_API
  static bool ConnectToSource(UsdGeomOutput const &output,
                              UsdGeomConnectableAPI const &source,
                              TfToken const &sourceName,
                              UsdGeomAttributeType const sourceType = UsdGeomAttributeType::Output,
                              SdfValueTypeName typeName             = SdfValueTypeName())
  {
    return ConnectToSource(output.GetAttr(), source, sourceName, sourceType, typeName);
  }

  /// \overload
  ///
  /// Connect the given shading attribute to the source at path, \p sourcePath.
  ///
  /// \p sourcePath should be the fully namespaced property path.
  ///
  /// This overload is provided for convenience, for use in contexts where
  /// the prim types are unknown or unavailable.
  ///
  USDGEOM_API
  static bool ConnectToSource(UsdAttribute const &shadingAttr, SdfPath const &sourcePath);

  /// \overload
  USDGEOM_API
  static bool ConnectToSource(UsdGeomInput const &input, SdfPath const &sourcePath)
  {
    return ConnectToSource(input.GetAttr(), sourcePath);
  }

  /// \overload
  USDGEOM_API
  static bool ConnectToSource(UsdGeomOutput const &output, SdfPath const &sourcePath)
  {
    return ConnectToSource(output.GetAttr(), sourcePath);
  }

  /// \overload
  ///
  /// Connect the given shading attribute to the given source input.
  ///
  USDGEOM_API
  static bool ConnectToSource(UsdAttribute const &shadingAttr, UsdGeomInput const &sourceInput);

  /// \overload
  USDGEOM_API
  static bool ConnectToSource(UsdGeomInput const &input, UsdGeomInput const &sourceInput)
  {
    return ConnectToSource(input.GetAttr(), sourceInput);
  }

  /// \overload
  USDGEOM_API
  static bool ConnectToSource(UsdGeomOutput const &output, UsdGeomInput const &sourceInput)
  {
    return ConnectToSource(output.GetAttr(), sourceInput);
  }

  /// \overload
  ///
  /// Connect the given shading attribute to the given source output.
  ///
  USDGEOM_API
  static bool ConnectToSource(UsdAttribute const &shadingAttr, UsdGeomOutput const &sourceOutput);

  /// \overload
  USDGEOM_API
  static bool ConnectToSource(UsdGeomInput const &input, UsdGeomOutput const &sourceOutput)
  {
    return ConnectToSource(input.GetAttr(), sourceOutput);
  }

  /// \overload
  USDGEOM_API
  static bool ConnectToSource(UsdGeomOutput const &output, UsdGeomOutput const &sourceOutput)
  {
    return ConnectToSource(output.GetAttr(), sourceOutput);
  }

  /// Authors a list of connections for a given shading attribute
  /// \p shadingAttr.
  ///
  /// \p shadingAttr can represent a parameter, an input or an output.
  /// \p sourceInfos is a vector of structs that describes the upstream source
  /// attributes with all the information necessary to make all the
  /// connections. See the documentation for UsdGeomConnectionSourceInfo.
  ///
  /// \return
  /// \c true if all connection were created successfully.
  /// \c false if the \p shadingAttr or one of the sources are invalid.
  ///
  /// \note A valid connection is one that has a valid
  /// \p UsdGeomConnectionSourceInfo, which requires the existence of the
  /// upstream source prim. It does not require the existence of the source
  /// attribute as it will be create if necessary.
  USDGEOM_API
  static bool SetConnectedSources(UsdAttribute const &shadingAttr,
                                  std::vector<UsdGeomConnectionSourceInfo> const &sourceInfos);

  /// \deprecated Shading attributes can have multiple connections and so
  /// using GetConnectedSources is needed in general
  ///
  /// Finds the source of a connection for the given shading attribute.
  ///
  /// \p shadingAttr is the shading attribute whose connection we want to
  /// interrogate.
  /// \p source is an output parameter which will be set to the source
  /// connectable prim.
  /// \p sourceName will be set to the name of the source shading attribute,
  /// which may be an input or an output, as specified by \p sourceType
  /// \p sourceType will have the type of the source shading attribute, i.e.
  /// whether it is an \c Input or \c Output
  ///
  /// \return
  /// \c true if the shading attribute is connected to a valid, defined source
  /// attribute.
  /// \c false if the shading attribute is not connected to a single, defined
  /// source attribute.
  ///
  /// \note Previously this method would silently return false for multiple
  /// connections. We are changing the behavior of this method to return the
  /// result for the first connection and issue a TfWarn about it. We want to
  /// encourage clients to use GetConnectedSources going forward.
  /// \note The python wrapping for this method returns a
  /// (source, sourceName, sourceType) tuple if the parameter is connected,
  /// else \c None
  USDGEOM_API
  static bool GetConnectedSource(UsdAttribute const &shadingAttr,
                                 UsdGeomConnectableAPI *source,
                                 TfToken *sourceName,
                                 UsdGeomAttributeType *sourceType);

  /// \deprecated
  /// \overload
  USDGEOM_API
  static bool GetConnectedSource(UsdGeomInput const &input,
                                 UsdGeomConnectableAPI *source,
                                 TfToken *sourceName,
                                 UsdGeomAttributeType *sourceType)
  {
    return GetConnectedSource(input.GetAttr(), source, sourceName, sourceType);
  }

  /// \deprecated
  /// \overload
  USDGEOM_API
  static bool GetConnectedSource(UsdGeomOutput const &output,
                                 UsdGeomConnectableAPI *source,
                                 TfToken *sourceName,
                                 UsdGeomAttributeType *sourceType)
  {
    return GetConnectedSource(output.GetAttr(), source, sourceName, sourceType);
  }

  /// Finds the valid sources of connections for the given shading attribute.
  ///
  /// \p shadingAttr is the shading attribute whose connections we want to
  /// interrogate.
  /// \p invalidSourcePaths is an optional output parameter to collect the
  /// invalid source paths that have not been reported in the returned vector.
  ///
  /// Returns a vector of \p UsdGeomConnectionSourceInfo structs with
  /// information about each upsteam attribute. If the vector is empty, there
  /// have been no connections.
  ///
  /// \note A valid connection requires the existence of the source attribute
  /// and also requires that the source prim is UsdGeomConnectableAPI
  /// compatible.
  /// \note The python wrapping returns a tuple with the valid connections
  /// first, followed by the invalid source paths.
  USDGEOM_API
  static UsdGeomSourceInfoVector GetConnectedSources(UsdAttribute const &shadingAttr,
                                                     SdfPathVector *invalidSourcePaths = nullptr);

  /// \overload
  USDGEOM_API
  static UsdGeomSourceInfoVector GetConnectedSources(UsdGeomInput const &input,
                                                     SdfPathVector *invalidSourcePaths = nullptr);

  /// \overload
  USDGEOM_API
  static UsdGeomSourceInfoVector GetConnectedSources(UsdGeomOutput const &output,
                                                     SdfPathVector *invalidSourcePaths = nullptr);

  /// \deprecated Please us GetConnectedSources to retrieve multiple
  /// connections
  ///
  /// Returns the "raw" (authored) connected source paths for the given
  /// shading attribute.
  USDGEOM_API
  static bool GetRawConnectedSourcePaths(UsdAttribute const &shadingAttr,
                                         SdfPathVector *sourcePaths);

  /// \deprecated
  /// \overload
  USDGEOM_API
  static bool GetRawConnectedSourcePaths(UsdGeomInput const &input, SdfPathVector *sourcePaths)
  {
    return GetRawConnectedSourcePaths(input.GetAttr(), sourcePaths);
  }

  /// \deprecated
  /// \overload
  USDGEOM_API
  static bool GetRawConnectedSourcePaths(UsdGeomOutput const &output, SdfPathVector *sourcePaths)
  {
    return GetRawConnectedSourcePaths(output.GetAttr(), sourcePaths);
  }

  /// Returns true if and only if the shading attribute is currently connected
  /// to at least one valid (defined) source.
  ///
  /// If you will be calling GetConnectedSources() afterwards anyways,
  /// it will be \em much faster to instead check if the returned vector is
  /// empty:
  /// \code
  /// UsdGeomSourceInfoVector connections =
  ///     UsdGeomConnectableAPI::GetConnectedSources(attribute);
  /// if (!connections.empty()){
  ///      // process connected attribute
  /// } else {
  ///      // process unconnected attribute
  /// }
  /// \endcode
  USDGEOM_API
  static bool HasConnectedSource(const UsdAttribute &shadingAttr);

  /// \overload
  USDGEOM_API
  static bool HasConnectedSource(const UsdGeomInput &input)
  {
    return HasConnectedSource(input.GetAttr());
  }

  /// \overload
  USDGEOM_API
  static bool HasConnectedSource(const UsdGeomOutput &output)
  {
    return HasConnectedSource(output.GetAttr());
  }

  /// Returns true if the connection to the given shading attribute's source,
  /// as returned by UsdGeomConnectableAPI::GetConnectedSource(), is authored
  /// across a specializes arc, which is used to denote a base material.
  ///
  USDGEOM_API
  static bool IsSourceConnectionFromBaseMaterial(const UsdAttribute &shadingAttr);

  /// \overload
  USDGEOM_API
  static bool IsSourceConnectionFromBaseMaterial(const UsdGeomInput &input)
  {
    return IsSourceConnectionFromBaseMaterial(input.GetAttr());
  }

  /// \overload
  USDGEOM_API
  static bool IsSourceConnectionFromBaseMaterial(const UsdGeomOutput &output)
  {
    return IsSourceConnectionFromBaseMaterial(output.GetAttr());
  }

  /// Disconnect source for this shading attribute.
  ///
  /// If \p sourceAttr is valid it will disconnect the connection to this
  /// upstream attribute. Otherwise it will disconnect all connections by
  /// authoring an empty list of connections for the attribute \p shadingAttr.
  ///
  /// This may author more scene description than you might expect - we define
  /// the behavior of disconnect to be that, even if a shading attribute
  /// becomes connected in a weaker layer than the current UsdEditTarget, the
  /// attribute will \em still be disconnected in the composition, therefore
  /// we must "block" it in the current UsdEditTarget.
  ///
  /// \sa ConnectToSource().
  USDGEOM_API
  static bool DisconnectSource(UsdAttribute const &shadingAttr,
                               UsdAttribute const &sourceAttr = UsdAttribute());

  /// \overload
  USDGEOM_API
  static bool DisconnectSource(UsdGeomInput const &input,
                               UsdAttribute const &sourceAttr = UsdAttribute())
  {
    return DisconnectSource(input.GetAttr(), sourceAttr);
  }

  /// \overload
  USDGEOM_API
  static bool DisconnectSource(UsdGeomOutput const &output,
                               UsdAttribute const &sourceAttr = UsdAttribute())
  {
    return DisconnectSource(output.GetAttr(), sourceAttr);
  }

  /// Clears sources for this shading attribute in the current UsdEditTarget.
  ///
  /// Most of the time, what you probably want is DisconnectSource()
  /// rather than this function.
  ///
  /// \sa DisconnectSource()
  USDGEOM_API
  static bool ClearSources(UsdAttribute const &shadingAttr);

  /// \overload
  USDGEOM_API
  static bool ClearSources(UsdGeomInput const &input)
  {
    return ClearSources(input.GetAttr());
  }

  /// \overload
  USDGEOM_API
  static bool ClearSources(UsdGeomOutput const &output)
  {
    return ClearSources(output.GetAttr());
  }

  /// \deprecated This is the older version that only referenced a single
  /// source. Please use ClearSources instead.
  USDGEOM_API
  static bool ClearSource(UsdAttribute const &shadingAttr)
  {
    return ClearSources(shadingAttr);
  }

  /// \deprecated
  /// \overload
  USDGEOM_API
  static bool ClearSource(UsdGeomInput const &input)
  {
    return ClearSources(input.GetAttr());
  }

  /// \deprecated
  /// \overload
  USDGEOM_API
  static bool ClearSource(UsdGeomOutput const &output)
  {
    return ClearSources(output.GetAttr());
  }

  /// Return true if the \p schemaType has a connectableAPIBehavior
  /// registered, false otherwise.
  USDGEOM_API
  static bool HasConnectableAPI(const TfType &schemaType);

  /// Return true if the schema type \p T has a connectableAPIBehavior
  /// registered, false otherwise.
  template<typename T> static bool HasConnectableAPI()
  {
    static_assert(std::is_base_of<UsdTyped, T>::value, "Provided type must derive UsdTyped.");
    return HasConnectableAPI(TfType::Find<T>());
  };

  /// @}

  /// \name Outputs
  /// @{

  /// Create an output, which represents and externally computed, typed value.
  /// Outputs on node-graphs can be connected.
  ///
  /// The attribute representing an output is created in the "outputs:"
  /// namespace.
  ///
  USDGEOM_API
  UsdGeomOutput CreateOutput(const TfToken &name, const SdfValueTypeName &typeName) const;

  /// Return the requested output if it exists.
  ///
  /// \p name is the unnamespaced base name.
  ///
  USDGEOM_API
  UsdGeomOutput GetOutput(const TfToken &name) const;

  /// Returns all outputs on the connectable prim (i.e. geom or node-graph).
  /// Outputs are represented by attributes in the "outputs:" namespace.
  /// If \p onlyAuthored is true (the default), then only return authored
  /// attributes; otherwise, this also returns un-authored builtins.
  ///
  USDGEOM_API
  std::vector<UsdGeomOutput> GetOutputs(bool onlyAuthored = true) const;

  /// @}

  /// \name Inputs
  /// @{

  /// Create an input which can both have a value and be connected.
  /// The attribute representing the input is created in the "inputs:"
  /// namespace.
  ///
  USDGEOM_API
  UsdGeomInput CreateInput(const TfToken &name, const SdfValueTypeName &typeName) const;

  /// Return the requested input if it exists.
  ///
  /// \p name is the unnamespaced base name.
  ///
  USDGEOM_API
  UsdGeomInput GetInput(const TfToken &name) const;

  /// Returns all inputs on the connectable prim (i.e. geom or node-graph).
  /// Inputs are represented by attributes in the "inputs:" namespace.
  /// If \p onlyAuthored is true (the default), then only return authored
  /// attributes; otherwise, this also returns un-authored builtins.
  ///
  USDGEOM_API
  std::vector<UsdGeomInput> GetInputs(bool onlyAuthored = true) const;

  /// @}
};

/// A compact struct to represent a bundle of information about an upstream
/// source attribute
struct UsdGeomConnectionSourceInfo {
  /// \p source is the connectable prim that produces or contains a value
  /// for the given shading attribute.
  UsdGeomConnectableAPI source;
  /// \p sourceName is the name of the shading attribute that is the target
  /// of the connection. This excludes any namespace prefix that determines
  /// the type of the source (eg, output).
  TfToken sourceName;
  /// \p sourceType is used to indicate the type of the shading attribute
  /// that is the target of the connection. The source type is used to
  /// determine the namespace prefix that must be attached to \p sourceName
  /// to determine the source full attribute name.
  UsdGeomAttributeType sourceType = UsdGeomAttributeType::Invalid;
  /// \p typeName, if specified, is the typename of the attribute to create
  /// on the source if it doesn't exist when creating a connection
  SdfValueTypeName typeName;

  UsdGeomConnectionSourceInfo() = default;
  explicit UsdGeomConnectionSourceInfo(UsdGeomConnectableAPI const &source_,
                                       TfToken const &sourceName_,
                                       UsdGeomAttributeType sourceType_,
                                       SdfValueTypeName typeName_ = SdfValueTypeName())
      : source(source_),
        sourceName(sourceName_),
        sourceType(sourceType_),
        typeName(typeName_)
  {}
  explicit UsdGeomConnectionSourceInfo(UsdGeomInput const &input)
      : source(input.GetPrim()),
        sourceName(input.GetBaseName()),
        sourceType(UsdGeomAttributeType::Input),
        typeName(input.GetAttr().GetTypeName())
  {}
  explicit UsdGeomConnectionSourceInfo(UsdGeomOutput const &output)
      : source(output.GetPrim()),
        sourceName(output.GetBaseName()),
        sourceType(UsdGeomAttributeType::Output),
        typeName(output.GetAttr().GetTypeName())
  {}
  /// Construct the information for this struct from a property path. The
  /// source attribute does not have to exist, but the \p sourcePath needs to
  /// have a valid prefix to identify the sourceType. The source prim needs
  /// to exist and be UsdGeomConnectableAPI compatible
  USDGEOM_API
  explicit UsdGeomConnectionSourceInfo(UsdStagePtr const &stage, SdfPath const &sourcePath);

  /// Return true if this source info is valid for setting up a connection
  bool IsValid() const
  {
    // typeName can be invalid, so we don't check it. Order of checks is in
    // order of cost (cheap to expensive).
    // Note, for the source we only check that the prim is valid. We do not
    // verify that the prim is compatibel with UsdGeomConnectableAPI. This
    // makes it possible to target pure overs
    return (sourceType != UsdGeomAttributeType::Invalid) && !sourceName.IsEmpty() &&
           (bool)source.GetPrim();
  }
  explicit operator bool() const
  {
    return IsValid();
  }
  bool operator==(UsdGeomConnectionSourceInfo const &other) const
  {
    // We don't compare the typeName, since it is optional
    return sourceName == other.sourceName && sourceType == other.sourceType &&
           source.GetPrim() == other.source.GetPrim();
  }
  bool operator!=(const UsdGeomConnectionSourceInfo &other) const
  {
    return !(*this == other);
  }
};

WABI_NAMESPACE_END

#endif
