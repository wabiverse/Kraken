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

#include "KKE_context.h"
#include "KKE_main.h"

#include "UNI_api.h"
#include "UNI_types.h"
#include "UNI_wm_types.h"

#include "UNI_object.h"

WABI_NAMESPACE_BEGIN

/* Types */
extern KrakenUNI KRAKEN_UNI;

/* Keep Sorted. */
extern ObjectUNI UNI_Area;
extern ObjectUNI UNI_Context;
extern ObjectUNI UNI_KrakenUNI;
extern ObjectUNI UNI_Region;
extern ObjectUNI UNI_Screen;
extern ObjectUNI UNI_Window;
extern ObjectUNI UNI_WorkSpace;
extern ObjectUNI UNI_Object;

void UNI_main_pointer_create(Main *kmain, PointerUNI *r_ptr);

void UNI_kraken_uni_pointer_create(PointerUNI *r_ptr);

ObjectRegisterFunc UNI_object_register(ObjectUNI *type);
ObjectUnregisterFunc UNI_object_unregister(ObjectUNI *type);

PropertyUNI *UNI_object_find_property(PointerUNI *ptr, const char *identifier);
void **UNI_object_instance(PointerUNI *ptr);
const char *UNI_object_identifier(const ObjectUNI *type);

SdfValueTypeName UNI_property_type(PropertyUNI *prop);

void UNI_property_collection_begin(PointerUNI *ptr,
                                   PropertyUNI *prop,
                                   CollectionPropertyUNI iter);

WABI_NAMESPACE_END