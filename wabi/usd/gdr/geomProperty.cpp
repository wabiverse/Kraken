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

#include "wabi/usd/gdr/geomProperty.h"
#include "wabi/base/tf/staticTokens.h"
#include "wabi/usd/gdr/geomMetadataHelpers.h"
#include "wabi/usd/sdf/types.h"
#include "wabi/wabi.h"

WABI_NAMESPACE_BEGIN

TF_DEFINE_PUBLIC_TOKENS(GdrPropertyTypes, GDR_PROPERTY_TYPE_TOKENS);
TF_DEFINE_PUBLIC_TOKENS(GdrPropertyMetadata, GDR_PROPERTY_METADATA_TOKENS);
TF_DEFINE_PUBLIC_TOKENS(GdrPropertyRole, GDR_PROPERTY_ROLE_TOKENS);

using GeomMetadataHelpers::GetRoleFromMetadata;
using GeomMetadataHelpers::IsPropertyAnAssetIdentifier;
using GeomMetadataHelpers::IsTruthy;
using GeomMetadataHelpers::StringVal;
using GeomMetadataHelpers::StringVecVal;
using GeomMetadataHelpers::TokenVal;
using GeomMetadataHelpers::TokenVecVal;

typedef std::unordered_map<TfToken, SdfValueTypeName, TfToken::HashFunctor> TokenToSdfTypeMap;

