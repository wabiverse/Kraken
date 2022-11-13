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
 * Luxo.
 * The Universe Gets Animated.
 */

#pragma once

#include <stdbool.h>

#include "LUXO_runtime.h"

#include "KKE_context.h"
#include "KKE_main.h"
#include "KKE_utils.h"

#include "USD_wm_types.h"
#include "USD_api.h"
#include "USD_types.h"
#include "USD_object.h"

#ifdef UNIT_TEST
#  define LUXO_MAX_ARRAY_LENGTH 64
#else
#  define LUXO_MAX_ARRAY_LENGTH 32
#endif

#define LUXO_MAX_ARRAY_DIMENSION 3

KrakenPRIM *PRIM_def_struct_ptr(const KrakenSTAGE &kstage,
                                const wabi::SdfPath &identifier,
                                KrakenPRIM *primfrom = nullptr);

KrakenPRIM *PRIM_def_struct(KrakenPRIM *kprim,
                            const wabi::SdfPath &identifier,
                            const wabi::TfToken &from = wabi::TfToken());

void PRIM_def_struct_identifier(const KrakenSTAGE &kstage,
                                KrakenPRIM *prim,
                                const TfToken &identifier);

void PRIM_def_struct_ui_text(KrakenPRIM *prim,
                             const std::string &ui_name,
                             const std::string &ui_description);

void PRIM_def_boolean(KrakenPRIM *prim,
                      const std::string &identifier,
                      bool default_value,
                      const std::string &ui_name,
                      const std::string &ui_description);

void PRIM_def_asset(KrakenPRIM *prim,
                    const std::string &identifier,
                    const std::string &default_value,
                    const std::string &ui_name,
                    const std::string &ui_description);

void PRIM_def_py_data(KrakenPROP *prop, void *py_data);

void PRIM_def_property_free_pointers_set_py_data_callback(
  void (*py_data_clear_fn)(KrakenPROP *prop));
