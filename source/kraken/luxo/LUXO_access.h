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

#include "UNI_api.h"
#include "UNI_types.h"
#include "UNI_wm_types.h"

#include "UNI_object.h"

WABI_NAMESPACE_BEGIN

struct Main;
struct ReportList;
struct Scene;
struct kContext;

extern KrakenPIXAR KRAKEN_PIXAR;

extern PointerLUXO LUXO_StageData;
extern PointerLUXO LUXO_KrakenPixar;
extern PointerLUXO LUXO_Context;
extern PointerLUXO LUXO_Struct;
extern PointerLUXO LUXO_Window;
extern PointerLUXO LUXO_WorkSpace;
extern PointerLUXO LUXO_Screen;
extern PointerLUXO LUXO_Area;
extern PointerLUXO LUXO_Region;

#define LUXO_POINTER_INVALIDATE(ptr) \
  {                                  \
    (ptr)->ptr = NULL;               \
    (ptr)->owner_id = NULL;          \
  }                                  \
  (void)0

void LUXO_kraken_luxo_pointer_create(PointerLUXO *ptr);
void LUXO_main_pointer_create(Main *main, PointerLUXO *ptr);
void LUXO_pointer_create(PointerLUXO *type, void *data, PointerLUXO *r_ptr);

void *LUXO_struct_py_type_get(PointerLUXO *srna);
void LUXO_struct_py_type_set(PointerLUXO *srna, void *type);

const char *LUXO_property_type(PropertyLUXO *prop);

ObjectRegisterFunc LUXO_struct_register(const PointerLUXO *ptr);
ObjectUnregisterFunc LUXO_struct_unregister(PointerLUXO *ptr);

PropertyLUXO *LUXO_object_find_property(PointerLUXO *ptr, const char *identifier);
void **LUXO_struct_instance(PointerLUXO *ptr);
const char *LUXO_object_identifier(const PointerLUXO &ptr);
const char *LUXO_struct_identifier(const PointerLUXO *type);
const char *LUXO_struct_identifier(const PointerLUXO *type);
bool LUXO_struct_is_a(const PointerLUXO *type, const PointerLUXO *srna);

std::vector<PointerLUXO *> &LUXO_struct_type_functions(PointerLUXO *srna);
const char *LUXO_function_identifier(FunctionLUXO *func);

int LUXO_function_flag(FunctionLUXO *func);

PointerLUXO *srna_from_ptr(PointerLUXO *ptr);

// void LUXO_property_collection_begin(PointerLUXO *ptr,
//                                     PropertyLUXO *prop,
//                                     CollectionPropertyLUXO iter);

WABI_NAMESPACE_END