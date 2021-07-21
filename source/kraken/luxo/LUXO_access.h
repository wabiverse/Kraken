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

/* Types */
extern KrakenUNI KRAKEN_LUXO;

/* Keep Sorted. */
extern KrakenPrim LUXO_Area;
extern KrakenPrim LUXO_Context;
extern KrakenPrim LUXO_KrakenData;
extern KrakenPrim LUXO_KrakenLUXO;
extern KrakenPrim LUXO_Region;
extern KrakenPrim LUXO_Screen;
extern KrakenPrim LUXO_Window;
extern KrakenPrim LUXO_WorkSpace;
extern KrakenPrim LUXO_Object;

void LUXO_kraken_luxo_pointer_create(PointerUNI *r_ptr);
void LUXO_main_pointer_create(Main *main, PointerUNI *r_ptr);

ObjectRegisterFunc LUXO_object_register(KrakenPrim *type);
ObjectUnregisterFunc LUXO_object_unregister(KrakenPrim *type);

PropertyUNI *LUXO_object_find_property(PointerUNI *ptr, const char *identifier);
void **LUXO_object_instance(PointerUNI *ptr);
const char *LUXO_object_identifier(const KrakenPrim *type);

SdfValueTypeName LUXO_property_type(PropertyUNI *prop);

void LUXO_property_collection_begin(PointerUNI *ptr,
                                    PropertyUNI *prop,
                                    CollectionPropertyUNI iter);

WABI_NAMESPACE_END