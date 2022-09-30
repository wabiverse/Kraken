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

#include "KLI_kraklib.h"

#include "KKE_utils.h"
#include "KKE_appdir.h"
#include "KKE_idprop.h"

#include "USD_api.h"
#include "USD_types.h"
#include "USD_object.h"
#include "USD_scene.h"

#include "LUXO_runtime.h"
#include "LUXO_access.h"
#include "LUXO_main.h"

#include "LUXO_internal.h"

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
    if (base->GetTypeName() == srna->GetTypeName()) {
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

KrakenPROP *luxo_ensure_property(KrakenPROP *prop)
{
  if (prop) {
    return prop;
  }
}

void LUXO_property_int_range(KrakenPRIM *ptr, KrakenPROP *prop, int *hardmin, int *hardmax)
{
  KrakenPROP *iprop = luxo_ensure_property(&KrakenPROP(ptr->GetAttribute(prop->GetName())));
  *hardmin = INT_MIN;
  *hardmax = INT_MAX;
}

void LUXO_property_float_range(KrakenPRIM *ptr, KrakenPROP *prop, float *hardmin, float *hardmax)
{
  KrakenPROP *iprop = luxo_ensure_property(&KrakenPROP(ptr->GetAttribute(prop->GetName())));
  *hardmin = FLT_MIN;
  *hardmax = FLT_MAX;
}

PropertyScaleType LUXO_property_ui_scale(KrakenPROP *prop)
{
  // KrakenPROP *prim_prop = luxo_ensure_property(prop);
  // SdfValueTypeName s_type = prim_prop->GetTypeName().GetScalarType();

  return PROP_SCALE_LINEAR;
}

PropertyType LUXO_property_type_enum(KrakenPROP *prop)
{
  return luxo_ensure_property(prop)->type;
}

bool LUXO_struct_undo_check(const KrakenPRIM *type)
{
  return (type->flag & STRUCT_UNDO) != 0;
}

// bool RNA_path_resolve_property(const KrakenPRIM *ptr,
//                                const char *path,
//                                KrakenPRIM *r_ptr,
//                                KrakenPROP **r_prop)
// {
//   if (!luxo_path_parse(ptr, path, r_ptr, r_prop, nullptr, nullptr, nullptr, false)) {
//     return false;
//   }

//   return r_ptr->data != nullptr && *r_prop != nullptr;
// }

KrakenPROP *LUXO_struct_iterator_property(KrakenPRIM *type)
{
  return type->props.front();
}

KrakenPROP *LUXO_struct_find_property(KrakenPRIM *ptr, const char *identifier)
{
  if (identifier[0] == '[' && identifier[1] == '"') {
    /* id prop lookup, not so common */
    KrakenPROP *r_prop = NULL;
    KrakenPRIM r_ptr; /* only support single level props */
    // if (RNA_path_resolve_property(ptr, identifier, &r_ptr, &r_prop) && (r_ptr.type == ptr->type)
    // &&
    //     (r_ptr.data == ptr->data)) {
    //   return r_prop;
    // }
  } else {
    /* most common case */
    KrakenPROP *iterprop = LUXO_struct_iterator_property(ptr->type);
    KrakenPRIM propptr;

    // if (LUXO_property_collection_lookup_string(ptr, iterprop, identifier, &propptr)) {
    //   return propptr.data;
    // }
  }

  return NULL;
}

static const TfToken luxo_ensure_property_identifier(const KrakenPROP *prop)
{
  // if (prop->magic == LUXO_MAGIC) {
  //   return prop->identifier;
  // }
  return ((const IDProperty *)prop)->name;
}

const TfToken LUXO_property_identifier(const KrakenPROP *prop)
{
  return luxo_ensure_property_identifier(prop);
}

PropertySubType LUXO_property_subtype(KrakenPROP *prop)
{
  KrakenPROP *stage_prop = luxo_ensure_property(prop);

  /* For custom properties, find and parse the 'subtype' metadata field. */
  IDProperty *idprop = (IDProperty *)prop;

  if (idprop && idprop->ui_data) {
    IDPropertyUIData *ui_data = idprop->ui_data;
    return (PropertySubType)ui_data->rna_subtype;
  }

  return stage_prop->subtype;
}

PropertyType LUXO_property_type(KrakenPROP *prop)
{
  SdfValueTypeName type = prop->GetTypeName();

  if (type == SdfValueTypeNames->Bool) {
    return PROP_BOOLEAN;
  }

  if ((type == SdfValueTypeNames->Int) || (type == SdfValueTypeNames->Int2) ||
      (type == SdfValueTypeNames->Int3) || (type == SdfValueTypeNames->Int4)) {
    return PROP_INT;
  }

  if ((type == SdfValueTypeNames->Float) || (type == SdfValueTypeNames->Float2) ||
      (type == SdfValueTypeNames->Float3) || (type == SdfValueTypeNames->Float4) ||
      (type == SdfValueTypeNames->Double) || (type == SdfValueTypeNames->Double2) ||
      (type == SdfValueTypeNames->Double3) || (type == SdfValueTypeNames->Double4)) {
    return PROP_FLOAT;
  }

  if (type == SdfValueTypeNames->String) {
    return PROP_STRING;
  }

  if (type == SdfValueTypeNames->Token) {
    return PROP_ENUM;
  }

  return PROP_COLLECTION;
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
  *r_ptr = prim.GetAttribute(name);

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

IDProperty **LUXO_struct_idprops_p(KrakenPRIM *ptr)
{
  KrakenPRIM *type = ptr->type;
  if (type == NULL) {
    return NULL;
  }
  if (type->idproperties == NULL) {
    return NULL;
  }

  return type->idproperties(ptr);
}

IDProperty *LUXO_struct_idprops(KrakenPRIM *ptr, bool create)
{
  IDProperty **property_ptr = LUXO_struct_idprops_p(ptr);
  if (property_ptr == NULL) {
    return NULL;
  }

  if (create && *property_ptr == NULL) {
    IDPropertyTemplate val = {0};
    *property_ptr = IDP_New(IDP_GROUP, &val, wabi::TfToken(__func__));
  }

  return *property_ptr;
}

IDProperty *prim_idproperty_find(KrakenPRIM *ptr, const TfToken &name)
{
  IDProperty *group = LUXO_struct_idprops(ptr, 0);

  if (group) {
    if (group->type == IDP_GROUP) {
      return IDP_GetPropertyFromGroup(group, name);
    }
    /* Not sure why that happens sometimes, with nested properties... */
    /* Seems to be actually array prop, name is usually "0"... To be sorted out later. */
#if 0
      printf(
          "Got unexpected IDProp container when trying to retrieve %s: %d\n", name, group->type);
#endif
  }

  return NULL;
}

static KrakenPROP *typemap[IDP_NUMTYPES] = {
    &prim_PropertyGroupItem_string,
    &prim_PropertyGroupItem_int,
    &prim_PropertyGroupItem_float,
    NULL,
    NULL,
    NULL,
    &prim_PropertyGroupItem_group,
    &prim_PropertyGroupItem_id,
    &prim_PropertyGroupItem_double,
    &prim_PropertyGroupItem_idp_array,
};

static KrakenPROP *arraytypemap[IDP_NUMTYPES] = {
    NULL,
    &prim_PropertyGroupItem_int_array,
    &prim_PropertyGroupItem_float_array,
    NULL,
    NULL,
    NULL,
    &prim_PropertyGroupItem_collection,
    NULL,
    &prim_PropertyGroupItem_double_array,
};

void prim_property_prim_or_id_get(KrakenPROP *prop,
                                  KrakenPRIM *ptr,
                                  PropertyPRIMOrID *r_prop_rna_or_id)
{
  /* This is quite a hack, but avoids some complexity in the API. we
   * pass IDProperty structs as PropertyRNA pointers to the outside.
   * We store some bytes in PropertyRNA structs that allows us to
   * distinguish it from IDProperty structs. If it is an ID property,
   * we look up an IDP PropertyRNA based on the type, and set the data
   * pointer to the IDProperty. */
  memset(r_prop_rna_or_id, 0, sizeof(*r_prop_rna_or_id));

  r_prop_rna_or_id->ptr = *ptr;
  r_prop_rna_or_id->rawprop = prop;


  IDProperty *idprop = (IDProperty *)prop;
  /* Given prop may come from the custom properties of another data, ensure we get the one from
   * given data ptr. */
  IDProperty *idprop_evaluated = prim_idproperty_find(ptr, idprop->name);
  if (idprop_evaluated != NULL && idprop->type != idprop_evaluated->type) {
    idprop_evaluated = NULL;
  }

  r_prop_rna_or_id->idprop = idprop_evaluated;
  r_prop_rna_or_id->is_idprop = true;
  /* Full IDProperties are always set, if it exists. */
  r_prop_rna_or_id->is_set = (idprop_evaluated != NULL);

  r_prop_rna_or_id->identifier = idprop->name;
  if (idprop->type == IDP_ARRAY) {
    r_prop_rna_or_id->rnaprop = arraytypemap[(int)(idprop->subtype)];
    r_prop_rna_or_id->is_array = true;
    r_prop_rna_or_id->array_len = idprop_evaluated != NULL ? (uint)idprop_evaluated->len : 0;
  } else {
    r_prop_rna_or_id->rnaprop = typemap[(int)(idprop->type)];
  }
}

IDProperty *prim_idproperty_check(KrakenPROP **prop, KrakenPRIM *ptr)
{
  PropertyPRIMOrID prop_prim_or_id;

  prim_property_prim_or_id_get(*prop, ptr, &prop_prim_or_id);

  *prop = prop_prim_or_id.rnaprop;
  return prop_prim_or_id.idprop;
}

void LUXO_property_collection_begin(KrakenPRIM *ptr,
                                    KrakenPROP *prop,
                                    CollectionPropertyIterator *iter)
{
  IDProperty *idprop;

  KLI_assert(LUXO_property_type(prop) == PROP_COLLECTION);

  memset(iter, 0, sizeof(*iter));

  if ((idprop = prim_idproperty_check(&prop, ptr)) || (prop->flag & PROP_IDPROPERTY)) {
    iter->parent = *ptr;
    iter->prop = prop;

  //   if (idprop) {
  //     prim_iterator_array_begin(iter,
  //                              IDP_IDPArray(idprop),
  //                              sizeof(IDProperty),
  //                              idprop->len,
  //                              0,
  //                              NULL);
  //   } else {
  //     prim_iterator_array_begin(iter, NULL, sizeof(IDProperty), 0, 0, NULL);
  //   }

  //   if (iter->valid) {
  //     prim_property_collection_get_idp(iter);
  //   }

  //   iter->idprop = 1;
  // } else {
  //   CollectionPropertyPRIM *cprop = (CollectionPropertyPRIM *)prop;
  //   cprop->begin(iter, ptr);
  }
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
  r_ptr->owner_id = id;
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

void LUXO_kraken_luxo_pointer_create(KrakenPRIM *r_ptr)
{
  *r_ptr = KRAKEN_STAGE->GetPseudoRoot().GetPrimAtPath(wabi::SdfPath("/WabiAnimationStudios"));
  r_ptr->owner_id = NULL;
  r_ptr->type = &LUXO_KrakenPixar;
  r_ptr->data = (void *&)KRAKEN_STAGE;
}

int LUXO_enum_from_value(const EnumPropertyItem *item, const int value)
{
  int i = 0;
  for (; item->identifier.data(); item++, i++) {
    if ((!item->identifier.IsEmpty()) && item->value == value) {
      return i;
    }
  }
  return -1;
}

void LUXO_property_enum_items_ex(kContext *C,
                                 KrakenPRIM *ptr,
                                 KrakenPROP *prop,
                                 const bool use_static,
                                 const EnumPropertyItem **r_item,
                                 int *r_totitem,
                                 bool *r_free)
{
  EnumPropertyPRIM *eprop = (EnumPropertyPRIM *)luxo_ensure_property(prop);

  *r_free = false;

  if (!use_static && (eprop->item_fn != NULL)) {
    const bool no_context = (prop->flag & PROP_ENUM_NO_CONTEXT) ||
                            ((ptr->type->flag & STRUCT_NO_CONTEXT_WITHOUT_OWNER_ID) &&
                             (ptr->owner_id == NULL));
    if (C != NULL || no_context) {
      const EnumPropertyItem *item;

      item = eprop->item_fn(no_context ? NULL : C, ptr, prop, r_free);

      /* any callbacks returning NULL should be fixed */
      KLI_assert(item != NULL);

      if (r_totitem) {
        int tot;
        for (tot = 0; item[tot].identifier.data(); tot++) {
          /* pass */
        }
        *r_totitem = tot;
      }

      *r_item = item;
      return;
    }
  }

  *r_item = eprop->item;
  if (r_totitem) {
    *r_totitem = eprop->totitem;
  }
}

void LUXO_property_enum_items(kContext *C,
                              KrakenPRIM *ptr,
                              KrakenPROP *prop,
                              const EnumPropertyItem **r_item,
                              int *r_totitem,
                              bool *r_free)
{
  LUXO_property_enum_items_ex(C, ptr, prop, false, r_item, r_totitem, r_free);
}

KRAKEN_NAMESPACE_END