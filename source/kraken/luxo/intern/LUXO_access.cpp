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

#include "LUXO_runtime.h"
#include "LUXO_access.h"
#include "LUXO_main.h"

#include "KLI_string.h"

#include "KKE_utils.h"
#include "KKE_appdir.h"

#include "USD_api.h"
#include "USD_object.h"
#include "USD_types.h"
#include "USD_scene.h"

#include <wabi/usd/ar/resolver.h>

WABI_NAMESPACE_USING

KRAKEN_NAMESPACE_BEGIN

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
  UsdPrim intern = KRAKEN_STAGE->DefinePrim(wabi::SdfPath("/WabiAnimationStudios"));

  UsdCollectionAPI capi = UsdCollectionAPI::Apply(intern, TfToken("structs"));
  capi.CreateIncludesRel().AddTarget(intern.GetPath().AppendPath(SdfPath("Structs")));
}

void LUXO_init(void)
{
  LUXO_struct_init();
  LUXO_main(KRAKEN_STAGE);

  KKE_tempdir_init(NULL);

  LUXO_save_usd();
}

wabi::UsdStageWeakPtr LUXO_get_stage()
{
  return KRAKEN_STAGE;
}

void LUXO_set_stage_ctx(kContext *C)
{
  Scene *scene = new Scene(KRAKEN_STAGE);
  CTX_data_scene_set(C, scene);
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
  } while ((ptr = ptr->base));

  return NULL;
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

PropertyType LUXO_property_type(KrakenPROP *prop)
{
  SdfValueTypeName type = prop->GetTypeName();

  if (type == SdfValueTypeNames->Bool) {
    return PROP_BOOLEAN;
  }

  if ((type == SdfValueTypeNames->Int) || 
      (type == SdfValueTypeNames->Int2) || 
      (type == SdfValueTypeNames->Int3) ||
      (type == SdfValueTypeNames->Int4)) {
    return PROP_INT;
  }

  if ((type == SdfValueTypeNames->Float) || 
      (type == SdfValueTypeNames->Float2) || 
      (type == SdfValueTypeNames->Float3) ||
      (type == SdfValueTypeNames->Float4) ||
      (type == SdfValueTypeNames->Double) ||
      (type == SdfValueTypeNames->Double2) ||
      (type == SdfValueTypeNames->Double3) ||
      (type == SdfValueTypeNames->Double4)) {
    return PROP_FLOAT;
  }

  if (type == SdfValueTypeNames->String) {
    return PROP_STRING;
  }

  if (type == SdfValueTypeNames->Token) {
    return PROP_ENUM;
  }
}

bool USD_enum_identifier(wabi::TfEnum item, const int value, const char **r_identifier)
{
  //   const int i = LUXO_enum_from_value(item, value);
  //   if (i != -1) {
  //     *r_identifier = item[i].identifier;
  //     return true;
  //   }
  return false;
}

void LUXO_object_find_property(KrakenPRIM *ptr, const TfToken &name, KrakenPROP *r_ptr)
{
  KrakenPRIM prim;

  if (!ptr->IsValid()) {
    *ptr = ptr->GetStage()->GetPseudoRoot();
  }

  prim = ptr->GetPrim();
  *r_ptr = prim.GetProperty(name);

  if (!r_ptr || !r_ptr->IsValid()) {
    std::string msg;
    msg = TfStringPrintf("%s has no property %s. Available properties are:\n",
                         prim.GetName().GetText(),
                         name.GetText());
    for (auto &p : prim.GetPropertyNames()) {
      msg += TfStringPrintf("* %s\n", p.GetText());
    }
    TF_WARN(msg);
  }
}

UsdCollectionsVector LUXO_property_collection_begin(KrakenPRIM *ptr, const TfToken &name)
{
  UsdCollectionAPI collection;
  UsdPrim prim;

  prim = ptr->GetPrim();

  collection = UsdCollectionAPI::Get(prim, name);
  return collection.GetAllCollections(prim);
}

void LUXO_stage_pointer_ensure(KrakenPRIM *r_ptr)
{
  if (!r_ptr || !r_ptr->IsValid()) {
    *r_ptr = KRAKEN_STAGE->GetPseudoRoot();
  }
}

void LUXO_main_pointer_create(struct Main *main, KrakenPRIM *r_ptr)
{
  *r_ptr = KRAKEN_STAGE->GetPseudoRoot().GetPrimAtPath(wabi::SdfPath("/WabiAnimationStudios"));
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
    *r_ptr = KRAKEN_STAGE->GetPseudoRoot().GetPrimAtPath(wabi::SdfPath("/WabiAnimationStudios"));
  }
  r_ptr->owner_id = r_ptr->GetParent().IsValid() ? r_ptr->GetParent().GetName().GetText() : NULL;
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
  return (type->identifier != NULL) ? type->identifier : "Context";
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
      // KrakenPRIM ctx = but->stagepoin;
      // KrakenSTAGE stage = but->stagepoin.GetStage();
      // UsdEditTarget trg = stage->GetEditTarget();
      // SdfPrimSpecHandle ptr = trg.GetPrimSpecForScenePath(ctx.GetPath());
/**
 * Use UsdStage::CreateInMemory when we get out of an alpha state
 * for now, this makes it easier to debug scene description - it
 * is located in ~/Library/Application Support/Kraken/1.50/config/userpref.usda */
KrakenSTAGE::KrakenSTAGE()
  : UsdStageRefPtr(UsdStage::CreateNew(KKE_kraken_globals_init().main->stage_id)),
    structs{&LUXO_Window, &LUXO_WorkSpace, &LUXO_Screen, &LUXO_Area, &LUXO_Region}
{}

void LUXO_pointer_create(ID *id, KrakenPRIM *type, void *data, KrakenPRIM *r_ptr)
{
  const KrakenPRIM *ctx = STAGE_CTX(type);

  r_ptr->owner_id = id;
  r_ptr->type = &KrakenPRIM(ctx->GetPrim());
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

void LUXO_kraken_luxo_pointer_create(KrakenPRIM *r_ptr)
{
  *r_ptr = KRAKEN_STAGE->GetPseudoRoot().GetPrimAtPath(wabi::SdfPath("/WabiAnimationStudios"));
  r_ptr->owner_id = NULL;
  r_ptr->type = &LUXO_KrakenPixar;
  r_ptr->data = (void *&)KRAKEN_STAGE;
}

KRAKEN_NAMESPACE_END