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
#ifndef WABI_USD_USD_GEOM_INPUT_H
#define WABI_USD_USD_GEOM_INPUT_H

#include "wabi/usd/usd/attribute.h"
#include "wabi/usd/usdGeom/api.h"
#include "wabi/usd/usdGeom/types.h"
#include "wabi/usd/usdGeom/utils.h"
#include "wabi/wabi.h"

#include "wabi/usd/ndr/declare.h"

#include <vector>

WABI_NAMESPACE_BEGIN

class UsdGeomConnectableAPI;
struct UsdGeomConnectionSourceInfo;
class UsdGeomOutput;

/// \class UsdGeomInput
///
/// This class encapsulates a geom or node-graph input, which is a
/// connectable attribute representing a typed value.
///
class UsdGeomInput {
 public:
  /// Default constructor returns an invalid Input.  Exists for the sake of
  /// container classes
  UsdGeomInput()
  {
    // nothing
  }

  /// Get the name of the attribute associated with the Input.
  ///
  TfToken const &GetFullName() const
  {
    return _attr.GetName();
  }

  /// Returns the name of the input.
  ///
  /// We call this the base name since it strips off the "inputs:" namespace
  /// prefix from the attribute name, and returns it.
  ///
  USDGEOM_API
  TfToken GetBaseName() const;

  /// Get the "scene description" value type name of the attribute associated
  /// with the Input.
  ///
  USDGEOM_API
  SdfValueTypeName GetTypeName() const;

  /// Get the prim that the input belongs to.
  UsdPrim GetPrim() const
  {
    return _attr.GetPrim();
  }

  /// Convenience wrapper for the templated UsdAttribute::Get().
  template<typename T> bool Get(T *value, UsdTimeCode time = UsdTimeCode::Default()) const
  {
    return GetAttr().Get(value, time);
  }

  /// Convenience wrapper for VtValue version of UsdAttribute::Get().
  USDGEOM_API
  bool Get(VtValue *value, UsdTimeCode time = UsdTimeCode::Default()) const;

  /// Set a value for the Input at \p time.
  ///
  USDGEOM_API
  bool Set(const VtValue &value, UsdTimeCode time = UsdTimeCode::Default()) const;

  /// \overload
  /// Set a value of the Input at \p time.
  ///
  template<typename T> bool Set(const T &value, UsdTimeCode time = UsdTimeCode::Default()) const
  {
    return _attr.Set(value, time);
  }

  /// Hash functor.
  struct Hash {
    inline size_t operator()(const UsdGeomInput &input) const
    {
      return hash_value(input._attr);
    }
  };

  /// \name Configuring the Input's Type
  /// @{

  /// Specify an alternative, renderer-specific type to use when
  /// emitting/translating this Input, rather than translating based
  /// on its GetTypeName()
  ///
  /// For example, we set the primitiveType to "struct" for Inputs that
  /// are of renderman custom struct types.
  ///
  /// \return true on success.
  ///
  USDGEOM_API
  bool SetPrimitiveType(TfToken const &primitiveType) const;

  /// Return this Input's specialized primitiveType, or an empty
  /// token if none was authored.
  ///
  /// \sa SetPrimitiveType()
  USDGEOM_API
  TfToken GetPrimitiveType() const;

  /// Return true if a primitiveType has been specified for this Input.
  ///
  /// \sa SetPrimitiveType()
  USDGEOM_API
  bool HasPrimitiveType() const;

  /// @}

  /// \name API to author and query an Input's gdrMetadata
  ///
  /// This section provides API for authoring and querying geom registry
  /// metadata on an Input. When the owning geom prim is providing a geom
  /// definition, the authored "gdrMetadata" dictionary value provides
  /// metadata needed to populate the Input correctly in the geom registry.
  ///
  /// We expect the keys in gdrMetadata to correspond to the keys
  /// in \ref GdrPropertyMetadata. However, this is not strictly enforced by
  /// the API. The only allowed value type in the "gdrMetadata" dictionary is
  /// a std::string since it needs to be converted into a NdrTokenMap, which
  /// Gdr will parse using the utilities available in \ref GdrMetadataHelpers.
  ///
  /// @{

