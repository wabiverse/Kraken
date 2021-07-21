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

#include "LUXO_runtime.h"
#include "LUXO_access.h"

#include "KKE_utils.h"

#include "UNI_api.h"
#include "UNI_object.h"

WABI_NAMESPACE_BEGIN

KrakenUNI KRAKEN_LUXO;

KrakenPrim LUXO_Area;
KrakenPrim LUXO_Context;
KrakenPrim LUXO_KrakenData;
KrakenPrim LUXO_KrakenLUXO;
KrakenPrim LUXO_Region;
KrakenPrim LUXO_Screen;
KrakenPrim LUXO_Window;
KrakenPrim LUXO_WorkSpace;
KrakenPrim LUXO_Object;

ObjectRegisterFunc LUXO_object_register(KrakenPrim *type)
{
  return type->reg;
}

ObjectUnregisterFunc LUXO_object_unregister(KrakenPrim *type)
{
  do
  {
    if (type->unreg)
    {
      return type->unreg;
    }
  } while ((type = type->base));

  return NULL;
}

const char *LUXO_object_identifier(const KrakenPrim *type)
{
  return type->identifier;
}

void **LUXO_object_instance(PointerUNI *ptr)
{
  KrakenPrim *type = ptr->type;

  do
  {
    if (type->instance)
    {
      return type->instance(ptr);
    }
  } while ((type = type->base));

  return NULL;
}

SdfValueTypeName LUXO_property_type(PropertyUNI *prop)
{
  return prop->type;
}

bool UNI_enum_identifier(TfEnum item, const int value, const char **r_identifier)
{
//   const int i = LUXO_enum_from_value(item, value);
//   if (i != -1) {
//     *r_identifier = item[i].identifier;
//     return true;
//   }
  return false;
}

PropertyUNI *LUXO_object_find_property(PointerUNI *ptr, const char *identifier)
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

void LUXO_property_collection_begin(PointerUNI *ptr,
                                   PropertyUNI *prop,
                                   CollectionPropertyUNI iter)
{
  iter.push_back(prop);
  iter.begin();
}

void LUXO_main_pointer_create(struct Main *main, PointerUNI *r_ptr)
{
  r_ptr->path = SdfPath("/Main");
  r_ptr->type = &LUXO_KrakenData;
  r_ptr->data = main;
}

void LUXO_kraken_luxo_pointer_create(PointerUNI *r_ptr)
{
  r_ptr->path = SdfPath("/Kraken");
  r_ptr->type = &LUXO_KrakenLUXO;
  r_ptr->data = &KRAKEN_LUXO;
}

WABI_NAMESPACE_END