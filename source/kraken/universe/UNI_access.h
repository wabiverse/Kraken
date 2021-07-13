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

/* Keep Sorted. */
extern UniverseObject UNI_Area;
extern UniverseObject UNI_Region;
extern UniverseObject UNI_Screen;
extern UniverseObject UNI_Window;
extern UniverseObject UNI_WorkSpace;
extern UniverseObject UNI_Object;

void UNI_main_pointer_create(Main *kmain, PointerUNI *r_ptr);

ObjectRegisterFunc UNI_object_register(UniverseObject *type);
ObjectUnregisterFunc UNI_object_unregister(UniverseObject *type);

const char *UNI_object_identifier(const UniverseObject *type);

WABI_NAMESPACE_END