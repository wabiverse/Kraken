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
#include "wabi/usd/usdGeom/metrics.h"

KRAKEN_NAMESPACE_BEGIN

// TF_REGISTRY_FUNCTION(TfType)
// {
//   TfType::Define<Scene, TfType::Bases<KrakenPrim>>();
//   TfType::AddAlias<UsdSchemaBase, Scene>("Scene");
// }

double KKE_scene_unit_scale(const UnitSettings *unit, const int unit_type, double value, const wabi::UsdStageRefPtr &stage)
{
  if (unit->system == USER_UNIT_NONE) {
    /* Never apply scale_length when not using a unit setting! */
    if (UsdGeomStageHasAuthoredMetersPerUnit(stage)) {
      /* Use existing stage meters if present. */
      return UsdGeomGetStageMetersPerUnit(stage);
    }

    /* Good luck. */
    return value;
  }

  static double metersPerUnit = 0;

  switch (unit_type) {
    case K_UNIT_LENGTH:
    case K_UNIT_VELOCITY:
    case K_UNIT_ACCELERATION:
      metersPerUnit = value * (double)unit->scale_length;
      wabi::UsdGeomSetStageMetersPerUnit(stage, metersPerUnit);
      return metersPerUnit;
    case K_UNIT_AREA:
    case K_UNIT_POWER:
      metersPerUnit = value * pow(unit->scale_length, 2);
      wabi::UsdGeomSetStageMetersPerUnit(stage, metersPerUnit);
      return metersPerUnit;
    case K_UNIT_VOLUME:
      metersPerUnit = value * pow(unit->scale_length, 3);
      wabi::UsdGeomSetStageMetersPerUnit(stage, metersPerUnit);
      return metersPerUnit;
    case K_UNIT_MASS:
      metersPerUnit = value * pow(unit->scale_length, 3);
      wabi::UsdGeomSetStageMetersPerUnit(stage, metersPerUnit);
      return metersPerUnit;
    case K_UNIT_CAMERA: /* *Do not* use scene's unit scale for camera focal lens! See T42026. */
    default:
      return value;
  }
}

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