namespace {
/**
 * This only establishes EXACT mappings. If a mapping is not included here,
 * a one-to-one mapping isn't possible. */
const TokenToSdfTypeMap &GetTokenTypeToSdfType()
{
  static const TokenToSdfTypeMap tokenTypeToSdfType = {
      {GdrPropertyTypes->Int, SdfValueTypeNames->Int},
      {GdrPropertyTypes->String, SdfValueTypeNames->String},
      {GdrPropertyTypes->Float, SdfValueTypeNames->Float},
      {GdrPropertyTypes->Point, SdfValueTypeNames->Point3f},
      {GdrPropertyTypes->Normal, SdfValueTypeNames->Normal3f},
      {GdrPropertyTypes->Vector, SdfValueTypeNames->Vector3f},
      {GdrPropertyTypes->Matrix, SdfValueTypeNames->Matrix4d}};
  return tokenTypeToSdfType;
}

/**
 * The array equivalent of the above map. */
const TokenToSdfTypeMap &GetTokenTypeToSdfArrayType()
{
  static const TokenToSdfTypeMap tokenTypeToSdfArrayType = {
      {GdrPropertyTypes->Int, SdfValueTypeNames->IntArray},
      {GdrPropertyTypes->String, SdfValueTypeNames->StringArray},
      {GdrPropertyTypes->Float, SdfValueTypeNames->FloatArray},
      {GdrPropertyTypes->Point, SdfValueTypeNames->Point3fArray},
      {GdrPropertyTypes->Normal, SdfValueTypeNames->Normal3fArray},
      {GdrPropertyTypes->Vector, SdfValueTypeNames->Vector3fArray},
      {GdrPropertyTypes->Matrix, SdfValueTypeNames->Matrix4dArray}};
  return tokenTypeToSdfArrayType;
}

// -------------------------------------------------------------------------

/**
 * The following typedefs are only needed to support the table below that
 * indicates how to convert an GdrPropertyType given a particular "role"
 * value. */
typedef std::unordered_map<TfToken, std::pair<TfToken, size_t>, TfToken::HashFunctor>
    TokenToPairTable;

typedef std::unordered_map<TfToken, TokenToPairTable, TfToken::HashFunctor> TokenToMapTable;

/**
 * Establishes exact mappings for converting GdrPropertyTypes using "role"
 * The keys are original GdrPropertyTypes, and the value is another map,
 * keyed by the "role" metadata value. The value of that map is the
 * converted GdrPropertyType and array size. */
const TokenToMapTable &_GetConvertedGdrTypes()
{
  static const TokenToMapTable convertedGdrTypes = {
      {GdrPropertyTypes->Point, {{GdrPropertyRole->None, {GdrPropertyTypes->Float, 3}}}},
      {GdrPropertyTypes->Normal, {{GdrPropertyRole->None, {GdrPropertyTypes->Float, 3}}}},
      {GdrPropertyTypes->Vector, {{GdrPropertyRole->None, {GdrPropertyTypes->Float, 3}}}}};
  return convertedGdrTypes;
}

// -------------------------------------------------------------------------

/**
 * Determines if the metadata contains a key identifying the property as an
 * asset identifier. */
bool _IsAssetIdentifier(const NdrTokenMap &metadata)
{
  return metadata.count(GdrPropertyMetadata->IsAssetIdentifier);
}

bool _IsDefaultInput(const NdrTokenMap &metadata)
{
  return metadata.count(GdrPropertyMetadata->DefaultInput);
}

// -------------------------------------------------------------------------

/**
 * Helper to convert array types to Sdf types. Shouldn't be used directly;
 * use `_GetTypeAsSdfType()` instead. */
const SdfTypeIndicator _GetTypeAsSdfArrayType(const TfToken &type, size_t arraySize)
{
  SdfValueTypeName convertedType = SdfValueTypeNames->Token;
  bool conversionSuccessful      = false;

  /**
   * We prefer more specific types, so if the array size is 2, 3, or 4,
   * then try to convert to a fixed-dimension float array. In the future
   * if we change this to not return a fixed-size array, all the parsers
   * need to be updated to not return a fixed-size array as well. */
  if (type == GdrPropertyTypes->Float) {
    if (arraySize == 2) {
      convertedType        = SdfValueTypeNames->Float2;
      conversionSuccessful = true;
    }
    else if (arraySize == 3) {
      convertedType        = SdfValueTypeNames->Float3;
      conversionSuccessful = true;
    }
    else if (arraySize == 4) {
      convertedType        = SdfValueTypeNames->Float4;
      conversionSuccessful = true;
    }
  }

  /**
   * If no float array conversion was done, try converting to an array
   * type without a fixed dimension. */
  if (!conversionSuccessful) {
    const TokenToSdfTypeMap &tokenTypeToSdfArrayType = GetTokenTypeToSdfArrayType();
    TokenToSdfTypeMap::const_iterator it             = tokenTypeToSdfArrayType.find(type);

    if (it != tokenTypeToSdfArrayType.end()) {
      convertedType        = it->second;
      conversionSuccessful = true;
    }
  }

  return std::make_pair(convertedType, conversionSuccessful ? TfToken() : type);
}

// -------------------------------------------------------------------------

/**
 * Helper to convert the type to an Sdf type (this will call
 * `_GetTypeAsSdfArrayType()` if an array type is detected). */
const SdfTypeIndicator _GetTypeAsSdfType(const TfToken &type,
                                         size_t arraySize,
                                         const NdrTokenMap &metadata)
{
  /**
   * There is one Sdf type (Asset) that is not included in the type
   * mapping because it is determined dynamically. */
  if (_IsAssetIdentifier(metadata)) {
    if (arraySize > 0) {
      return std::make_pair(SdfValueTypeNames->AssetArray, TfToken());
    }
    return std::make_pair(SdfValueTypeNames->Asset, TfToken());
  }

  /**
   * We have several special GdrPropertyTypes that we want to map to
   * 'token', which is the type we otherwise reserve for unknown types.
   * We call out this conversion here so it is explicitly documented
   * rather than happening implicitly. */
  if (type == GdrPropertyTypes->Struct || type == GdrPropertyTypes->Vstruct) {
    return std::make_pair(SdfValueTypeNames->Token, type);
  }

  bool conversionSuccessful = false;

  /**
   * If the conversion can't be made, it defaults to the 'Token' type. */
  SdfValueTypeName convertedType = SdfValueTypeNames->Token;

  if (arraySize > 0) {
    return _GetTypeAsSdfArrayType(type, arraySize);
  }

  const TokenToSdfTypeMap &tokenTypeToSdfType = GetTokenTypeToSdfType();
  TokenToSdfTypeMap::const_iterator it        = tokenTypeToSdfType.find(type);
  if (it != tokenTypeToSdfType.end()) {
    conversionSuccessful = true;
    convertedType        = it->second;
  }

  return std::make_pair(convertedType, conversionSuccessful ? TfToken() : type);
}

// -------------------------------------------------------------------------

/**
 * This method converts a given GdrPropertyType to a new GdrPropertyType
 * and appropriate array size if the metadata indicates that such a
 * conversion is necessary.  The conversion is based on the value of the
 * "role" metadata. */
std::pair<TfToken, size_t> _ConvertGdrPropertyTypeAndArraySize(const TfToken &type,
                                                               const size_t &arraySize,
                                                               const NdrTokenMap &metadata)
{
  TfToken role = GetRoleFromMetadata(metadata);

  if (!type.IsEmpty() && !role.IsEmpty()) {
    /**
     * Look up using original type and role declaration. */
    const TokenToMapTable::const_iterator &typeSearch = _GetConvertedGdrTypes().find(type);
    if (typeSearch != _GetConvertedGdrTypes().end()) {
      const TokenToPairTable::const_iterator &roleSearch = typeSearch->second.find(role);
      if (roleSearch != typeSearch->second.end()) {
        /**
         * Return converted type and size. */
        return roleSearch->second;
      }
    }
  }

  /**
   * No conversion needed or found. */
  return std::pair<TfToken, size_t>(type, arraySize);
}

// -------------------------------------------------------------------------

template<class T> bool _GetValue(const VtValue &defaultValue, T *val)
{
  if (defaultValue.IsHolding<T>()) {
    *val = defaultValue.UncheckedGet<T>();
    return true;
  }
  return false;
}

/**
 * This methods conforms the given default value's type with the property's
 * SdfValueTypeName.  This step is important because a Gdr parser should not
 * care about what SdfValueTypeName the parsed property will eventually map
 * to, and a parser will just return the value it sees with the type that
 * most closely matches the type in the  geom  file.  Any  special  type
 * 'transformations' that make use of metadata and other knowledge should
 * happen in this conformance step when the GdrGeomProperty is instantiated. */
VtValue _ConformDefaultValue(const VtValue &defaultValue,
                             const TfToken &gdrType,
                             size_t arraySize,
                             const NdrTokenMap &metadata)
{
  /**
   * Return early if there is no value to conform. */
  if (defaultValue.IsEmpty()) {
    return defaultValue;
  }

  /**
   * Return early if no conformance issue. */
  const SdfTypeIndicator sdfTypeIndicator = _GetTypeAsSdfType(gdrType, arraySize, metadata);
  const SdfValueTypeName sdfType          = sdfTypeIndicator.first;

  if (defaultValue.GetType() == sdfType.GetType()) {
    return defaultValue;
  }

  bool isDynamicArray = IsTruthy(GdrPropertyMetadata->IsDynamicArray, metadata);
  bool isArray        = (arraySize > 0) || isDynamicArray;

  // ---------------------------------------------------------------------

  /**
   * ASSET and ASSET ARRAY */

  if (gdrType == GdrPropertyTypes->String && IsPropertyAnAssetIdentifier(metadata)) {
    if (isArray) {
      VtStringArray arrayVal;
      _GetValue(defaultValue, &arrayVal);

      VtArray<SdfAssetPath> array;
      array.reserve(arrayVal.size());

      for (const std::string &val : arrayVal) {
        array.push_back(SdfAssetPath(val));
      }
      return VtValue::Take(array);
    }
    else {
      std::string val;
      _GetValue(defaultValue, &val);
      return VtValue(SdfAssetPath(val));
    }
  }

  // ---------------------------------------------------------------------

  /**
   * FLOAT ARRAY (FIXED SIZE 2, 3, 4) */

  else if (gdrType == GdrPropertyTypes->Float && isArray) {
    VtFloatArray arrayVal;
    _GetValue(defaultValue, &arrayVal);

    /**
     * We return a fixed-size array for arrays with size 2, 3, or 4
     * because GdrGeomProperty::GetTypeAsSdfType returns a specific
     * size type (Float2, Float3, Float4). If in the future we want
     * to return a VtFloatArray instead, we need to change the logic
     * in GdrGeomProperty::GetTypeAsSdfType. */
    if (arraySize == 2) {
      return VtValue(GfVec2f(arrayVal[0], arrayVal[1]));
    }
    else if (arraySize == 3) {
      return VtValue(GfVec3f(arrayVal[0], arrayVal[1], arrayVal[2]));
    }
    else if (arraySize == 4) {
      return VtValue(GfVec4f(arrayVal[0], arrayVal[1], arrayVal[2], arrayVal[3]));
    }
  }

  /**
   * Default value's type was not conformant, but no special translation
   * step was found. */
  return defaultValue;
}
}  // namespace