  /// Returns this Input's composed "gdrMetadata" dictionary as a
  /// NdrTokenMap.
  USDGEOM_API
  NdrTokenMap GetGdrMetadata() const;

  /// Returns the value corresponding to \p key in the composed
  /// <b>gdrMetadata</b> dictionary.
  USDGEOM_API
  std::string GetGdrMetadataByKey(const TfToken &key) const;

  /// Authors the given \p gdrMetadata value on this Input at the current
  /// EditTarget.
  USDGEOM_API
  void SetGdrMetadata(const NdrTokenMap &gdrMetadata) const;

  /// Sets the value corresponding to \p key to the given string \p value, in
  /// the Input's "gdrMetadata" dictionary at the current EditTarget.
  USDGEOM_API
  void SetGdrMetadataByKey(const TfToken &key, const std::string &value) const;

  /// Returns true if the Input has a non-empty composed "gdrMetadata"
  /// dictionary value.
  USDGEOM_API
  bool HasGdrMetadata() const;

  /// Returns true if there is a value corresponding to the given \p key in
  /// the composed "gdrMetadata" dictionary.
  USDGEOM_API
  bool HasGdrMetadataByKey(const TfToken &key) const;

  /// Clears any "gdrMetadata" value authored on the Input in the current
  /// EditTarget.
  USDGEOM_API
  void ClearGdrMetadata() const;

  /// Clears the entry corresponding to the given \p key in the
  /// "gdrMetadata" dictionary authored in the current EditTarget.
  USDGEOM_API
  void ClearGdrMetadataByKey(const TfToken &key) const;

  /// @}

  // ---------------------------------------------------------------
  /// \name UsdAttribute API
  // ---------------------------------------------------------------

  /// @{

  /// Speculative constructor that will produce a valid UsdGeomInput when
  /// \p attr already represents a geom Input, and produces an \em invalid
  /// UsdGeomInput otherwise (i.e. the explicit bool conversion operator will
  /// return false).
  USDGEOM_API
  explicit UsdGeomInput(const UsdAttribute &attr);

  /// Test whether a given UsdAttribute represents a valid Input, which
  /// implies that creating a UsdGeomInput from the attribute will succeed.
  ///
  /// Success implies that \c attr.IsDefined() is true.
  USDGEOM_API
  static bool IsInput(const UsdAttribute &attr);

  /// Test if this name has a namespace that indicates it could be an
  /// input.
  USDGEOM_API
  static bool IsInterfaceInputName(const std::string &name);

  /// Explicit UsdAttribute extractor.
  const UsdAttribute &GetAttr() const
  {
    return _attr;
  }

  /// Allow UsdGeomInput to auto-convert to UsdAttribute, so you can
  /// pass a UsdGeomInput to any function that accepts a UsdAttribute or
  /// const-ref thereto.
  operator const UsdAttribute &() const
  {
    return GetAttr();
  }

  /// Return true if the wrapped UsdAttribute is defined, and in addition the
  /// attribute is identified as an input.
  bool IsDefined() const
  {
    return _attr && IsInput(_attr);
  }

  /// Set documentation string for this Input.
  /// \sa UsdObject::SetDocumentation()
  USDGEOM_API
  bool SetDocumentation(const std::string &docs) const;

  /// Get documentation string for this Input.
  /// \sa UsdObject::GetDocumentation()
  USDGEOM_API
  std::string GetDocumentation() const;

  /// Set the displayGroup metadata for this Input,  i.e. hinting for the
  /// location and nesting of the attribute.
  /// \sa UsdProperty::SetDisplayGroup(), UsdProperty::SetNestedDisplayGroup()
  USDGEOM_API
  bool SetDisplayGroup(const std::string &displayGroup) const;

  /// Get the displayGroup metadata for this Input, i.e. hint for the location
  /// and nesting of the attribute.
  /// \sa UsdProperty::GetDisplayGroup(), UsdProperty::GetNestedDisplayGroup()
  USDGEOM_API
  std::string GetDisplayGroup() const;

  /// @}

  /// Return true if this Input is valid for querying and authoring
  /// values and metadata, which is identically equivalent to IsDefined().
  explicit operator bool() const
  {
    return IsDefined();
  }

