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

#include "LUXO_runtime.h"

#include "KKE_context.h"
#include "KKE_main.h"
#include "KKE_utils.h"

#include "USD_api.h"
#include "USD_ID.h"
#include "USD_types.h"
#include "USD_wm_types.h"
#include "USD_object.h"

KRAKEN_NAMESPACE_BEGIN

struct Main;
struct ReportList;
struct Scene;
struct kContext;
struct KrakenPROP;

extern KrakenSTAGE KRAKEN_STAGE;

extern KrakenPRIM LUXO_StageData;
extern KrakenPRIM LUXO_KrakenPixar;
extern KrakenPRIM LUXO_Context;
extern KrakenPRIM LUXO_Struct;
extern KrakenPRIM LUXO_Window;
extern KrakenPRIM LUXO_WorkSpace;
extern KrakenPRIM LUXO_Screen;
extern KrakenPRIM LUXO_Area;
extern KrakenPRIM LUXO_Region;

#define LUXO_POINTER_INVALIDATE(ptr) \
  {                                  \
    (ptr)->ptr = NULL;               \
    (ptr)->owner_id = NULL;          \
  }                                  \
  (void)0

void LUXO_init(void);

/* remove this once we go back to UsdStage::CreateInMemory */
void LUXO_save_usd(void);

void LUXO_kraken_luxo_pointer_create(KrakenPRIM *r_ptr);
void LUXO_main_pointer_create(Main *main, KrakenPRIM *r_ptr);
void LUXO_pointer_create(KrakenPRIM *type, void *data, KrakenPRIM *r_ptr);
void LUXO_pointer_create(ID *id, KrakenPRIM *type, void *data, KrakenPRIM *r_ptr);
void LUXO_stage_pointer_ensure(KrakenPRIM *r_ptr);

void *LUXO_struct_py_type_get(KrakenPRIM *srna);
void LUXO_struct_py_type_set(KrakenPRIM *srna, void *type);

PropertyType LUXO_property_type_enum(KrakenPROP *prop);
PropertyType LUXO_property_type(KrakenPROP *prop)

ObjectRegisterFunc LUXO_struct_register(const KrakenPRIM *ptr);
ObjectUnregisterFunc LUXO_struct_unregister(KrakenPRIM *ptr);

void LUXO_object_find_property(KrakenPRIM *ptr, const TfToken &name, KrakenPROP *r_ptr);
void **LUXO_struct_instance(KrakenPRIM *ptr);
const char *LUXO_struct_identifier(const KrakenPRIM *type);
bool LUXO_struct_is_a(const KrakenPRIM *type, const KrakenPRIM *srna);

std::vector<KrakenPRIM *> &LUXO_struct_type_functions(KrakenPRIM *srna);
const char *LUXO_function_identifier(KrakenFUNC *func);

int LUXO_function_flag(KrakenFUNC *func);

KrakenPRIM *srna_from_ptr(KrakenPRIM *ptr);

UsdCollectionsVector LUXO_property_collection_begin(KrakenPRIM *ptr, const TfToken &name);

void LUXO_set_stage_ctx(kContext *C);

wabi::UsdStageWeakPtr LUXO_get_stage();

KRAKEN_NAMESPACE_END