GdrGeomProperty::GdrGeomProperty(const TfToken &name,
                                 const TfToken &type,
                                 const VtValue &defaultValue,
                                 bool isOutput,
                                 size_t arraySize,
                                 const NdrTokenMap &metadata,
                                 const NdrTokenMap &hints,
                                 const NdrOptionVec &options)
    : NdrProperty(
          name,
          _ConvertGdrPropertyTypeAndArraySize(type, arraySize, metadata).first, /* type */
          _ConformDefaultValue(defaultValue, type, arraySize, metadata),
          isOutput,
          _ConvertGdrPropertyTypeAndArraySize(type, arraySize, metadata).second, /* arraySize */
          false, /* isDynamicArray */
          metadata),

      _hints(hints),
      _options(options)
{
  _isDynamicArray = IsTruthy(GdrPropertyMetadata->IsDynamicArray, _metadata);

  /**
   * Note that outputs are always connectable. If "connectable" metadata is
   * found on outputs, ignore it. */
  if (isOutput) {
    _isConnectable = true;
  }
  else {
    _isConnectable = _metadata.count(GdrPropertyMetadata->Connectable) ?
                         IsTruthy(GdrPropertyMetadata->Connectable, _metadata) :
                         true;
  }

  /**
   * Indicate a "default" widget if one was not assigned. */
  _metadata.insert({GdrPropertyMetadata->Widget, "default"});

  /**
   * Tokenize metadata. */
  _label                  = TokenVal(GdrPropertyMetadata->Label, _metadata);
  _page                   = TokenVal(GdrPropertyMetadata->Page, _metadata);
  _widget                 = TokenVal(GdrPropertyMetadata->Widget, _metadata);
  _vstructMemberOf        = TokenVal(GdrPropertyMetadata->VstructMemberOf, _metadata);
  _vstructMemberName      = TokenVal(GdrPropertyMetadata->VstructMemberName, _metadata);
  _vstructConditionalExpr = TokenVal(GdrPropertyMetadata->VstructConditionalExpr, _metadata);
  _validConnectionTypes   = TokenVecVal(GdrPropertyMetadata->ValidConnectionTypes, _metadata);
}

