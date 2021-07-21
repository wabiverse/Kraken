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
#include "wabi/usd/usdPhysics/articulationRootAPI.h"
#include "wabi/usd/usd/schemaRegistry.h"
#include "wabi/usd/usd/typed.h"
#include "wabi/usd/usd/tokens.h"

#include "wabi/usd/sdf/types.h"
#include "wabi/usd/sdf/assetPath.h"

WABI_NAMESPACE_BEGIN

// Register the schema with the TfType system.
TF_REGISTRY_FUNCTION(TfType)
{
    TfType::Define<UsdPhysicsArticulationRootAPI,
        TfType::Bases< UsdAPISchemaBase > >();
    
}

TF_DEFINE_PRIVATE_TOKENS(
    _schemaTokens,
    (PhysicsArticulationRootAPI)
);

/* virtual */
UsdPhysicsArticulationRootAPI::~UsdPhysicsArticulationRootAPI()
{
}

/* static */
UsdPhysicsArticulationRootAPI
UsdPhysicsArticulationRootAPI::Get(const UsdStagePtr &stage, const SdfPath &path)
{
    if (!stage) {
        TF_CODING_ERROR("Invalid stage");
        return UsdPhysicsArticulationRootAPI();
    }
    return UsdPhysicsArticulationRootAPI(stage->GetPrimAtPath(path));
}


/* virtual */
UsdSchemaKind UsdPhysicsArticulationRootAPI::GetSchemaKind() const
{
    return UsdPhysicsArticulationRootAPI::schemaKind;
}

/* static */
bool
UsdPhysicsArticulationRootAPI::CanApply(
    const UsdPrim &prim, std::string *whyNot)
{
    return prim.CanApplyAPI<UsdPhysicsArticulationRootAPI>(whyNot);
}

/* static */
UsdPhysicsArticulationRootAPI
UsdPhysicsArticulationRootAPI::Apply(const UsdPrim &prim)
{
    if (prim.ApplyAPI<UsdPhysicsArticulationRootAPI>()) {
        return UsdPhysicsArticulationRootAPI(prim);
    }
    return UsdPhysicsArticulationRootAPI();
}

/* static */
const TfType &
UsdPhysicsArticulationRootAPI::GetStaticTfType()
{
    static TfType tfType = TfType::Find<UsdPhysicsArticulationRootAPI>();
    return tfType;
}

/* static */
bool 
UsdPhysicsArticulationRootAPI::IsTypedSchema()
{
    static bool isTyped = GetStaticTfType().IsA<UsdTyped>();
    return isTyped;
}

/* virtual */
const TfType &
UsdPhysicsArticulationRootAPI::GetTfType() const
{
    return GetStaticTfType();
}

/*static*/
const TfTokenVector&
UsdPhysicsArticulationRootAPI::GetSchemaAttributeNames(bool includeInherited)
{
    static TfTokenVector localNames;
    static TfTokenVector allNames =
        UsdAPISchemaBase::GetSchemaAttributeNames(true);

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
