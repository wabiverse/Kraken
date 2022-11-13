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

#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "KLI_compiler_attrs.h"

#include "MEM_guardedalloc.h"

#include "KLI_kraklib.h"
#include "KLI_time.h"
#include "KLI_dynstr.h"
#include "KLI_threads.h"

#include "KKE_utils.h"
#include "KKE_appdir.h"
#include "KKE_appdir.hh"
#include "KKE_idprop.h"
#include "KKE_lib_id.h"
#include "KKE_global.h"

#include "USD_api.h"
#include "USD_types.h"
#include "USD_object.h"
#include "USD_scene.h"
#include "USD_ID.h"

#include "WM_event_system.h"
#include "WM_msgbus.h"

#include "LUXO_define.h"
#include "LUXO_runtime.h"
#include "LUXO_access.h"
#include "LUXO_main.h"

#include "LUXO_internal.h"

#include <wabi/base/tf/stackTrace.h>
#include <wabi/usd/ar/resolver.h>

WABI_NAMESPACE_USING

KrakenPRIM PRIM_StageData;
KrakenPRIM PRIM_KrakenPRIM;
KrakenPRIM PRIM_KrakenPROP;
KrakenPRIM PRIM_Context;
KrakenPRIM PRIM_Struct;
KrakenPRIM PRIM_Window;
KrakenPRIM PRIM_WorkSpace;
KrakenPRIM PRIM_Screen;
KrakenPRIM PRIM_Area;
KrakenPRIM PRIM_Region;

KrakenSTAGE KRAKEN_STAGE = {};

static void LUXO_prim_init(void)
{
  UsdPrim intern = KRAKEN_STAGE->DefinePrim(K_FOUNDATION);

  UsdCollectionAPI capi = UsdCollectionAPI::Apply(intern, TfToken("structs"));
  capi.CreateIncludesRel().AddTarget(intern.GetPath().AppendPath(SdfPath("Structs")));
}

static void *prim_iterator_array_get(CollectionPropIT *iter)
{
  ArrayIT *internal = &iter->internal.array;

  return internal->ptr;
}

static void prim_pointer_inherit_id(KrakenPRIM *type, KrakenPRIM *parent, KrakenPRIM *ptr)
{
  if (type && type->flag & STRUCT_ID) {
    ptr->owner_id = (ID *)ptr->data;
  } else {
    ptr->owner_id = parent->owner_id;
  }
}

void LUXO_init(void)
{
  LUXO_prim_init();
  LUXO_main(KRAKEN_STAGE);

  KKE_tempdir_init(NULL);

  LUXO_save_usd();
}

void LUXO_set_stage_ctx(kContext *C)
{
  kScene *scene = new kScene(KRAKEN_STAGE);
  CTX_data_scene_set(C, scene);
}

PrimRegisterFUNC LUXO_prim_register(const KrakenPRIM *ptr)
{
  return ptr->reg;
}

PrimUnregisterFUNC LUXO_prim_unregister(KrakenPRIM *ptr)
{
  do {
    if (ptr->unreg) {
      return ptr->unreg;
    }
  } while ((ptr = ptr->base));

  return NULL;
}

bool LUXO_prim_is_a(const KrakenPRIM *type, const KrakenPRIM *sprim)
{
  const KrakenPRIM *base;

  if (!type) {
    return false;
  }

  for (base = type; base; base = base->base) {
    if (base->GetTypeName() == sprim->GetTypeName()) {
      return true;
    }
  }

  return false;
}

void **LUXO_prim_instance(KrakenPRIM *ptr)
{
  KrakenPRIM *type = ptr->type;

  do {
    if (type->instance) {
      return type->instance(ptr);
    }
  } while ((type = type->base));

  return NULL;
}

KrakenPROP *prim_ensure_property(KrakenPROP *prop)
{
  if (prop) {
    return prop;
  }
}

void LUXO_prop_int_range(KrakenPRIM *ptr, KrakenPROP *prop, int *hardmin, int *hardmax)
{
  KrakenPROP *iprop = prim_ensure_property(prop);
  *hardmin = INT_MIN;
  *hardmax = INT_MAX;
}

void LUXO_prop_float_range(KrakenPRIM *ptr, KrakenPROP *prop, float *hardmin, float *hardmax)
{
  KrakenPROP *iprop = prim_ensure_property(prop);
  *hardmin = FLT_MIN;
  *hardmax = FLT_MAX;
}

PropScaleTYPE LUXO_prop_ui_scale(KrakenPROP *prop)
{
  // KrakenPROP *prim_prop = prim_ensure_property(prop);
  // SdfValueTypeName s_type = prim_prop->GetTypeName().GetScalarType();

  return PROP_SCALE_LINEAR;
}

PropertyType LUXO_prop_type_enum(KrakenPROP *prop)
{
  return prim_ensure_property(prop)->type;
}

bool LUXO_prim_undo_check(const KrakenPRIM *type)
{
  return (type->flag & STRUCT_UNDO) != 0;
}

// bool LUXO_path_resolve_property(const KrakenPRIM *ptr,
//                                const char *path,
//                                KrakenPRIM *r_ptr,
//                                KrakenPROP **r_prop)
// {
//   if (!luxo_path_parse(ptr, path, r_ptr, r_prop, nullptr, nullptr, nullptr, false)) {
//     return false;
//   }

//   return r_ptr->data != nullptr && *r_prop != nullptr;
// }

KrakenPROP *LUXO_prim_iterator_property(KrakenPRIM *type)
{
  return type->props.front();
}

KrakenPROP *LUXO_prim_find_property(KrakenPRIM *ptr, const char *identifier)
{
  if (identifier[0] == '[' && identifier[1] == '"') {
    /* id prop lookup, not so common */
    KrakenPROP *r_prop = NULL;
    KrakenPRIM r_ptr; /* only support single level props */
    // if (LUXO_path_resolve_property(ptr, identifier, &r_ptr, &r_prop) && (r_ptr.type ==
    // ptr->type)
    // &&
    //     (r_ptr.data == ptr->data)) {
    //   return r_prop;
    // }
  } else {
    /* most common case */
    KrakenPROP *iterprop = LUXO_prim_iterator_property(ptr->type);
    KrakenPRIM propptr;

    // if (LUXO_prop_collection_lookup_string(ptr, iterprop, identifier, &propptr)) {
    //   return propptr.data;
    // }
  }

  return NULL;
}

static const TfToken prim_ensure_property_identifier(const KrakenPROP *prop)
{
  // if (prop->magic == LUXO_MAGIC) {
  //   return prop->identifier;
  // }
  return TfToken(((const IDProperty *)prop)->name);
}

const TfToken LUXO_prop_identifier(const KrakenPROP *prop)
{
  return prim_ensure_property_identifier(prop);
}

PropertySubType LUXO_prop_subtype(KrakenPROP *prop)
{
  KrakenPROP *stage_prop = prim_ensure_property(prop);

  /* For custom properties, find and parse the 'subtype' metadata field. */
  IDProperty *idprop = (IDProperty *)prop;

  if (idprop && idprop->ui_data) {
    IDPropertyUIData *ui_data = idprop->ui_data;
    return (PropertySubType)ui_data->prim_subtype;
  }

  return stage_prop->subtype;
}

