/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Copyright 2021, Wabi.
 */

/**
 * @file
 * KRAKEN Kernel.
 * Purple Underground.
 */

#include "UNI_scene.h"
#include "UNI_object.h"

#include "KKE_main.h"
#include "KKE_scene.h"

#include "KKE_kraken_prim.h"

#include "wabi/base/tf/registryManager.h"

WABI_NAMESPACE_BEGIN

TF_REGISTRY_FUNCTION(TfType)
{
  TfType::Define<Scene, TfType::Bases<KrakenPrim>>();
  TfType::AddAlias<UsdSchemaBase, Scene>("Scene");
}

Scene::Scene(const std::string &identifier, const UsdPrim &prim)
  : stage(UsdStage::CreateNew(identifier)),
    KrakenPrim(prim)
{}

Scene::Scene(const std::string &identifier, const UsdSchemaBase &schemaObj)
  : stage(UsdStage::CreateNew(identifier)),
    KrakenPrim(schemaObj)
{}

Scene::~Scene()
{}

UsdSchemaKind Scene::GetSchemaKind() const
{
  return Scene::schemaKind;
}

/* static */
const TfType &Scene::GetStaticTfType()
{
  static TfType tfType = TfType::Find<Scene>();
  return tfType;
}

/* static */
bool Scene::IsTypedSchema()
{
  static bool isTyped = GetStaticTfType().IsA<UsdTyped>();
  return isTyped;
}

/* virtual */
const TfType &Scene::GetType() const
{
  return GetStaticTfType();
}


static bool SceneInitData(KrakenPrim *prim)
{
  TF_MSG("TEST, scene init");

  Scene scene(G.main->stage_id.string(), *prim);
  Stage stage = scene.stage;

  if (ARCH_UNLIKELY(scene.GetPrim().IsValid() != NULL))
  {
    return false;
  }

  stage->SetMetadata(UsdGeomTokens->upAxis, UsdGeomTokens->z);
  stage->GetRootLayer()->SetDocumentation(KRAKEN_FILE_VERSION_HEADER);
  stage->SetColorConfiguration(SdfAssetPath(STRCAT(G.main->datafiles_path, "colormanagement/config.ocio")));
  stage->SetColorManagementSystem(HdxColorCorrectionTokens->openColorIO);

  return true;
}

TF_REGISTRY_FUNCTION_WITH_TAG(KrakenPrimRegistry, KrakenPrim)
{
  RegisterKrakenInitFunction<Scene>(SceneInitData);
}

WABI_NAMESPACE_END