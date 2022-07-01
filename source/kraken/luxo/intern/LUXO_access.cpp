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
#include "LUXO_main.h"

#include "KKE_utils.h"

#include "USD_api.h"
#include "USD_object.h"
#include "USD_types.h"

WABI_NAMESPACE_BEGIN

KrakenPRIM LUXO_StageData;
KrakenPRIM LUXO_KrakenPixar;
KrakenPRIM LUXO_Context;
KrakenPRIM LUXO_Struct;
KrakenPRIM LUXO_Window;
KrakenPRIM LUXO_WorkSpace;
KrakenPRIM LUXO_Screen;
KrakenPRIM LUXO_Area;
KrakenPRIM LUXO_Region;

KrakenSTAGE KRAKEN_STAGE = {};

static void LUXO_struct_init(void)
{
  KrakenPRIM sprim = KRAKEN_STAGE->DefinePrim(STAGE("structs"));

  UsdCollectionAPI capi = UsdCollectionAPI::Apply(sprim, TfToken("structs"));
  capi.CreateIncludesRel().AddTarget(sprim.GetPath());
}

void LUXO_init(void)
{
  LUXO_struct_init();
  LUXO_main(KRAKEN_STAGE);

  /* remove this once we go back to UsdStage::CreateInMemory */
  LUXO_save_usd();
}

ObjectRegisterFunc LUXO_struct_register(const KrakenPRIM *ptr)
{
  return ptr->reg;
}

ObjectUnregisterFunc LUXO_struct_unregister(KrakenPRIM *ptr)
{
  do {
    if (ptr->unreg) {
      return ptr->unreg;
    }
  } while ((ptr = ptr));

  return NULL;
}

const char *LUXO_object_identifier(const KrakenPRIM &ptr)
{
  return ptr.identifier;
}

bool LUXO_struct_is_a(const KrakenPRIM *type, const KrakenPRIM *srna)
{
  const KrakenPRIM *base;

  if (!type) {
    return false;
  }

  for (base = type; base; base = base->base) {
    if (base == srna) {
      return true;
    }
  }

  return false;
}

void **LUXO_struct_instance(KrakenPRIM *ptr)
{
  KrakenPRIM *type = ptr->type;

  do {
    if (type->instance) {
      return type->instance(ptr);
    }
  } while ((type = type->base));

  return NULL;
}

KrakenPROP *rna_ensure_property(KrakenPROP *prop)
{
  if (prop) {
    return prop;
  }
}

PropertyType LUXO_property_type_enum(KrakenPROP *prop)
{
  return rna_ensure_property(prop)->type;
}

const char *LUXO_property_type(KrakenPRIM *ptr)
{
  return ptr->type->GetTypeName().GetText();
}

bool USD_enum_identifier(TfEnum item, const int value, const char **r_identifier)
{
  //   const int i = LUXO_enum_from_value(item, value);
  //   if (i != -1) {
  //     *r_identifier = item[i].identifier;
  //     return true;
  //   }
  return false;
}

KrakenPROP *LUXO_object_find_property(KrakenPRIM *ptr, const TfToken &name)
{
  KrakenPRIM prim;
  KrakenPROP prop;

  if (!ptr->IsValid()) {
    *ptr = ptr->GetStage()->GetPseudoRoot();
  }

  prim = ptr->GetPrim();
  prop = prim.GetProperty(name);

  if (!prop || !prop.IsValid()) {
    TF_WARN("%s has no property %s, this is not supposed to happen!",
            prim.GetName().GetText(),
            name.GetText());
  }

  return &prop;
}

UsdCollectionsVector LUXO_property_collection_begin(KrakenPRIM *ptr, const TfToken &name)
{
  UsdCollectionAPI collection;
  UsdPrim prim;

  prim = ptr->GetPrim();

  collection = UsdCollectionAPI::Get(prim, name);
  return collection.GetAllCollections(prim);
}

void LUXO_main_pointer_create(struct Main *main, KrakenPRIM *r_ptr)
{
  *r_ptr = KRAKEN_STAGE->GetPseudoRoot();
  r_ptr->owner_id = NULL;
  r_ptr->type = &LUXO_StageData;
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

int LUXO_property_string_length(KrakenPRIM *ptr, KrakenPROP *prop)
{
  KrakenPROPString *sprop = (KrakenPROPString *)prop;

  KLI_assert(LUXO_property_type_enum(prop) == PROP_STRING);

  if (sprop->length) {
    return sprop->length(ptr);
  }
  if (sprop->length_ex) {
    return sprop->length_ex(ptr, prop);
  }
  return strlen(sprop->defaultvalue);
}

void *LUXO_struct_py_type_get(KrakenPRIM *srna)
{
  return srna->py_type;
}

void LUXO_struct_py_type_set(KrakenPRIM *srna, void *type)
{
  srna->py_type = type;
}

void LUXO_pointer_create(KrakenPRIM *type, void *data, KrakenPRIM *r_ptr)
{
  if (!r_ptr->IsValid()) {
    *r_ptr = KRAKEN_STAGE->GetPseudoRoot();
  }
  r_ptr->owner_id = type->GetName().GetText();
  r_ptr->type = type;
  r_ptr->data = data;

  if (data) {
    while (r_ptr->type && r_ptr->type->refine) {
      KrakenPRIM *rtype = r_ptr->type->refine(r_ptr);

      if (rtype == r_ptr->type) {
        break;
      }
      r_ptr->type = rtype;
    }
  }
}

std::vector<KrakenPRIM *> &LUXO_struct_type_functions(KrakenPRIM *srna)
{
  return srna->functions;
}

int LUXO_function_flag(KrakenFUNC *func)
{
  return func->flag;
}

const char *LUXO_function_identifier(KrakenFUNC *func)
{
  return func->identifier;
}

const char *LUXO_struct_identifier(const KrakenPRIM *type)
{
  return type->identifier;
}

/**
 * Use for sub-typing so we know which ptr is used for a #KrakenPRIM.
 */
KrakenPRIM *srna_from_ptr(KrakenPRIM *ptr)
{
  if (ptr->type == &LUXO_Struct) {
    return (KrakenPRIM *)ptr->data;
  }

  return ptr->type;
}

/* remove this once we go back to UsdStage::CreateInMemory */
void LUXO_save_usd(void)
{
  KRAKEN_STAGE->GetRootLayer()->Save();
}

/**
 * Use UsdStage::CreateInMemory when we get out of an alpha state
 * for now, this makes it easier to debug scene description - it
 * is located in the Kraken installation /1.50/startup.usda */
KrakenSTAGE::KrakenSTAGE()
  : UsdStageRefPtr(UsdStage::CreateNew(KKE_kraken_globals_init().main->stage_id)),
    structs{&LUXO_Window, &LUXO_WorkSpace, &LUXO_Screen, &LUXO_Area, &LUXO_Region}
{}

void LUXO_kraken_luxo_pointer_create(KrakenPRIM *r_ptr)
{
  *r_ptr = KRAKEN_STAGE->GetPseudoRoot();
  r_ptr->owner_id = NULL;
  r_ptr->type = &LUXO_KrakenPixar;
  r_ptr->data = (void *&)KRAKEN_STAGE;
}

WABI_NAMESPACE_END