PropertyType LUXO_prop_type(KrakenPROP *prop)
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

void LUXO_object_find_property(KrakenPRIM *ptr, const TfToken &name, KrakenPROP *r_ptr)
{
  KrakenPRIM prim;

  if (!ptr->IsValid()) {
    *ptr = ptr->GetStage()->GetPseudoRoot();
  }

  prim = ptr->GetPrim();
  r_ptr = MEM_new<KrakenPROP>(__func__, prim.GetProperty(name));

  if (!r_ptr || !r_ptr->intern_prop.IsValid()) {
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

static void prim_iterator_array_next(CollectionPropIT *iter)
{
  ArrayIT *internal = &iter->internal.array;

  if (internal->skip) {
    do {
      internal->ptr += internal->itemsize;
      iter->valid = (internal->ptr != internal->endptr);
    } while (iter->valid && internal->skip(iter, internal->ptr));
  } else {
    internal->ptr += internal->itemsize;
    iter->valid = (internal->ptr != internal->endptr);
  }
}

static void prim_property_collection_get_idp(CollectionPropIT *iter)
{
  CollectionPropPRIM *cprop = (CollectionPropPRIM *)iter->prop;

  iter->ptr.data = prim_iterator_array_get(iter);
  iter->ptr.type = cprop->item_type;
  prim_pointer_inherit_id(cprop->item_type, &iter->parent, &iter->ptr);
}

static void prim_iterator_array_end(CollectionPropIT *iter)
{
  ArrayIT *internal = &iter->internal.array;

  MEM_SAFE_FREE(internal->free_ptr);
}

void LUXO_prop_collection_next(CollectionPropIT *iter)
{
  CollectionPropPRIM *cprop = (CollectionPropPRIM *)prim_ensure_property(iter->prop);

  if (iter->idprop) {
    prim_iterator_array_next(iter);

    if (iter->valid) {
      prim_property_collection_get_idp(iter);
    }
  } else {
    cprop->next(iter);
  }
}

void LUXO_prop_collection_end(CollectionPropIT *iter)
{
  CollectionPropPRIM *cprop = (CollectionPropPRIM *)prim_ensure_property(iter->prop);

  if (iter->idprop) {
    prim_iterator_array_end(iter);
  } else {
    cprop->end(iter);
  }
}

void LUXO_collection_begin(KrakenPRIM *ptr, const char *name, CollectionPropIT *iter)
{
  KrakenPROP *prop = LUXO_prim_find_property(ptr, name);

  if (prop) {
    LUXO_prop_collection_begin(ptr, prop, iter);
  } else {
    printf("%s: %s.%s not found.\n", __func__, ptr->type->identifier.data(), name);
  }
}

static void prim_iterator_array_begin(CollectionPropIT *iter,
                                      void *ptr,
                                      int itemsize,
                                      int length,
                                      bool free_ptr,
                                      IteratorSkipFUNC skip)
{
  ArrayIT *internal;

  if (ptr == NULL) {
    length = 0;
  } else if (length == 0) {
    ptr = NULL;
    itemsize = 0;
  }

  internal = &iter->internal.array;
  internal->ptr = (char *)ptr;
  internal->free_ptr = free_ptr ? ptr : nullptr;
  internal->endptr = ((char *)ptr) + length * itemsize;
  internal->itemsize = itemsize;
  internal->skip = skip;
  internal->length = length;

  iter->valid = (internal->ptr != internal->endptr);

  if (skip && iter->valid && skip(iter, internal->ptr)) {
    prim_iterator_array_next(iter);
  }
}

void LUXO_prop_collection_begin(KrakenPRIM *ptr, KrakenPROP *prop, CollectionPropIT *iter)
{
  IDProperty *idprop;

  KLI_assert(LUXO_prop_type(prop) == PROP_COLLECTION);

  memset(iter, 0, sizeof(*iter));

  if ((idprop = prim_idproperty_check(&prop, ptr)) || (prop->flag & PROP_IDPROPERTY)) {
    iter->parent = *ptr;
    iter->prop = prop;

    if (idprop) {
      prim_iterator_array_begin(iter,
                                IDP_IDPArray(idprop),
                                sizeof(IDProperty),
                                idprop->len,
                                0,
                                NULL);
    } else {
      prim_iterator_array_begin(iter, NULL, sizeof(IDProperty), 0, 0, NULL);
    }

    if (iter->valid) {
      prim_property_collection_get_idp(iter);
    }

    iter->idprop = 1;
  } else {
    CollectionPropPRIM *cprop = (CollectionPropPRIM *)prop;
    cprop->begin(iter, ptr);
  }
}

/* ID Properties */

void prim_idproperty_touch(IDProperty *idprop)
{
  /* so the property is seen as 'set'. */
  idprop->flag &= ~IDP_FLAG_ANCHOR;
}

IDProperty **LUXO_prim_idprops_p(KrakenPRIM *ptr)
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

IDProperty *LUXO_prim_idprops(KrakenPRIM *ptr, bool create)
{
  IDProperty **property_ptr = LUXO_prim_idprops_p(ptr);
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
  IDProperty *group = LUXO_prim_idprops(ptr, 0);

  if (group) {
    if (group->type == IDP_GROUP) {
      return IDP_GetPropertyFromGroup(group, name);
    }
    /* Not sure why that happens sometimes, with nested properties... */
    /* Seems to be actually array prop, name is usually "0"... To be sorted out later. */
#if 0
      printf(
          "Got unexpected IDProp container when trying to retrieve %s: %d\n", name.data(), group->type);
#endif
  }

  return NULL;
}

bool LUXO_prim_is_ID(const KrakenPRIM *type)
{
  return (type->flag & STRUCT_ID) != 0;
}

bool LUXO_prop_array_check(KrakenPROP *prop)
{
  if (prop->GetTypeName().IsArray()) {
    return (prop->GetTypeName().GetDimensions().size > 0);
  }
  IDProperty *idprop = (IDProperty *)prop;

  return (idprop->type == IDP_ARRAY);
}

bool LUXO_prim_idprops_register_check(const KrakenPRIM *type)
{
  return (type->flag & STRUCT_NO_IDPROPERTIES) == 0;
}

bool LUXO_prim_idprops_datablock_allowed(const KrakenPRIM *type)
{
  return (type->flag & (STRUCT_NO_DATABLOCK_IDPROPERTIES | STRUCT_NO_IDPROPERTIES)) == 0;
}

bool LUXO_prim_idprops_contains_datablock(const KrakenPRIM *type)
{
  return (type->flag & (STRUCT_CONTAINS_DATABLOCK_IDPROPERTIES | STRUCT_ID)) != 0;
}

// static KrakenPROP *typemap[IDP_NUMTYPES] = {
//     &prim_PropertyGroupItem_string,
//     &prim_PropertyGroupItem_int,
//     &prim_PropertyGroupItem_float,
//     NULL,
//     NULL,
//     NULL,
//     &prim_PropertyGroupItem_group,
//     &prim_PropertyGroupItem_id,
//     &prim_PropertyGroupItem_double,
//     &prim_PropertyGroupItem_idp_array,
// };

// static KrakenPROP *arraytypemap[IDP_NUMTYPES] = {
//     NULL,
//     &prim_PropertyGroupItem_int_array,
//     &prim_PropertyGroupItem_float_array,
//     NULL,
//     NULL,
//     NULL,
//     &prim_PropertyGroupItem_collection,
//     NULL,
//     &prim_PropertyGroupItem_double_array,
// };

void prim_property_prim_or_id_get(KrakenPROP *prop,
                                  KrakenPRIM *ptr,
                                  PropertyPRIMOrID *r_prop_prim_or_id)
{
  /* This is quite a hack, but avoids some complexity in the API. we
   * pass IDProperty structs as KrakenPROP pointers to the outside.
   * We store some bytes in KrakenPROP structs that allows us to
   * distinguish it from IDProperty structs. If it is an ID property,
   * we look up an IDP KrakenPROP based on the type, and set the data
   * pointer to the IDProperty. */
  memset(r_prop_prim_or_id, 0, sizeof(*r_prop_prim_or_id));

  r_prop_prim_or_id->ptr = *ptr;
  r_prop_prim_or_id->rawprop = prop;


  IDProperty *idprop = (IDProperty *)prop;
  /* Given prop may come from the custom properties of another data, ensure we get the one from
   * given data ptr. */
  IDProperty *idprop_evaluated = prim_idproperty_find(ptr, TfToken(idprop->name));
  if (idprop_evaluated != NULL && idprop->type != idprop_evaluated->type) {
    idprop_evaluated = NULL;
  }

  r_prop_prim_or_id->idprop = idprop_evaluated;
  r_prop_prim_or_id->is_idprop = true;
  /* Full IDProperties are always set, if it exists. */
  r_prop_prim_or_id->is_set = (idprop_evaluated != NULL);

  r_prop_prim_or_id->identifier = TfToken(idprop->name);
  if (idprop->type == IDP_ARRAY) {
    // r_prop_prim_or_id->primprop = arraytypemap[(int)(idprop->subtype)];
    r_prop_prim_or_id->is_array = true;
    r_prop_prim_or_id->array_len = idprop_evaluated != NULL ? (uint)idprop_evaluated->len : 0;
  } else {
    // r_prop_prim_or_id->primprop = typemap[(int)(idprop->type)];
  }
}

IDProperty *prim_idproperty_check(KrakenPROP **prop, KrakenPRIM *ptr)
{
  PropertyPRIMOrID prop_prim_or_id;

  prim_property_prim_or_id_get(*prop, ptr, &prop_prim_or_id);

  *prop = prop_prim_or_id.primprop;
  return prop_prim_or_id.idprop;
}

void LUXO_stage_pointer_ensure(KrakenPRIM *r_ptr)
{
  if (!r_ptr || !r_ptr->IsValid()) {
    *r_ptr = KRAKEN_STAGE->GetPseudoRoot();
  }
}

void LUXO_main_pointer_create(struct Main *main, KrakenPRIM *r_ptr)
{
  *r_ptr = KRAKEN_STAGE->GetPseudoRoot().GetPrimAtPath(K_FOUNDATION);
  r_ptr->owner_id = NULL;
  r_ptr->type = &PRIM_StageData;
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

int LUXO_prop_string_length(KrakenPRIM *ptr, KrakenPROP *prop)
{
  KrakenPROPString *sprop = (KrakenPROPString *)prop;

  KLI_assert(LUXO_prop_type_enum(prop) == PROP_STRING);

  if (sprop->length) {
    return sprop->length(ptr);
  }
  if (sprop->length_ex) {
    return sprop->length_ex(ptr, prop);
  }
  return strlen(sprop->defaultvalue);
}

void *LUXO_prim_py_type_get(KrakenPRIM *sprim)
{
  return sprim->py_type;
}

void LUXO_prim_py_type_set(KrakenPRIM *sprim, void *type)
{
  sprim->py_type = type;
}

void *LUXO_prop_py_data_get(KrakenPROP *prop)
{
  return prop->py_data;
}

std::vector<KrakenPRIM *> &LUXO_prim_type_functions(KrakenPRIM *sprim)
{
  return sprim->functions;
}

KrakenPRIM *LUXO_prim_base(KrakenPRIM *type)
{
  return type->base;
}

int LUXO_prim_ui_icon(const KrakenPRIM *type)
{
  if (type) {
    return type->icon;
  }
  return ICON_DOT;
}

const char *LUXO_function_identifier(KrakenFUNC *func)
{
  return func->identifier;
}

int LUXO_function_flag(KrakenFUNC *func)
{
  return func->flag;
}

int LUXO_function_defined(KrakenFUNC *func)
{
  return func->call != NULL;
}

const TfToken LUXO_prim_identifier(const KrakenPRIM *type)
{
  return (type->identifier != TfToken()) ? type->identifier : TfToken("Context");
}

const TfToken LUXO_prop_identifier(const KrakenPROP *prop)
{
  return prop->GetName();
}

static void prim_property_update(kContext *C,
                                 Main *kmain,
                                 Scene *scene,
                                 KrakenPRIM *ptr,
                                 KrakenPROP *prop)
{
  const bool is_root = (prop->GetPrim().GetParent() == UsdPrim());
  prop = prim_ensure_property(prop);

  if (is_root) {
    if (prop->update) {
      /* ideally no context would be needed for update, but there's some
       * parts of the code that need it still, so we have this exception */
      if (prop->flag & PROP_CONTEXT_UPDATE) {
        if (C) {
          if ((prop->flag & PROP_CONTEXT_PROPERTY_UPDATE) == PROP_CONTEXT_PROPERTY_UPDATE) {
            ((ContextPropUpdateFUNC)prop->update)(C, ptr, prop);
          } else {
            ((ContextUpdateFUNC)prop->update)(C, ptr);
          }
        }
      } else {
        prop->update(kmain, scene, ptr);
      }
    }

#if 1
    /* TODO(@campbellbarton): Should eventually be replaced entirely by message bus (below)
     * for now keep since COW, bugs are hard to track when we have other missing updates. */
    if (prop->noteflag) {
      WM_main_add_notifier(prop->noteflag, ptr->owner_id);
    }
#endif

    /* if C is NULL, we're updating from animation.
     * avoid slow-down from f-curves by not publishing (for now). */
    if (C != NULL) {
      struct wmMsgBus *mbus = CTX_wm_message_bus(C);
      /* we could add NULL check, for now don't */
      WM_msg_publish_prim(mbus, ptr, prop);
    }
    // if (ptr->owner_id != NULL && ((prop->flag & PROP_NO_DEG_UPDATE) == 0)) {
    //   const short id_type = GS(ptr->owner_id->name);
    //   if (ID_TYPE_IS_COW(id_type)) {
    //     DEG_id_tag_update(ptr->owner_id, ID_RECALC_COPY_ON_WRITE);
    //   }
    // }
    /* End message bus. */
  }

  if (!is_root || (prop->flag & PROP_IDPROPERTY)) {

    /* Disclaimer: this logic is not applied consistently, causing some confusing behavior.
     *
     * - When animated (which skips update functions).
     * - When ID-properties are edited via Python (since RNA properties aren't used in this case).
     *
     * Adding updates will add a lot of overhead in the case of animation.
     * For Python it may cause unexpected slow-downs for developers using ID-properties
     * for data storage. Further, the root ID isn't available with nested data-structures.
     *
     * So editing custom properties only causes updates in the UI,
     * keep this exception because it happens to be useful for driving settings.
     * Python developers on the other hand will need to manually 'update_tag', see: T74000. */
    // DEG_id_tag_update(ptr->owner_id,
    //                   ID_RECALC_TRANSFORM | ID_RECALC_GEOMETRY | ID_RECALC_PARAMETERS);

    // /* When updating an ID pointer property, tag depsgraph for update. */
    // if (prop->type == PROP_POINTER && LUXO_struct_is_ID(LUXO_prop_pointer_type(ptr, prop)))
    // {
    //   DEG_relations_tag_update(kmain);
    // }

    WM_main_add_notifier(NC_WINDOW, NULL);
    /* Not nice as well, but the only way to make sure material preview
     * is updated with custom nodes.
     */
    if ((prop->flag & PROP_IDPROPERTY) != 0 && (ptr->owner_id != NULL) &&
        (GS(ptr->owner_id->name) == ID_NT)) {
      WM_main_add_notifier(NC_MATERIAL | ND_SHADING, NULL);
    }
  }
}

bool LUXO_prop_update_check(KrakenPROP *prop)
{
  /* NOTE: must keep in sync with #prim_property_update. */
  return (prop->GetPrim().GetParent() != UsdPrim() || prop->update || prop->noteflag);
}

void LUXO_prop_update(kContext *C, KrakenPRIM *ptr, KrakenPROP *prop)
{
  prim_property_update(C, CTX_data_main(C), CTX_data_scene(C), ptr, prop);
}

static int prim_ensure_property_array_length(KrakenPRIM *ptr, KrakenPROP *prop)
{
  if (prop->GetPrim().GetParent() == UsdPrim()) {
    int arraylen[LUXO_MAX_ARRAY_DIMENSION];
    return (prop->GetTypeName().GetDimensions().size && ptr->data) ?
             prop->GetTypeName().GetArrayType().GetDimensions().size :
             (int)prop->GetPrim().GetAttributes().size();
  }
  IDProperty *idprop = (IDProperty *)prop;

  if (idprop->type == IDP_ARRAY) {
    return idprop->len;
  }
  return 0;
}

int LUXO_prop_array_length(KrakenPRIM *ptr, KrakenPROP *prop)
{
  return prim_ensure_property_array_length(ptr, prop);
}


char *LUXO_pointer_as_string_keywords_ex(kContext *C,
                                         KrakenPRIM *ptr,
                                         const bool as_function,
                                         const bool all_args,
                                         const bool nested_args,
                                         const int max_prop_length,
                                         KrakenPROP *iterprop)
{
  const char *arg_name = NULL;

  DynStr *dynstr = KLI_dynstr_new();
  char *cstring, *buf;
  bool first_iter = true;
  int flag, flag_parameter;

  for (auto &prop : ptr->GetAttributes()) {
    KrakenPROP kprop = prop;
    flag = (&kprop)->flag;
    // flag_parameter =

    // if (as_function && (flag_parameter & PARM_OUTPUT)) {
    //   continue;
    // }

    arg_name = LUXO_prop_identifier(&kprop).GetText();

    if (STREQ(arg_name, "prim_type")) {
      continue;
    }

    if ((nested_args == false) && (LUXO_prop_type(&kprop) == PROP_POINTER)) {
      continue;
    }

    // if (as_function && ((&kprop)->flag_parameter & PARM_REQUIRED)) {
    /* required args don't have useful defaults */
    // KLI_dynstr_appendf(dynstr, first_iter ? "%s" : ", %s", arg_name);
    // first_iter = false;
    // } else {
    bool ok = true;

    if (all_args == true) {
      /* pass */
    } else if (ptr->type && !ptr->type->GetAttributes().empty()) {
      ok = ptr->GetAttribute(prop.GetName()).IsValid();
    }

    if (ok) {
      if (as_function && LUXO_prop_type(&kprop) == PROP_POINTER) {
        /* don't expand pointers for functions */
        if (flag & PROP_NEVER_NULL) {
          /* we can't really do the right thing here. arg=arg?, hrmf! */
          buf = KLI_strdup(arg_name);
        } else {
          buf = KLI_strdup("None");
        }
      } else {
        buf = KLI_strdup(ptr->GetAttribute(prop.GetName()).GetName().GetText());
      }

      KLI_dynstr_appendf(dynstr, first_iter ? "%s=%s" : ", %s=%s", arg_name, buf);
      first_iter = false;
      MEM_freeN(buf);
    }
    // }
  }

  cstring = KLI_dynstr_get_cstring(dynstr);
  KLI_dynstr_free(dynstr);
  return cstring;
}

char *LUXO_pointer_as_string_keywords(kContext *C,
                                      KrakenPRIM *ptr,
                                      const bool as_function,
                                      const bool all_args,
                                      const bool nested_args,
                                      const int max_prop_length)
{
  KrakenPROP *iterprop;

  *iterprop = ptr->GetAttributes().front();

  return LUXO_pointer_as_string_keywords_ex(C,
                                            ptr,
                                            as_function,
                                            all_args,
                                            nested_args,
                                            max_prop_length,
                                            iterprop);
}

char *LUXO_pointer_as_string_id(kContext *C, KrakenPRIM *ptr)
{
  DynStr *dynstr = KLI_dynstr_new();
  char *cstring;

  const char *propname;
  int first_time = 1;

  KLI_dynstr_append(dynstr, "{");

  for (auto &prop : ptr->GetAttributes()) {
    KrakenPROP kprop = prop;
    propname = LUXO_prop_identifier(&kprop).GetText();

    if (STREQ(propname, "prim_type")) {
      continue;
    }

    if (first_time == 0) {
      KLI_dynstr_append(dynstr, ", ");
    }
    first_time = 0;

    cstring = KLI_strdup(prop.GetName().GetText());
    KLI_dynstr_appendf(dynstr, "\"%s\":%s", propname, cstring);
    MEM_freeN(cstring);
  }

  KLI_dynstr_append(dynstr, "}");

  cstring = KLI_dynstr_get_cstring(dynstr);
  KLI_dynstr_free(dynstr);
  return cstring;
}

/**
 * Use for sub-typing so we know which ptr is used for a #KrakenPRIM.
 */
KrakenPRIM *sprim_from_ptr(KrakenPRIM *ptr)
{
  if (ptr->type == &PRIM_Struct) {
    return (KrakenPRIM *)ptr->data;
  }

  return ptr->type;
}

void LUXO_save_usd(void)
{
  const char *uprefdir;
  char upreffile[FILE_MAX];
  char render_oclock[80];

  uprefdir = KKE_appdir_folder_id(KRAKEN_USER_CONFIG, NULL);

  if (uprefdir) {
    KLI_join_dirfile(upreffile, sizeof(upreffile), uprefdir, KRAKEN_USERPREF_FILE);
  } else {
    return;
  }

  KLI_pretty_time(std::time(0), render_oclock);
  KRAKEN_STAGE->GetRootLayer()->Export(upreffile,
                                       TfStringPrintf("File last modified: %s", render_oclock));
}

KrakenSTAGE::KrakenSTAGE()
  : UsdStageRefPtr(UsdStage::CreateInMemory()),
    structs{&PRIM_Window, &PRIM_WorkSpace, &PRIM_Screen, &PRIM_Area, &PRIM_Region}
{}

void LUXO_id_pointer_create(ID *id, KrakenPRIM *r_ptr)
{
  KrakenPRIM *type, *idtype = NULL;

  if (id) {
    KrakenPRIM tmp = {NULL};
    tmp.data = id;
    idtype = prim_ID_refine(&tmp);

    while (idtype->refine) {
      type = idtype->refine(&tmp);

      if (type == idtype) {
        break;
      }
      idtype = type;
    }
  }

  r_ptr->owner_id = id;
  r_ptr->type = idtype;
  r_ptr->data = id;
}

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
  *r_ptr = KRAKEN_STAGE->GetPseudoRoot().GetPrimAtPath(K_FOUNDATION);
  r_ptr->owner_id = NULL;
  r_ptr->type = &PRIM_KrakenPRIM;
  r_ptr->data = (void *&)KRAKEN_STAGE;
}

int LUXO_prop_enum_get(KrakenPRIM *ptr, KrakenPROP *prop)
{
  EnumPropPRIM *eprop = (EnumPropPRIM *)prop;
  IDProperty *idprop;

  KLI_assert(LUXO_prop_type(prop) == PROP_ENUM);

  if ((idprop = prim_idproperty_check(&prop, ptr))) {
    return IDP_Int(idprop);
  }
  if (eprop->get) {
    return eprop->get(ptr);
  }
  if (eprop->get_ex) {
    return eprop->get_ex(ptr, prop);
  }
  return eprop->defaultvalue;
}

void LUXO_prop_enum_set(KrakenPRIM *ptr, KrakenPROP *prop, int value)
{
  EnumPropPRIM *eprop = (EnumPropPRIM *)prop;
  IDProperty *idprop;

  KLI_assert(LUXO_prop_type(prop) == PROP_ENUM);

  if ((idprop = prim_idproperty_check(&prop, ptr))) {
    IDP_Int(idprop) = value;
    prim_idproperty_touch(idprop);
  } else if (eprop->set) {
    eprop->set(ptr, value);
  } else if (eprop->set_ex) {
    eprop->set_ex(ptr, prop, value);
  } else if (prop->flag & PROP_EDITABLE) {
    IDPropertyTemplate val = {0};
    IDProperty *group;

    val.i = value;

    group = LUXO_prim_idprops(ptr, 1);
    if (group) {
      IDP_AddToGroup(group, IDP_New(IDP_INT, &val, prop->GetName()));
    }
  }
}

bool LUXO_prop_enum_value(kContext *C,
                          KrakenPRIM *ptr,
                          KrakenPROP *prop,
                          const char *identifier,
                          int *r_value)
{
  const EnumPROP *item;
  bool free;
  bool found;

  LUXO_prop_enum_items(C, ptr, prop, &item, NULL, &free);

  if (item) {
    const int i = LUXO_enum_from_identifier(item, identifier);
    if (i != -1) {
      *r_value = item[i].value;
      found = true;
    } else {
      found = false;
    }

    if (free) {
      MEM_freeN((void *)item);
    }
  } else {
    found = false;
  }
  return found;
}

int LUXO_enum_from_value(const EnumPROP *item, const int value)
{
  int i = 0;
  for (; item->identifier.data(); item++, i++) {
    if ((!item->identifier.IsEmpty()) && item->value == value) {
      return i;
    }
  }
  return -1;
}

void LUXO_prop_enum_items_ex(kContext *C,
                             KrakenPRIM *ptr,
                             KrakenPROP *prop,
                             const bool use_static,
                             const EnumPROP **r_item,
                             int *r_totitem,
                             bool *r_free)
{
  EnumPropPRIM *eprop = (EnumPropPRIM *)prim_ensure_property(prop);

  *r_free = false;

  if (!use_static && (eprop->item_fn != NULL)) {
    const bool no_context = (prop->flag & PROP_ENUM_NO_CONTEXT) ||
                            ((ptr->type->flag & STRUCT_NO_CONTEXT_WITHOUT_OWNER_ID) &&
                             (ptr->owner_id == NULL));
    if (C != NULL || no_context) {
      const EnumPROP *item;

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

void LUXO_prop_enum_items(kContext *C,
                          KrakenPRIM *ptr,
                          KrakenPROP *prop,
                          const EnumPROP **r_item,
                          int *r_totitem,
                          bool *r_free)
{
  LUXO_prop_enum_items_ex(C, ptr, prop, false, r_item, r_totitem, r_free);
}

int LUXO_enum_get(KrakenPRIM *ptr, const char *name)
{
  KrakenPROP *prop = LUXO_prim_find_property(ptr, name);

  if (prop) {
    return LUXO_prop_enum_get(ptr, prop);
  }
  printf("%s: %s.%s not found.\n", __func__, ptr->type->identifier.data(), name);
  return 0;
}

void LUXO_enum_set(KrakenPRIM *ptr, const char *name, int value)
{
  KrakenPROP *prop = LUXO_prim_find_property(ptr, name);

  if (prop) {
    LUXO_prop_enum_set(ptr, prop, value);
  } else {
    printf("%s: %s.%s not found.\n", __func__, ptr->type->identifier.data(), name);
  }
}

int LUXO_enum_from_identifier(const EnumPROP *item, const char *identifier)
{
  int i = 0;
  for (; item->identifier.data(); item++, i++) {
    if (item->identifier.data()[0] && STREQ(item->identifier.data(), identifier)) {
      return i;
    }
  }
  return -1;
}

void LUXO_enum_set_identifier(kContext *C, KrakenPRIM *ptr, const char *name, const char *id)
{
  KrakenPROP *prop = LUXO_prim_find_property(ptr, name);

  if (prop) {
    int value;
    if (LUXO_prop_enum_value(C, ptr, prop, id, &value)) {
      LUXO_prop_enum_set(ptr, prop, value);
    } else {
      printf("%s: %s.%s has no enum id '%s'.\n", __func__, ptr->type->identifier.data(), name, id);
    }
  } else {
    printf("%s: %s.%s not found.\n", __func__, ptr->type->identifier.data(), name);
  }
}

bool LUXO_enum_is_equal(kContext *C, KrakenPRIM *ptr, const char *name, const char *enumname)
{
  KrakenPROP *prop = LUXO_prim_find_property(ptr, name);
  const EnumPROP *item;
  bool free;

  if (prop) {
    int i;
    bool cmp = false;

    LUXO_prop_enum_items(C, ptr, prop, &item, NULL, &free);
    i = LUXO_enum_from_identifier(item, enumname);
    if (i != -1) {
      cmp = (item[i].value == LUXO_prop_enum_get(ptr, prop));
    }

    if (free) {
      MEM_freeN((void *)item);
    }

    if (i != -1) {
      return cmp;
    }

    printf("%s: %s.%s item %s not found.\n",
           __func__,
           ptr->type->identifier.data(),
           name,
           enumname);
    return false;
  }
  printf("%s: %s.%s not found.\n", __func__, ptr->type->identifier.data(), name);
  return false;
}

bool LUXO_enum_value_from_id(const EnumPROP *item, const char *identifier, int *r_value)
{
  const int i = LUXO_enum_from_identifier(item, identifier);
  if (i != -1) {
    *r_value = item[i].value;
    return true;
  }
  return false;
}

bool LUXO_enum_id_from_value(const EnumPROP *item, int value, const char **r_identifier)
{
  const int i = LUXO_enum_from_value(item, value);
  if (i != -1) {
    *r_identifier = item[i].identifier.data();
    return true;
  }
  return false;
}

bool LUXO_enum_icon_from_value(const EnumPROP *item, int value, int *r_icon)
{
  const int i = LUXO_enum_from_value(item, value);
  if (i != -1) {
    *r_icon = item[i].icon;
    return true;
  }
  return false;
}

bool LUXO_enum_name_from_value(const EnumPROP *item, int value, const char **r_name)
{
  const int i = LUXO_enum_from_value(item, value);
  if (i != -1) {
    *r_name = item[i].name;
    return true;
  }
  return false;
}

KrakenPROP *LUXO_prim_iterator_property(KrakenPRIM *type)
{
  KrakenPROP it = type->GetAttributes().front();
  return &it;
}

KrakenPRIM *LUXO_prop_pointer_type(KrakenPRIM *ptr, KrakenPROP *prop)
{
  prop = prim_ensure_property(prop);

  if (prop->type == PROP_POINTER) {
    KrakenPrimPROP *pprop = (KrakenPrimPROP *)prop;

    if (pprop->type_fn) {
      return pprop->type_fn(ptr);
    }
    if (pprop->type) {
      return pprop->type;
    }
  } else if (prop->type == PROP_COLLECTION) {
    CollectionPropPRIM *cprop = (CollectionPropPRIM *)prop;

    if (cprop->item_type) {
      return cprop->item_type;
    }
  }
  /* ignore other types, prim_struct_find_nested calls with unchecked props */

  return MEM_new<KrakenPRIM>(__func__, KrakenPRIM());
}

/* Find the property which uses the given nested struct */
KrakenPROP *prim_struct_find_nested(KrakenPRIM *ptr, KrakenPRIM *sprim)
{
  KrakenPROP *prop = NULL;

  LUXO_PRIM_BEGIN(ptr, iprop)
  {
    /* This assumes that there can only be one user of this nested struct */
    if (LUXO_prop_pointer_type(ptr, iprop) == sprim) {
      prop = iprop;
      break;
    }
  }
  LUXO_PROP_END;

  return prop;
}

ID *LUXO_find_real_ID_and_path(ID *id, const char **r_path)
{
  if (r_path) {
    *r_path = "";
  }

  if ((id == nullptr) || (id->flag & LIB_EMBEDDED_DATA) == 0) {
    return id;
  }

  if (r_path) {
    switch (GS(id->name)) {
      case ID_NT:
        *r_path = "node_tree";
        break;
      case ID_GR:
        *r_path = "collection";
        break;
      default:
        KLI_assert_msg(0, "Missing handling of embedded id type.");
    }
  }

  ID *owner_id = KKE_id_owner_get(id);
  KLI_assert_msg(owner_id != nullptr, "Missing handling of embedded id type.");
  return (owner_id != nullptr) ? owner_id : id;
}

static char *prim_prepend_real_ID_path(Main * /*kmain*/, ID *id, char *path, ID **r_real_id)
{
  if (r_real_id != nullptr) {
    *r_real_id = nullptr;
  }

  const char *prefix;
  ID *real_id = LUXO_find_real_ID_and_path(id, &prefix);

  if (r_real_id != nullptr) {
    *r_real_id = real_id;
  }

  if (path != nullptr) {
    char *new_path = nullptr;

    if (real_id) {
      if (prefix[0]) {
        new_path = KLI_sprintfN("%s%s%s", prefix, path[0] == '[' ? "" : ".", path);
      } else {
        return path;
      }
    }

    MEM_freeN(path);
    return new_path;
  }
  return prefix[0] != '\0' ? KLI_strdup(prefix) : nullptr;
}

bool LUXO_prop_collection_type_get(KrakenPRIM *ptr, KrakenPROP *prop, KrakenPRIM *r_ptr)
{
  KrakenPRIM *type;

  KLI_assert(LUXO_prop_type(prop) == PROP_COLLECTION);

  *r_ptr = *ptr;
  *type = prim_ensure_property(prop)->GetPrim();

  return ((r_ptr->type = type) ? 1 : 0);
}

IDProperty **LUXO_prim_idprops_p(KrakenPRIM *ptr)
{
  KrakenPRIM *type = ptr->type;
  if (type == nullptr) {
    return nullptr;
  }
  if (type->idproperties == nullptr) {
    return nullptr;
  }

  return type->idproperties(ptr);
}

IDProperty *LUXO_prim_idprops(KrakenPRIM *ptr, bool create)
{
  IDProperty **property_ptr = LUXO_prim_idprops_p(ptr);
  if (property_ptr == NULL) {
    return NULL;
  }

  if (create && *property_ptr == NULL) {
    IDPropertyTemplate val = {0};
    *property_ptr = IDP_New(IDP_GROUP, &val, wabi::TfToken(__func__));
  }

  return *property_ptr;
}

bool LUXO_pointer_is_null(const KrakenPRIM *ptr)
{
  return (ptr->data == nullptr) || (ptr->owner_id == nullptr) || (ptr->type == nullptr);
}

/* generic path search func
 * if its needed this could also reference the IDProperty direct */
typedef struct IDP_Chain
{
  struct IDP_Chain *up; /* parent member, reverse and set to child for path conversion. */

  const char *name;
  int index;

} IDP_Chain;

static char *prim_idp_path_create(IDP_Chain *child_link)
{
  DynStr *dynstr = KLI_dynstr_new();
  char *path;
  bool is_first = true;

  int tot = 0;
  IDP_Chain *link = child_link;

  /* reverse the list */
  IDP_Chain *link_prev;
  link_prev = nullptr;
  while (link) {
    IDP_Chain *link_next = link->up;
    link->up = link_prev;
    link_prev = link;
    link = link_next;
    tot++;
  }

  for (link = link_prev; link; link = link->up) {
    /* pass */
    if (link->index >= 0) {
      KLI_dynstr_appendf(dynstr, is_first ? "%s[%d]" : ".%s[%d]", link->name, link->index);
    } else {
      KLI_dynstr_appendf(dynstr, is_first ? "%s" : ".%s", link->name);
    }

    is_first = false;
  }

  path = KLI_dynstr_get_cstring(dynstr);
  KLI_dynstr_free(dynstr);

  if (*path == '\0') {
    MEM_freeN(path);
    path = nullptr;
  }

  return path;
}

void LUXO_prop_pointer_add(KrakenPRIM *ptr, KrakenPROP *prop)
{
  KLI_assert(LUXO_prop_type(prop) == PROP_POINTER);

  if (/*idprop=*/prim_idproperty_check(&prop, ptr)) {
    /* already exists */
  } else if (prop->flag & PROP_IDPROPERTY) {
    IDPropertyTemplate val = {0};
    IDProperty *group;

    val.i = 0;

    group = LUXO_prim_idprops(ptr, 1);
    if (group) {
      IDP_AddToGroup(group, IDP_New(IDP_GROUP, &val, prop->GetName()));
    }
  } else {
    printf("%s %s.%s: only supported for id properties.\n",
           __func__,
           ptr->type->identifier,
           prop->GetName().data());
  }
}

KrakenPRIM prim_pointer_inherit_refine(KrakenPRIM *ptr, KrakenPRIM *type, void *data)
{
  if (data) {
    KrakenPRIM result;
    result.data = data;
    result.type = type;
    prim_pointer_inherit_id(type, ptr, &result);

    while (result.type->refine) {
      type = result.type->refine(&result);

      if (type == result.type) {
        break;
      }
      result.type = type;
    }
    return result;
  }
  return KrakenPRIM();
}

KrakenPRIM LUXO_prop_pointer_get(KrakenPRIM *ptr, KrakenPROP *prop)
{
  KrakenPrimPROP *pprop = (KrakenPrimPROP *)prop;
  IDProperty *idprop;

  static ThreadMutex lock = KLI_MUTEX_INITIALIZER;

  KLI_assert(LUXO_prop_type(prop) == PROP_POINTER);

  if ((idprop = prim_idproperty_check(&prop, ptr))) {
    pprop = (KrakenPrimPROP *)prop;

    if (LUXO_prim_is_ID(pprop->type)) {
      return prim_pointer_inherit_refine(ptr, pprop->type, IDP_Id(idprop));
    }

    /* for groups, data is idprop itself */
    if (pprop->type_fn) {
      return prim_pointer_inherit_refine(ptr, pprop->type_fn(ptr), idprop);
    }
    return prim_pointer_inherit_refine(ptr, pprop->type, idprop);
  }
  if (pprop->get) {
    return pprop->get(ptr);
  }
  if (prop->flag & PROP_IDPROPERTY) {
    /* NOTE: While creating/writing data in an accessor is really bad design-wise, this is
     * currently very difficult to avoid in that case. So a global mutex is used to keep ensuring
     * thread safety. */
    KLI_mutex_lock(&lock);
    /* NOTE: We do not need to check again for existence of the pointer after locking here, since
     * this is also done in #LUXO_prop_pointer_add itself. */
    LUXO_prop_pointer_add(ptr, prop);
    KLI_mutex_unlock(&lock);
    return LUXO_prop_pointer_get(ptr, prop);
  }
  return KrakenPRIM();
}

bool LUXO_prop_collection_lookup_int_has_fn(KrakenPROP *prop)
{
  KLI_assert(LUXO_prop_type(prop) == PROP_COLLECTION);
  CollectionPropPRIM *cprop = (CollectionPropPRIM *)prim_ensure_property(prop);
  return cprop->lookupint != nullptr;
}

int LUXO_prop_collection_lookup_int(KrakenPRIM *ptr, KrakenPROP *prop, int key, KrakenPRIM *r_ptr)
{
  CollectionPropPRIM *cprop = (CollectionPropPRIM *)prim_ensure_property(prop);

  KLI_assert(LUXO_prop_type(prop) == PROP_COLLECTION);

  if (cprop->lookupint) {
    /* we have a callback defined, use it */
    return cprop->lookupint(ptr, key, r_ptr);
  }
  /* no callback defined, just iterate and find the nth item */
  CollectionPropIT iter;
  int i;

  LUXO_prop_collection_begin(ptr, prop, &iter);
  for (i = 0; iter.valid; LUXO_prop_collection_next(&iter), i++) {
    if (i == key) {
      *r_ptr = iter.ptr;
      break;
    }
  }
  LUXO_prop_collection_end(&iter);

  if (!iter.valid) {
    memset(r_ptr, 0, sizeof(*r_ptr));
  }

  return iter.valid;
}

static char *prim_idp_path(KrakenPRIM *ptr,
                           IDProperty *haystack,
                           IDProperty *needle,
                           IDP_Chain *parent_link)
{
  char *path = nullptr;
  IDP_Chain link;

  IDProperty *iter;
  int i;

  KLI_assert(haystack->type == IDP_GROUP);

  link.up = parent_link;
  /* Always set both name and index, else a stale value might get used. */
  link.name = nullptr;
  link.index = -1;

  for (i = 0, iter = static_cast<IDProperty *>(haystack->data.group.first); iter;
       iter = iter->next, i++) {
    if (needle == iter) { /* found! */
      link.name = iter->name;
      link.index = -1;
      path = prim_idp_path_create(&link);
      break;
    }

    /* Early out in case the IDProperty type cannot contain RNA properties. */
    if (!ELEM(iter->type, IDP_GROUP, IDP_IDPARRAY)) {
      continue;
    }

    /* Ensure this is RNA. */
    /* NOTE: `iter` might be a fully user-defined IDProperty (a.k.a. custom data), which name
     * collides with an actual fully static RNA property of the same struct (which would then not
     * be flagged with `PROP_IDPROPERTY`).
     *
     * That case must be ignored here, we only want to deal with runtime RNA properties stored in
     * IDProps.
     *
     * See T84091. */
    KrakenPROP *prop = LUXO_prim_find_property(ptr, iter->name);
    if (prop == nullptr || (prop->flag & PROP_IDPROPERTY) == 0) {
      continue;
    }

    if (iter->type == IDP_GROUP) {
      if (prop->type == PROP_POINTER) {
        KrakenPRIM child_ptr = LUXO_prop_pointer_get(ptr, prop);
        if (LUXO_pointer_is_null(&child_ptr)) {
          /* Pointer ID prop might be a 'leaf' in the IDProp group hierarchy, in which case a NULL
           * value is perfectly valid. Just means it won't match the searched needle. */
          continue;
        }

        link.name = iter->name;
        link.index = -1;
        if ((path = prim_idp_path(&child_ptr, iter, needle, &link))) {
          break;
        }
      }
    } else if (iter->type == IDP_IDPARRAY) {
      if (prop->type == PROP_COLLECTION) {
        IDProperty *array = IDP_IDPArray(iter);
        if (needle >= array && needle < (iter->len + array)) { /* found! */
          link.name = iter->name;
          link.index = int(needle - array);
          path = prim_idp_path_create(&link);
          break;
        }
        int j;
        link.name = iter->name;
        for (j = 0; j < iter->len; j++, array++) {
          KrakenPRIM child_ptr;
          if (LUXO_prop_collection_lookup_int(ptr, prop, j, &child_ptr)) {
            if (LUXO_pointer_is_null(&child_ptr)) {
              /* Array item ID prop might be a 'leaf' in the IDProp group hierarchy, in which case
               * a NULL value is perfectly valid. Just means it won't match the searched needle. */
              continue;
            }
            link.index = j;
            if ((path = prim_idp_path(&child_ptr, array, needle, &link))) {
              break;
            }
          }
        }
        if (path) {
          break;
        }
      }
    }
  }

  return path;
}

static char *luxo_path_from_struct_to_idproperty(KrakenPRIM *ptr, IDProperty *needle)
{
  IDProperty *haystack = LUXO_prim_idprops(ptr, false);

  if (haystack) { /* can fail when called on bones */
    return prim_idp_path(ptr, haystack, needle, nullptr);
  }
  return nullptr;
}


static char *prim_path_from_ID_to_idpgroup(const KrakenPRIM *ptr)
{
  KrakenPRIM id_ptr;

  KLI_assert(ptr->owner_id != nullptr);

  /* TODO: Support Bones/PoseBones. no pointers stored to the bones from here, only the ID.
   *       See example in T25746.
   *       Unless this is added only way to find this is to also search
   *       all bones and pose bones of an armature or object.
   */
  LUXO_id_pointer_create(ptr->owner_id, &id_ptr);

  return luxo_path_from_struct_to_idproperty(&id_ptr, static_cast<IDProperty *>(ptr->data));
}

char *LUXO_path_from_ID_to_struct(const KrakenPRIM *ptr)
{
  char *ptrpath = nullptr;

  if (!ptr->owner_id || !ptr->data) {
    return nullptr;
  }

  if (!LUXO_prim_is_ID(ptr->type)) {
    if (ptr->type->IsValid()) {
      /* if type has a path to some ID, use it */
      ptrpath = KLI_strdup(ptr->type->GetPath().GetText());
    } else if (ptr->type->nested && LUXO_prim_is_ID(ptr->type->nested)) {
      KrakenPRIM parentptr;
      KrakenPROP *userprop;

      /* find the property in the struct we're nested in that references this struct, and
       * use its identifier as the first part of the path used...
       */
      LUXO_id_pointer_create(ptr->owner_id, &parentptr);
      userprop = prim_struct_find_nested(&parentptr, ptr->type);

      if (userprop) {
        ptrpath = KLI_strdup(LUXO_prop_identifier(userprop).data());
      } else {
        return nullptr; /* can't do anything about this case yet... */
      }
    } else if (LUXO_prim_is_a(ptr->type, ptr)) {
      /* special case, easier to deal with here than in ptr->type->path() */
      return prim_path_from_ID_to_idpgroup(ptr);
    } else {
      return nullptr;
    }
  }

  return ptrpath;
}

static void prim_ensure_property_multi_array_length(const KrakenPRIM *ptr,
                                                    KrakenPROP *prop,
                                                    int length[])
{
  if (prop->IsValid()) {
    if (prop->GetTypeName() && prop->GetTypeName().IsArray()) {
      prop->arraylength = (unsigned int)prop->GetTypeName().GetArrayType().GetDimensions().size;
    } else {
      // memcpy(length, prop->arraylength, prop->arraydimension * sizeof(int));
    }
  } else {
    IDProperty *idprop = (IDProperty *)prop;

    if (idprop->type == IDP_ARRAY) {
      length[0] = idprop->len;
    } else {
      length[0] = 0;
    }
  }
}

int LUXO_prop_array_dimension(const KrakenPRIM *ptr, KrakenPROP *prop, int length[])
{
  KrakenPROP *rprop = prim_ensure_property(prop);

  if (length) {
    prim_ensure_property_multi_array_length(ptr, prop, length);
  }

  return rprop->arraylength;
}

int LUXO_prop_multi_array_length(KrakenPRIM *ptr, KrakenPROP *prop, int dim)
{
  int len[LUXO_MAX_ARRAY_DIMENSION];

  prim_ensure_property_multi_array_length(ptr, prop, len);

  return len[dim];
}

static void prim_path_array_multi_from_flat_index(const int dimsize[LUXO_MAX_ARRAY_LENGTH],
                                                  const int totdims,
                                                  const int index_dim,
                                                  int index,
                                                  int r_index_multi[LUXO_MAX_ARRAY_LENGTH])
{
  int dimsize_step[LUXO_MAX_ARRAY_LENGTH + 1];
  int i = totdims - 1;
  dimsize_step[i + 1] = 1;
  dimsize_step[i] = dimsize[i];
  while (--i != -1) {
    dimsize_step[i] = dimsize[i] * dimsize_step[i + 1];
  }
  while (++i != index_dim) {
    int index_round = index / dimsize_step[i + 1];
    r_index_multi[i] = index_round;
    index -= (index_round * dimsize_step[i + 1]);
  }
  KLI_assert(index == 0);
}

static void prim_path_array_multi_string_from_flat_index(const KrakenPRIM *ptr,
                                                         KrakenPROP *prop,
                                                         int index_dim,
                                                         int index,
                                                         char *index_str,
                                                         int index_str_len)
{
  int dimsize[LUXO_MAX_ARRAY_LENGTH];
  int totdims = LUXO_prop_array_dimension(ptr, prop, dimsize);
  int index_multi[LUXO_MAX_ARRAY_LENGTH];

  prim_path_array_multi_from_flat_index(dimsize, totdims, index_dim, index, index_multi);

  for (int i = 0, offset = 0; (i < index_dim) && (offset < index_str_len); i++) {
    offset += KLI_snprintf_rlen(&index_str[offset],
                                index_str_len - offset,
                                "[%d]",
                                index_multi[i]);
  }
}

char *LUXO_path_from_ID_to_property_index(const KrakenPRIM *ptr,
                                          KrakenPROP *prop,
                                          int index_dim,
                                          int index)
{
  const bool is_valid = prop->IsValid();
  const char *propname;
  char *ptrpath, *path;

  if (!ptr->owner_id || !ptr->data) {
    return nullptr;
  }

  /* path from ID to the struct holding this property */
  ptrpath = LUXO_path_from_ID_to_struct(ptr);

  propname = LUXO_prop_identifier(prop).data();

  /* support indexing w/ multi-dimensional arrays */
  char index_str[LUXO_MAX_ARRAY_LENGTH * 12 + 1];
  if (index_dim == 0) {
    index_str[0] = '\0';
  } else {
    prim_path_array_multi_string_from_flat_index(ptr,
                                                 prop,
                                                 index_dim,
                                                 index,
                                                 index_str,
                                                 sizeof(index_str));
  }

  if (ptrpath) {
    if (is_valid) {
      path = KLI_sprintfN("%s.%s%s", ptrpath, propname, index_str);
    } else {
      char propname_esc[MAX_IDPROP_NAME * 2];
      KLI_str_escape(propname_esc, propname, sizeof(propname_esc));
      path = KLI_sprintfN("%s[\"%s\"]%s", ptrpath, propname_esc, index_str);
    }
    MEM_freeN(ptrpath);
  } else if (LUXO_prim_is_ID(ptr->type)) {
    if (is_valid) {
      path = KLI_sprintfN("%s%s", propname, index_str);
    } else {
      char propname_esc[MAX_IDPROP_NAME * 2];
      KLI_str_escape(propname_esc, propname, sizeof(propname_esc));
      path = KLI_sprintfN("[\"%s\"]%s", propname_esc, index_str);
    }
  } else {
    path = nullptr;
  }

  return path;
}

char *LUXO_path_from_ID_to_property(const KrakenPRIM *ptr, KrakenPROP *prop)
{
  return LUXO_path_from_ID_to_property_index(ptr, prop, 0, -1);
}

char *LUXO_path_from_real_ID_to_property_index(Main *kmain,
                                               const KrakenPRIM *ptr,
                                               KrakenPROP *prop,
                                               int index_dim,
                                               int index,
                                               ID **r_real_id)
{
  char *path = LUXO_path_from_ID_to_property_index(ptr, prop, index_dim, index);

  /* NULL path is always an error here, in that case do not return the 'fake ID from real ID' part
   * of the path either. */
  return path != nullptr ? prim_prepend_real_ID_path(kmain, ptr->owner_id, path, r_real_id) :
                           nullptr;
}
