//
// Copyright 2016 Pixar
//
// Licensed under the Apache License, Version 2.0 (the "Apache License")
// with the following modification; you may not use this file except in
// compliance with the Apache License and the following modification to it:
// Section 6. Trademarks. is deleted and replaced with:
//
// 6. Trademarks. This License does not grant permission to use the trade
//    names, trademarks, service marks, or product names of the Licensor
//    and its affiliates, except as required to comply with Section 4(c) of
//    the License and to reproduce the content of the NOTICE file.
//
// You may obtain a copy of the Apache License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the Apache License with the above modification is
// distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied. See the Apache License for the specific
// language governing permissions and limitations under the Apache License.
//
#include "wabi/usd/usdGeom/boundable.h"
#include "wabi/usd/usd/schemaRegistry.h"
#include "wabi/usd/usd/typed.h"

#include "wabi/usd/sdf/assetPath.h"
#include "wabi/usd/sdf/types.h"

WABI_NAMESPACE_BEGIN

// Register the schema with the TfType system.
TF_REGISTRY_FUNCTION(TfType)
{
  TfType::Define<UsdGeomBoundable, TfType::Bases<UsdGeomXformable>>();
}

/* virtual */
UsdGeomBoundable::~UsdGeomBoundable()
{}

/* static */
UsdGeomBoundable UsdGeomBoundable::Get(const UsdStagePtr &stage, const SdfPath &path)
{
  if (!stage)
  {
    TF_CODING_ERROR("Invalid stage");
    return UsdGeomBoundable();
  }
  return UsdGeomBoundable(stage->GetPrimAtPath(path));
}

/* virtual */
UsdSchemaKind UsdGeomBoundable::GetSchemaKind() const
{
  return UsdGeomBoundable::schemaKind;
}

/* static */
const TfType &UsdGeomBoundable::GetStaticTfType()
{
  static TfType tfType = TfType::Find<UsdGeomBoundable>();
  return tfType;
}

/* static */
bool UsdGeomBoundable::IsTypedSchema()
{
  static bool isTyped = GetStaticTfType().IsA<UsdTyped>();
  return isTyped;
}

/* virtual */
const TfType &UsdGeomBoundable::GetType() const
{
  return GetStaticTfType();
}

UsdAttribute UsdGeomBoundable::GetExtentAttr() const
{
  return GetPrim().GetAttribute(UsdGeomTokens->extent);
}

UsdAttribute UsdGeomBoundable::CreateExtentAttr(VtValue const &defaultValue, bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(UsdGeomTokens->extent,
                                    SdfValueTypeNames->Float3Array,
                                    /* custom = */ false,
                                    SdfVariabilityVarying,
                                    defaultValue,
                                    writeSparsely);
}

namespace
{
static inline TfTokenVector _ConcatenateAttributeNames(const TfTokenVector &left, const TfTokenVector &right)
{
  TfTokenVector result;
  result.reserve(left.size() + right.size());
  result.insert(result.end(), left.begin(), left.end());
  result.insert(result.end(), right.begin(), right.end());
  return result;
}
}  // namespace

/*static*/
const TfTokenVector &UsdGeomBoundable::GetSchemaAttributeNames(bool includeInherited)
{
  static TfTokenVector localNames = {
    UsdGeomTokens->extent,
  };
  static TfTokenVector allNames = _ConcatenateAttributeNames(UsdGeomXformable::GetSchemaAttributeNames(true),
                                                             localNames);

  if (includeInherited)
    return allNames;
  else
    return localNames;
}

WABI_NAMESPACE_END

// ===================================================================== //
// Feel free to add custom code below this line. It will be preserved by
// the code generator.
//
// Just remember to wrap code in the appropriate delimiters:
// 'WABI_NAMESPACE_BEGIN', 'WABI_NAMESPACE_END'.
// ===================================================================== //
// --(BEGIN CUSTOM CODE)--
