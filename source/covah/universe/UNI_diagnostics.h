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

#pragma once

#include "UNI_api.h"
#include "UNI_context.h"
#include "UNI_scene.h"
#include "UNI_system.h"

#include <wabi/usdImaging/usdImagingGL/engine.h>

typedef std::shared_ptr<class wabi::UsdImagingGLEngine> EnginePtr;

COVAH_UNIVERSE_API
void UNI_enable_all_debug_codes(void);

COVAH_UNIVERSE_API
void UNI_diagnostics_check(void);

COVAH_UNIVERSE_API
void UNI_diagnostics_hydra_test(void);