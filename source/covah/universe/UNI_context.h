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
#include "UNI_object.h"
#include "UNI_scene.h"
#include "UNI_system.h"

#include "CKE_context.h"

#include "UNI_path_defaults.h"

WABI_NAMESPACE_BEGIN

/**
 * Pixar Stage IO
 * - Stage Creation.
 * - Stage Destruction
 * - Opening Stages.
 * - Saving Stages. */

COVAH_UNIVERSE_API
void UNI_create_stage(const cContext &C);

COVAH_UNIVERSE_API
void UNI_destroy(const cContext &C);

COVAH_UNIVERSE_API
void UNI_open_stage(const cContext &C);

COVAH_UNIVERSE_API
void UNI_save_stage(const cContext &C);

/**
 * Pixar Stage Defaults
 * - Covah GUI defaults.
 * - Covah Scene defaults. */

COVAH_UNIVERSE_API
void UNI_set_defaults(const cContext &C);

COVAH_UNIVERSE_API
void UNI_author_default_scene(const cContext &C);

#define COVAH_UNIVERSE_CREATE_CHILD(i) Define(CTX_data_stage(i), prim->path.AppendPath(stagepath))
#define COVAH_UNIVERSE_CREATE(g) Define(CTX_data_stage(g), stagepath)
#define COVAH_PRIM_OPERATOR_CREATE(x, y) CTX_data_stage(x)->DefinePrim(SdfPath(COVAH_PATH_DEFAULTS::COVAH_OPERATORS).AppendPath(SdfPath(y)))


WABI_NAMESPACE_END