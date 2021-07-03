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
 * Universe.
 * Set the Stage.
 */

#include "UNI_pixar_utils.h"

#include <wabi/usd/usd/stage.h>

WABI_NAMESPACE_BEGIN


void UNI_pixutil_convert(const std::string &args)
{
  UsdStageRefPtr stage = UsdStage::Open("/home/furby/animation/pixar/kitchen.usd");
  stage->Flatten();
  if (stage->Export("/home/furby/animation/pixar/kitchen.usda", true))
  {
    return;
  }
}


WABI_NAMESPACE_END