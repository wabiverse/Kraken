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
#include "wabi/usd/usdGeom/motionAPI.h"
#include "wabi/usd/usd/schemaRegistry.h"
#include "wabi/usd/usd/tokens.h"
#include "wabi/usd/usd/typed.h"

#include "wabi/usd/sdf/assetPath.h"
#include "wabi/usd/sdf/types.h"

WABI_NAMESPACE_BEGIN

// Register the schema with the TfType system.
TF_REGISTRY_FUNCTION(TfType)
{
  TfType::Define<UsdGeomMotionAPI, TfType::Bases<UsdAPISchemaBase>>();
}

TF_DEFINE_PRIVATE_TOKENS(_schemaTokens, (MotionAPI));

/* virtual */
UsdGeomMotionAPI::~UsdGeomMotionAPI()
{}

/* static */
UsdGeomMotionAPI UsdGeomMotionAPI::Get(const UsdStagePtr &stage, const SdfPath &path)
{
  if (!stage)
  {
    TF_CODING_ERROR("Invalid stage");
    return UsdGeomMotionAPI();
  }
  return UsdGeomMotionAPI(stage->GetPrimAtPath(path));
}

/* virtual */
UsdSchemaKind UsdGeomMotionAPI::GetSchemaKind() const
{
  return UsdGeomMotionAPI::schemaKind;
}

/* static */
UsdGeomMotionAPI UsdGeomMotionAPI::Apply(const UsdPrim &prim)
{
  if (prim.ApplyAPI<UsdGeomMotionAPI>())
  {
    return UsdGeomMotionAPI(prim);
  }
  return UsdGeomMotionAPI();
}

/* static */
const TfType &UsdGeomMotionAPI::GetStaticTfType()
{
  static TfType tfType = TfType::Find<UsdGeomMotionAPI>();
  return tfType;
}

/* static */
bool UsdGeomMotionAPI::IsTypedSchema()
{
  static bool isTyped = GetStaticTfType().IsA<UsdTyped>();
  return isTyped;
}

/* virtual */
const TfType &UsdGeomMotionAPI::GetType() const
{
  return GetStaticTfType();
}

UsdAttribute UsdGeomMotionAPI::GetVelocityScaleAttr() const
{
  return GetPrim().GetAttribute(UsdGeomTokens->motionVelocityScale);
}

UsdAttribute UsdGeomMotionAPI::CreateVelocityScaleAttr(VtValue const &defaultValue, bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(UsdGeomTokens->motionVelocityScale,
                                    SdfValueTypeNames->Float,
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
const TfTokenVector &UsdGeomMotionAPI::GetSchemaAttributeNames(bool includeInherited)
{
  static TfTokenVector localNames = {
    UsdGeomTokens->motionVelocityScale,
  };
  static TfTokenVector allNames = _ConcatenateAttributeNames(UsdAPISchemaBase::GetSchemaAttributeNames(true),
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

WABI_NAMESPACE_BEGIN

float UsdGeomMotionAPI::ComputeVelocityScale(UsdTimeCode time) const
{
  UsdPrim prim = GetPrim();
  UsdPrim pseudoRoot = prim.GetStage()->GetPseudoRoot();
  float velocityScale = 1.0;

  while (prim != pseudoRoot)
  {
    UsdAttribute vsAttr = prim.GetAttribute(UsdGeomTokens->motionVelocityScale);
    if (vsAttr.HasAuthoredValue() && vsAttr.Get(&velocityScale, time))
    {
      return velocityScale;
    }
    prim = prim.GetParent();
  }

  return velocityScale;
}

WABI_NAMESPACE_END
