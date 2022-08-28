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
 * Copyright 2022, Wabi Animation Studios, Ltd. Co.
 */

/**
 * @file
 * KRAKEN Kernel.
 * Purple Underground.
 */

#include "USD_scene.h"
#include "USD_object.h"

#include "KKE_main.h"
#include "KKE_scene.h"

#include "KKE_kraken_prim.h"

#include "wabi/base/tf/registryManager.h"

KRAKEN_NAMESPACE_BEGIN

// TF_REGISTRY_FUNCTION(TfType)
// {
//   TfType::Define<Scene, TfType::Bases<KrakenPrim>>();
//   TfType::AddAlias<UsdSchemaBase, Scene>("Scene");
// }

Scene::Scene(const wabi::UsdStageRefPtr &stage)
  : stage(stage)
{}

Scene::Scene(const std::string &identifier, const wabi::UsdPrim &prim)
  : stage(wabi::UsdStage::CreateNew(identifier))
{}

Scene::Scene(const std::string &identifier, const wabi::UsdSchemaBase &schemaObj)
  : stage(wabi::UsdStage::CreateNew(identifier))
{}

Scene::~Scene() {}

KRAKEN_NAMESPACE_END