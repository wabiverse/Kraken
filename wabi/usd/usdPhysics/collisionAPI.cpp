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
#include "wabi/usd/usdPhysics/collisionAPI.h"
#include "wabi/usd/usd/schemaRegistry.h"
#include "wabi/usd/usd/typed.h"
#include "wabi/usd/usd/tokens.h"

#include "wabi/usd/sdf/types.h"
#include "wabi/usd/sdf/assetPath.h"

WABI_NAMESPACE_BEGIN

// Register the schema with the TfType system.
TF_REGISTRY_FUNCTION(TfType)
{
  TfType::Define<UsdPhysicsCollisionAPI,
                 TfType::Bases<UsdAPISchemaBase>>();
}

TF_DEFINE_PRIVATE_TOKENS(
  _schemaTokens,
  (PhysicsCollisionAPI));

/* virtual */
UsdPhysicsCollisionAPI::~UsdPhysicsCollisionAPI()
{
}

/* static */
UsdPhysicsCollisionAPI
UsdPhysicsCollisionAPI::Get(const UsdStagePtr &stage, const SdfPath &path)
{
  if (!stage)
  {
    TF_CODING_ERROR("Invalid stage");
    return UsdPhysicsCollisionAPI();
  }
  return UsdPhysicsCollisionAPI(stage->GetPrimAtPath(path));
}


/* virtual */
UsdSchemaKind UsdPhysicsCollisionAPI::GetSchemaKind() const
{
  return UsdPhysicsCollisionAPI::schemaKind;
}

/* static */
bool UsdPhysicsCollisionAPI::CanApply(
  const UsdPrim &prim,
  std::string *whyNot)
{
  return prim.CanApplyAPI<UsdPhysicsCollisionAPI>(whyNot);
}

/* static */
UsdPhysicsCollisionAPI
UsdPhysicsCollisionAPI::Apply(const UsdPrim &prim)
{
  if (prim.ApplyAPI<UsdPhysicsCollisionAPI>())
  {
    return UsdPhysicsCollisionAPI(prim);
  }
  return UsdPhysicsCollisionAPI();
}

/* static */
const TfType &
UsdPhysicsCollisionAPI::GetStaticTfType()
{
  static TfType tfType = TfType::Find<UsdPhysicsCollisionAPI>();
  return tfType;
}

/* static */
bool UsdPhysicsCollisionAPI::IsTypedSchema()
{
  static bool isTyped = GetStaticTfType().IsA<UsdTyped>();
  return isTyped;
}

/* virtual */
const TfType &
UsdPhysicsCollisionAPI::GetTfType() const
{
  return GetStaticTfType();
}

UsdAttribute
UsdPhysicsCollisionAPI::GetCollisionEnabledAttr() const
{
  return GetPrim().GetAttribute(UsdPhysicsTokens->physicsCollisionEnabled);
}

UsdAttribute
UsdPhysicsCollisionAPI::CreateCollisionEnabledAttr(VtValue const &defaultValue, bool writeSparsely) const
{
  return UsdSchemaBase::_CreateAttr(UsdPhysicsTokens->physicsCollisionEnabled,
                                    SdfValueTypeNames->Bool,
                                    /* custom = */ false,
                                    SdfVariabilityVarying,
                                    defaultValue,
                                    writeSparsely);
}

UsdRelationship
UsdPhysicsCollisionAPI::GetSimulationOwnerRel() const
{
  return GetPrim().GetRelationship(UsdPhysicsTokens->physicsSimulationOwner);
}

UsdRelationship
UsdPhysicsCollisionAPI::CreateSimulationOwnerRel() const
{
  return GetPrim().CreateRelationship(UsdPhysicsTokens->physicsSimulationOwner,
                                      /* custom = */ false);
}

namespace
{
static inline TfTokenVector
_ConcatenateAttributeNames(const TfTokenVector &left, const TfTokenVector &right)
{
  TfTokenVector result;
  result.reserve(left.size() + right.size());
  result.insert(result.end(), left.begin(), left.end());
  result.insert(result.end(), right.begin(), right.end());
  return result;
}
}  // namespace

/*static*/
const TfTokenVector &
UsdPhysicsCollisionAPI::GetSchemaAttributeNames(bool includeInherited)
{
  static TfTokenVector localNames = {
    UsdPhysicsTokens->physicsCollisionEnabled,
  };
  static TfTokenVector allNames =
    _ConcatenateAttributeNames(
      UsdAPISchemaBase::GetSchemaAttributeNames(true),
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