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

ObjectRegisterFunc UNI_object_register(ObjectUNI *type)
{
  return type->reg;
}

ObjectUnregisterFunc UNI_object_unregister(ObjectUNI *type)
{
  do {
    if (type->unreg) {
      return type->unreg;
    }
  } while ((type = type->base));

  return NULL;
}

const char *UNI_object_identifier(const ObjectUNI *type)
{
  return type->identifier;
}

void **UNI_object_instance(PointerUNI *ptr)
{
  ObjectUNI *type = ptr->type;

  do {
    if (type->instance) {
      return type->instance(ptr);
    }
  } while ((type = type->base));

  return NULL;
}

SdfValueTypeName UNI_property_type(PropertyUNI *prop)
{
  return prop->type;
}

PropertyUNI *UNI_object_find_property(PointerUNI *ptr, const char *identifier)
{
  // if (identifier[0] == '[' && identifier[1] == '"') { /* "  (dummy comment to avoid confusing some
  //                                                      * function lists in text editors) */
  //   /* id prop lookup, not so common */
  //   PropertyUNI *r_prop = NULL;
  //   PointerUNI r_ptr; /* only support single level props */
  //   if (UNI_path_resolve_property(ptr, identifier, &r_ptr, &r_prop) && (r_ptr.type == ptr->type) &&
  //       (r_ptr.data == ptr->data)) {
  //     return r_prop;
  //   }
  // }
  // else {
  //   /* most common case */
  //   PropertyUNI *iterprop = UNI_object_iterator_property(ptr->type);
  //   PointerUNI propptr;

  //   if (UNI_property_collection_lookup_string(ptr, iterprop, identifier, &propptr)) {
  //     return propptr.data;
  //   }
  // }

  return NULL;
}

void UNI_property_collection_begin(PointerUNI *ptr,
                                   PropertyUNI *prop,
                                   CollectionPropertyUNI iter)
{
  iter.push_back(prop);
  iter.begin();
}

void UNI_main_pointer_create(struct Main *main, PointerUNI *r_ptr)
{
  r_ptr->path = SdfPath("/Main");
  r_ptr->type = &UNI_KrakenData;
  r_ptr->data = main;
}

void UNI_kraken_uni_pointer_create(PointerUNI *r_ptr)
{
  r_ptr->path = SdfPath("/Kraken");
  r_ptr->type = &UNI_KrakenUNI;
  r_ptr->data = &KRAKEN_UNI;
}

WABI_NAMESPACE_END