std::string GdrGeomProperty::GetHelp() const
{
  return StringVal(GdrPropertyMetadata->Help, _metadata);
}

std::string GdrGeomProperty::GetImplementationName() const
{
  return StringVal(GdrPropertyMetadata->ImplementationName, _metadata, GetName().GetString());
}

bool GdrGeomProperty::CanConnectTo(const NdrProperty &other) const
{
  NdrPropertyConstPtr input  = !_isOutput ? this : &other;
  NdrPropertyConstPtr output = _isOutput ? this : &other;

  /**
   * Outputs cannot connect to outputs and vice versa. */
  if (_isOutput == other.IsOutput()) {
    return false;
  }

  const TfToken &inputType         = input->GetType();
  size_t inputArraySize            = input->GetArraySize();
  const NdrTokenMap &inputMetadata = input->GetMetadata();

  const TfToken &outputType         = output->GetType();
  size_t outputArraySize            = output->GetArraySize();
  const NdrTokenMap &outputMetadata = output->GetMetadata();

  /**
   * Connections are always possible if the types match exactly and the
   * array size matches. */
  if ((inputType == outputType) && (inputArraySize == outputArraySize)) {
    return true;
  }

  /**
   * Connections are also possible if the types match exactly and the input
   * is a dynamic array. */
  if ((inputType == outputType) && !output->IsArray() && input->IsDynamicArray()) {
    return true;
  }

  /**
   * Convert input/output types to Sdf types. */
  const SdfTypeIndicator &sdfInputTypeInd = _GetTypeAsSdfType(
      inputType, inputArraySize, inputMetadata);
  const SdfTypeIndicator &sdfOutputTypeInd = _GetTypeAsSdfType(
      outputType, outputArraySize, outputMetadata);
  const SdfValueTypeName &sdfInputType  = sdfInputTypeInd.first;
  const SdfValueTypeName &sdfOutputType = sdfOutputTypeInd.first;

  bool inputIsFloat3 = (inputType == GdrPropertyTypes->Point) ||
                       (inputType == GdrPropertyTypes->Normal) ||
                       (inputType == GdrPropertyTypes->Vector) ||
                       (sdfInputType == SdfValueTypeNames->Float3);

  bool outputIsFloat3 = (outputType == GdrPropertyTypes->Point) ||
                        (outputType == GdrPropertyTypes->Normal) ||
                        (outputType == GdrPropertyTypes->Vector) ||
                        (sdfOutputType == SdfValueTypeNames->Float3);

  /**
   * Connections between float-3 types are possible. */
  if (inputIsFloat3 && outputIsFloat3) {
    return true;
  }

  /**
   * Special cases. */
  if ((outputType == GdrPropertyTypes->Vstruct) && (inputType == GdrPropertyTypes->Float)) {
    /**
     * vstruct -> float is accepted because vstruct seems to be an output
     * only type. */
    return true;
  }

  return false;
}

bool GdrGeomProperty::IsVStructMember() const
{
  return _metadata.count(GdrPropertyMetadata->VstructMemberName);
}

bool GdrGeomProperty::IsVStruct() const
{
  return _type == GdrPropertyTypes->Vstruct;
}

const SdfTypeIndicator GdrGeomProperty::GetTypeAsSdfType() const
{
  return _GetTypeAsSdfType(_type, _arraySize, _metadata);
}

bool GdrGeomProperty::IsAssetIdentifier() const
{
  return _IsAssetIdentifier(_metadata);
}

bool GdrGeomProperty::IsDefaultInput() const
{
  return _IsDefaultInput(_metadata);
}

WABI_NAMESPACE_END