  /// Equality comparison. Returns true if \a lhs and \a rhs represent the
  /// same UsdGeomInput, false otherwise.
  friend bool operator==(const UsdGeomInput &lhs, const UsdGeomInput &rhs)
  {
    return lhs.GetAttr() == rhs.GetAttr();
  }

  /// Inequality comparison. Return false if \a lhs and \a rhs represent the
  /// same UsdGeomInput, true otherwise.
  friend bool operator!=(const UsdGeomInput &lhs, const UsdGeomInput &rhs)
  {
    return !(lhs == rhs);
  }

  // -------------------------------------------------------------------------
  /// \name Connections API
  // -------------------------------------------------------------------------
  /// @{

  /// Determines whether this Input can be connected to the given
  /// source attribute, which can be an input or an output.
  ///
  /// \sa UsdGeomConnectableAPI::CanConnect
  USDGEOM_API
  bool CanConnect(const UsdAttribute &source) const;

  /// \overload
  USDGEOM_API
  bool CanConnect(const UsdGeomInput &sourceInput) const;

  /// \overload
  USDGEOM_API
  bool CanConnect(const UsdGeomOutput &sourceOutput) const;

  using ConnectionModification = UsdGeomConnectionModification;

  /// Authors a connection for this Input
  ///
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
  /// \c false if this input or \p source is invalid.
  ///
  /// \note This method does not verify the connectability of the shading
  /// attribute to the source. Clients must invoke CanConnect() themselves
  /// to ensure compatibility.
  /// \note The source shading attribute is created if it doesn't exist
  /// already.
  ///
  /// \sa UsdGeomConnectableAPI::ConnectToSource
  ///
  USDGEOM_API
  bool ConnectToSource(UsdGeomConnectionSourceInfo const &source,
                       ConnectionModification const mod = ConnectionModification::Replace) const;

  /// \deprecated
  /// \overload
  USDGEOM_API
  bool ConnectToSource(UsdGeomConnectableAPI const &source,
                       TfToken const &sourceName,
                       UsdGeomAttributeType const sourceType = UsdGeomAttributeType::Output,
                       SdfValueTypeName typeName             = SdfValueTypeName()) const;

  /// Authors a connection for this Input to the source at the given path.
  ///
  /// \sa UsdGeomConnectableAPI::ConnectToSource
  ///
  USDGEOM_API
  bool ConnectToSource(SdfPath const &sourcePath) const;

  /// Connects this Input to the given input, \p sourceInput.
  ///
  /// \sa UsdGeomConnectableAPI::ConnectToSource
  ///
  USDGEOM_API
  bool ConnectToSource(UsdGeomInput const &sourceInput) const;

  /// Connects this Input to the given output, \p sourceOutput.
  ///
  /// \sa UsdGeomConnectableAPI::ConnectToSource
  ///
  USDGEOM_API
  bool ConnectToSource(UsdGeomOutput const &sourceOutput) const;

  /// Connects this Input to the given sources, \p sourceInfos
  ///
  /// \sa UsdGeomConnectableAPI::SetConnectedSources
  ///
  USDGEOM_API
  bool SetConnectedSources(std::vector<UsdGeomConnectionSourceInfo> const &sourceInfos) const;

  using SourceInfoVector = TfSmallVector<UsdGeomConnectionSourceInfo, 1>;

  /// Finds the valid sources of connections for the Input.
  ///
  /// \p invalidSourcePaths is an optional output parameter to collect the
  /// invalid source paths that have not been reported in the returned vector.
  ///
  /// Returns a vector of \p UsdGeomConnectionSourceInfo structs with
  /// information about each upsteam attribute. If the vector is empty, there
  /// have been no valid connections.
  ///
  /// \note A valid connection requires the existence of the source attribute
  /// and also requires that the source prim is UsdGeomConnectableAPI
  /// compatible.
  /// \note The python wrapping returns a tuple with the valid connections
  /// first, followed by the invalid source paths.
  ///
  /// \sa UsdGeomConnectableAPI::GetConnectedSources
  ///
  USDGEOM_API
  SourceInfoVector GetConnectedSources(SdfPathVector *invalidSourcePaths = nullptr) const;

  /// \deprecated
  USDGEOM_API
  bool GetConnectedSource(UsdGeomConnectableAPI *source,
                          TfToken *sourceName,
                          UsdGeomAttributeType *sourceType) const;

