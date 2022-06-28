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
#include "UNI_types.h"

WABI_NAMESPACE_BEGIN

const PointerLUXO PointerLUXO_NULL = {NULL};

PointerLUXO LUXO_StageData;
PointerLUXO LUXO_KrakenPixar;
PointerLUXO LUXO_Context;
PointerLUXO LUXO_Struct;
PointerLUXO LUXO_Window;
PointerLUXO LUXO_WorkSpace;
PointerLUXO LUXO_Screen;
PointerLUXO LUXO_Area;
PointerLUXO LUXO_Region;

ObjectRegisterFunc LUXO_struct_register(const PointerLUXO *ptr)
{
  return ptr->reg;
}

ObjectUnregisterFunc LUXO_struct_unregister(PointerLUXO *ptr)
{
  do {
    if (ptr->unreg) {
      return ptr->unreg;
    }
  } while ((ptr = ptr));

  return NULL;
}

const char *LUXO_object_identifier(const PointerLUXO &ptr)
{
  return ptr.identifier;
}

bool LUXO_struct_is_a(const PointerLUXO *type, const PointerLUXO *srna)
{
  if (!type) {
    return false;
  }

  const auto &children = type->type.GetAllChildren();
  for (auto base : children) {
    if (srna && (base == srna->type)) {
      return true;
    }
  }

  return false;
}

void **LUXO_struct_instance(PointerLUXO *ptr)
{
  PointerLUXO *type = ptr->ptr;

  do {
    if (type->instance) {
      return type->instance(ptr);
    }
  } while ((type = type->ptr));

  return NULL;
}

const char *LUXO_property_type(PointerLUXO *ptr)
{
  return ptr->type.GetTypeName().GetText();
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

PropertyLUXO *LUXO_object_find_property(PointerLUXO *ptr, const char *identifier)
{
  // if (identifier[0] == '[' && identifier[1] == '"') { /* "  (dummy comment to avoid confusing
  // some
  //                                                      * function lists in text editors) */
  //   /* id prop lookup, not so common */
  //   PropertyLUXO *r_prop = NULL;
  //   PointerLUXO r_ptr; /* only support single level props */
  //   if (UNI_path_resolve_property(ptr, identifier, &r_ptr, &r_prop) && (r_ptr.type == ptr->type)
  //   &&
  //       (r_ptr.data == ptr->data)) {
  //     return r_prop;
  //   }
  // }
  // else {
  //   /* most common case */
  //   PropertyLUXO *iterprop = UNI_object_iterator_property(ptr->type);
  //   PointerLUXO propptr;

  //   if (UNI_property_collection_lookup_string(ptr, iterprop, identifier, &propptr)) {
  //     return propptr.data;
  //   }
  // }

  return *ptr->collection.cbegin();
}

void LUXO_property_collection_begin(PointerLUXO *ptr,
                                    PropertyLUXO *prop,
                                    CollectionPropertyLUXO iter)
{
  iter.push_back(prop);
  iter.begin();
}

void LUXO_main_pointer_create(struct Main *main, PointerLUXO *r_ptr)
{
  r_ptr->owner_id = NULL;
  r_ptr->ptr = &LUXO_StageData;
  r_ptr->data = main;
}

// BlenderDefRNA DefRNA = {
//     .sdna = NULL,
//     .structs = {NULL, NULL},
//     .allocs = {NULL, NULL},
//     .laststruct = NULL,
//     .error = 0,
//     .silent = false,
//     .preprocess = false,
//     .verify = true,
//     .animate = true,
//     .make_overridable = false,
// };

void *LUXO_struct_py_type_get(PointerLUXO *srna)
{
  return srna->py_type;
}

void LUXO_struct_py_type_set(PointerLUXO *srna, void *type)
{
  srna->py_type = type;
}

void LUXO_pointer_create(PointerLUXO *type, void *data, PointerLUXO *r_ptr)
{
  r_ptr->ptr = type;
  r_ptr->data = data;

  if (data) {
    while (r_ptr->ptr && r_ptr->ptr->refine) {
      PointerLUXO *rtype = r_ptr->ptr->refine(r_ptr);

      if (rtype == r_ptr->ptr) {
        break;
      }
      r_ptr->ptr = rtype;
    }
  }
}

std::vector<PointerLUXO *> &LUXO_struct_type_functions(PointerLUXO *srna)
{
  return srna->functions;
}

int LUXO_function_flag(FunctionLUXO *func)
{
  return func->flag;
}

const char *LUXO_function_identifier(FunctionLUXO *func)
{
  return func->identifier;
}

const char *LUXO_struct_identifier(const PointerLUXO *type)
{
  return type->identifier;
}

/**
 * Use for sub-typing so we know which ptr is used for a #PointerLUXO.
 */
PointerLUXO *srna_from_ptr(PointerLUXO *ptr)
{
  if (ptr->ptr == (PointerLUXO *)&LUXO_Struct) {
    return (PointerLUXO *)ptr->data;
  }

  return ptr->ptr;
}

KrakenPIXAR KRAKEN_PIXAR = {
  .structs =
    {&LUXO_Struct, &LUXO_Window, &LUXO_WorkSpace, &LUXO_Screen, &LUXO_Area, &LUXO_Region}
};

void LUXO_kraken_luxo_pointer_create(PointerLUXO *r_ptr)
{
  r_ptr->owner_id = NULL;
  r_ptr->ptr = &LUXO_KrakenPixar;
  r_ptr->data = &KRAKEN_PIXAR;
}

WABI_NAMESPACE_END