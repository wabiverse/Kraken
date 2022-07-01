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
 * Luxo.
 * The Universe Gets Animated.
 */

#pragma once

#include "LUXO_runtime.h"

#include "KKE_context.h"
#include "KKE_main.h"
#include "KKE_utils.h"

#include "USD_api.h"
#include "USD_types.h"
#include "USD_wm_types.h"
#include "USD_object.h"

WABI_NAMESPACE_BEGIN

void PRIM_def_struct_ptr(KrakenSTAGE kstage,
                         const SdfPath &identifier,
                         KrakenPRIM *r_ptr,
                         const TfToken &from = TfToken());

void PRIM_def_struct(KrakenSTAGE kstage,
                     const SdfPath &identifier,
                     KrakenPRIM *r_ptr,
                     const TfToken &from = TfToken());

WABI_NAMESPACE_END