  /// \deprecated
  /// Returns the "raw" (authored) connected source paths for this Input.
  ///
  /// \sa UsdGeomConnectableAPI::GetRawConnectedSourcePaths
  ///
  USDGEOM_API
  bool GetRawConnectedSourcePaths(SdfPathVector *sourcePaths) const;

  /// Returns true if and only if this Input is currently connected to a
  /// valid (defined) source.
  ///
  /// \sa UsdGeomConnectableAPI::HasConnectedSource
  ///
  USDGEOM_API
  bool HasConnectedSource() const;

  /// Returns true if the connection to this Input's source, as returned by
  /// GetConnectedSource(), is authored across a specializes arc, which is
  /// used to denote a base material.
  ///
  /// \sa UsdGeomConnectableAPI::IsSourceConnectionFromBaseMaterial
  ///
  USDGEOM_API
  bool IsSourceConnectionFromBaseMaterial() const;

  /// Disconnect source for this Input. If \p sourceAttr is valid, only a
  /// connection to the specified attribute is disconnected, otherwise all
  /// connections are removed.
  ///
  /// \sa UsdGeomConnectableAPI::DisconnectSource
  ///
  USDGEOM_API
  bool DisconnectSource(UsdAttribute const &sourceAttr = UsdAttribute()) const;

  /// Clears sources for this Input in the current UsdEditTarget.
  ///
  /// Most of the time, what you probably want is DisconnectSource()
  /// rather than this function.
  ///
  /// \sa UsdGeomConnectableAPI::ClearSources
  ///
  USDGEOM_API
  bool ClearSources() const;

  /// \deprecated
  USDGEOM_API
  bool ClearSource() const;

  /// @}

  // -------------------------------------------------------------------------
  /// \name Connectability API
  // -------------------------------------------------------------------------
  /// @{

  /// \brief Set the connectability of the Input.
  ///
  /// In certain shading data models, there is a need to distinguish which
  /// inputs <b>can</b> vary over a surface from those that must be
  /// <b>uniform</b>. This is accomplished in UsdGeom by limiting the
  /// connectability of the input. This is done by setting the
  /// "connectability" metadata on the associated attribute.
  ///
  /// Connectability of an Input can be set to UsdGeomTokens->full or
  /// UsdGeomTokens->interfaceOnly.
  ///
  /// \li <b>full</b> implies that  the Input can be connected to any other
  /// Input or Output.
  /// \li <b>interfaceOnly</b> implies that the Input can only be connected to
  /// a NodeGraph Input (which represents an interface override, not a
  /// render-time dataflow connection), or another Input whose connectability
  /// is also "interfaceOnly".
  ///
  /// The default connectability of an input is UsdGeomTokens->full.
  ///
  /// \sa SetConnectability()
  USDGEOM_API
  bool SetConnectability(const TfToken &connectability) const;

  /// \brief Returns the connectability of the Input.
  ///
  /// \sa SetConnectability()
  USDGEOM_API
  TfToken GetConnectability() const;

  /// \brief Clears any authored connectability on the Input.
  ///
  USDGEOM_API
  bool ClearConnectability() const;

  /// @}

  // -------------------------------------------------------------------------
  /// \name Connected Value API
  // -------------------------------------------------------------------------
  /// @{

  /// \brief Find what is connected to this Input recursively
  ///
  /// \sa UsdGeomUtils::GetValueProducingAttributes
  USDGEOM_API
  UsdGeomAttributeVector GetValueProducingAttributes(bool geomOutputsOnly = false) const;

  /// \deprecated in favor of calling GetValueProducingAttributes
  USDGEOM_API
  UsdAttribute GetValueProducingAttribute(UsdGeomAttributeType *attrType) const;

  /// @}

 private:
  friend class UsdGeomConnectableAPI;

  // Constructor that creates a UsdGeomInput with the given name on the
  // given prim.
  // \p name here is the unnamespaced name of the input.
  UsdGeomInput(UsdPrim prim, TfToken const &name, SdfValueTypeName const &typeName);

  UsdAttribute _attr;
};

WABI_NAMESPACE_END

#endif  // WABI_USD_USD_GEOM_INPUT_H
