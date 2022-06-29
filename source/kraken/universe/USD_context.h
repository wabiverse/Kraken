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

#include "USD_api.h"
#include "USD_area.h"
#include "USD_object.h"
#include "USD_region.h"
#include "USD_scene.h"
#include "USD_screen.h"
#include "USD_system.h"
#include "USD_window.h"

#include "KKE_context.h"

WABI_NAMESPACE_BEGIN

/**
 * Pixar Stage IO
 * - Stage Creation.
 * - Stage Destruction
 * - Opening Stages.
 * - Saving Stages. */
void USD_create_stage(kContext *C);
void USD_destroy(kContext *C);
void USD_open_stage(kContext *C);
void USD_save_stage(kContext *C);

/**
 * Pixar Stage Defaults
 * - Kraken GUI defaults.
 * - Kraken Scene defaults. */
void USD_set_defaults(kContext *C);


WABI_NAMESPACE_END