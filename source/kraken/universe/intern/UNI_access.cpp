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

#include "KKE_utils.h"

#include "UNI_api.h"
#include "UNI_object.h"
#include "UNI_access.h"

WABI_NAMESPACE_BEGIN

ObjectRegisterFunc UNI_object_register(UniverseObject *type)
{
  return type->reg;
}

ObjectUnregisterFunc UNI_object_unregister(UniverseObject *type)
{
  do {
    if (type->unreg) {
      return type->unreg;
    }
  } while ((type = type->base));

  return NULL;
}

const char *UNI_object_identifier(const UniverseObject *type)
{
  return type->identifier;
}

WABI_NAMESPACE_END