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

#include "WM_tokens.h"
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

const KrakenPRIM KrakenPRIM_NULL = {};

KrakenPRIM PRIM_Action;
KrakenPRIM PRIM_Area;
KrakenPRIM PRIM_Armature;
KrakenPRIM PRIM_Brush;
KrakenPRIM PRIM_CacheFile;
KrakenPRIM PRIM_Camera;
KrakenPRIM PRIM_Collection;
KrakenPRIM PRIM_Context;
KrakenPRIM PRIM_Curve;
KrakenPRIM PRIM_FreestyleLineStyle;
KrakenPRIM PRIM_GreasePencil;
KrakenPRIM PRIM_ID;
KrakenPRIM PRIM_Image;
KrakenPRIM PRIM_Key;
KrakenPRIM PRIM_KrakenPRIM;
KrakenPRIM PRIM_KrakenPROP;
KrakenPRIM PRIM_Lattice;
KrakenPRIM PRIM_Library;
KrakenPRIM PRIM_Light;
KrakenPRIM PRIM_LightProbe;
KrakenPRIM PRIM_Mask;
KrakenPRIM PRIM_Material;
KrakenPRIM PRIM_Mesh;
KrakenPRIM PRIM_MetaBall;
KrakenPRIM PRIM_MovieClip;
KrakenPRIM PRIM_NodeTree;
KrakenPRIM PRIM_Object;
KrakenPRIM PRIM_PaintCurve;
KrakenPRIM PRIM_Palette;
KrakenPRIM PRIM_ParticleSettings;
KrakenPRIM PRIM_PointCloud;
KrakenPRIM PRIM_Region;
KrakenPRIM PRIM_Scene;
KrakenPRIM PRIM_Screen;
KrakenPRIM PRIM_Simulation;
KrakenPRIM PRIM_Sound;
KrakenPRIM PRIM_Speaker;
KrakenPRIM PRIM_Struct;
KrakenPRIM PRIM_StageData;
KrakenPRIM PRIM_Text;
KrakenPRIM PRIM_Texture;
KrakenPRIM PRIM_VectorFont;
KrakenPRIM PRIM_Volume;
KrakenPRIM PRIM_Window;
KrakenPRIM PRIM_WindowManager;
KrakenPRIM PRIM_WorkSpace;
KrakenPRIM PRIM_World;

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

int LUXO_prop_flag(KrakenPROP *prop)
{
  return prim_ensure_property(prop)->flag;
}

void LUXO_prop_int_range(KrakenPRIM *ptr, KrakenPROP *prop, int *hardmin, int *hardmax)
{
  IntPrimPROP *iprop = (IntPrimPROP *)prim_ensure_property(prop);
  int softmin, softmax;

  if (prop->GetPrim().GetParent() == UsdPrim()) {
    const IDProperty *idprop = (IDProperty *)prop;
    if (idprop->ui_data) {
      IDPropertyUIDataInt *ui_data = (IDPropertyUIDataInt *)idprop->ui_data;
      *hardmin = ui_data->min;
      *hardmax = ui_data->max;
    } else {
      *hardmin = INT_MIN;
      *hardmax = INT_MAX;
    }
    return;
  }

  if (iprop->range) {
    *hardmin = INT_MIN;
    *hardmax = INT_MAX;

    iprop->range(ptr, hardmin, hardmax, &softmin, &softmax);
  } else if (iprop->range_ex) {
    *hardmin = INT_MIN;
    *hardmax = INT_MAX;

    iprop->range_ex(ptr, prop, hardmin, hardmax, &softmin, &softmax);
  } else {
    *hardmin = iprop->hardmin;
    *hardmax = iprop->hardmax;
  }
}

void LUXO_prop_float_range(KrakenPRIM *ptr, KrakenPROP *prop, float *hardmin, float *hardmax)
{
  FloatPrimPROP *fprop = (FloatPrimPROP *)prim_ensure_property(prop);
  float softmin, softmax;

  if (prop->GetPrim().GetParent() == UsdPrim()) {
    const IDProperty *idprop = (IDProperty *)prop;
    if (idprop->ui_data) {
      IDPropertyUIDataFloat *ui_data = (IDPropertyUIDataFloat *)idprop->ui_data;
      *hardmin = (float)ui_data->min;
      *hardmax = (float)ui_data->max;
    } else {
      *hardmin = -FLT_MAX;
      *hardmax = FLT_MAX;
    }
    return;
  }

  if (fprop->range) {
    *hardmin = -FLT_MAX;
    *hardmax = FLT_MAX;

    fprop->range(ptr, hardmin, hardmax, &softmin, &softmax);
  } else if (fprop->range_ex) {
    *hardmin = -FLT_MAX;
    *hardmax = FLT_MAX;

    fprop->range_ex(ptr, prop, hardmin, hardmax, &softmin, &softmax);
  } else {
    *hardmin = fprop->hardmin;
    *hardmax = fprop->hardmax;
  }
}

PropertyScaleType LUXO_prop_ui_scale(KrakenPROP *prop)
{
  KrakenPROP *prim_prop = prim_ensure_property(prop);

  switch (prim_prop->type) {
    case PROP_INT: {
      IntPrimPROP *iprop = (IntPrimPROP *)prim_prop;
      return iprop->ui_scale_type;
    }
    case PROP_FLOAT: {
      FloatPrimPROP *fprop = (FloatPrimPROP *)prim_prop;
      return fprop->ui_scale_type;
    }
    default:
      return PROP_SCALE_LINEAR;
  }
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
  KrakenPROP *prim_prop = prim_ensure_property(prop);

  /* For custom properties, find and parse the 'subtype' metadata field. */
  if (prop->GetPrim().GetParent() == UsdPrim()) {
    IDProperty *idprop = (IDProperty *)prop;

    if (idprop->ui_data) {
      IDPropertyUIData *ui_data = idprop->ui_data;
      return (PropertySubType)ui_data->prim_subtype;
    }
  }

  return prim_prop->subtype;
}

bool LUXO_prop_editable_flag(KrakenPRIM *ptr, KrakenPROP *prop)
{
  int flag;
  const char *dummy_info;

  prop = prim_ensure_property(prop);
  flag = prop->editable ? prop->editable(ptr, &dummy_info) : prop->flag;
  return (flag & PROP_EDITABLE) != 0;
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

bool LUXO_prim_idprops_check(KrakenPRIM *sprim)
{
  return (sprim && sprim->idproperties);
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
  CollectionPrimPROP *cprop = (CollectionPrimPROP *)iter->prop;

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
  CollectionPrimPROP *cprop = (CollectionPrimPROP *)prim_ensure_property(iter->prop);

  if (iter->idprop) {
    prim_iterator_array_next(iter);

    if (iter->valid) {
      prim_property_collection_get_idp(iter);
    }
  } else {
    cprop->next(iter);
  }
}

void LUXO_prop_collection_skip(CollectionPropIT *iter, int num)
{
  CollectionPrimPROP *cprop = (CollectionPrimPROP *)prim_ensure_property(iter->prop);
  int i;

  if (num > 1 && (iter->idprop || (cprop->property.flag_internal & PROP_INTERN_RAW_ARRAY))) {
    /* fast skip for array */
    ArrayIT *internal = &iter->internal.array;

    if (!internal->skip) {
      internal->ptr += internal->itemsize * (num - 1);
      iter->valid = (internal->ptr < internal->endptr);
      if (iter->valid) {
        LUXO_prop_collection_next(iter);
      }
      return;
    }
  }

  /* slow iteration otherwise */
  for (i = 0; i < num && iter->valid; i++) {
    LUXO_prop_collection_next(iter);
  }
}

int LUXO_prop_collection_assign_int(KrakenPRIM *ptr,
                                    KrakenPROP *prop,
                                    const int key,
                                    const KrakenPRIM *assign_ptr)
{
  CollectionPrimPROP *cprop = (CollectionPrimPROP *)prim_ensure_property(prop);

  KLI_assert(LUXO_prop_type(prop) == PROP_COLLECTION);

  if (cprop->assignint) {
    /* we have a callback defined, use it */
    return cprop->assignint(ptr, key, assign_ptr);
  }

  return 0;
}

void LUXO_prop_collection_end(CollectionPropIT *iter)
{
  CollectionPrimPROP *cprop = (CollectionPrimPROP *)prim_ensure_property(iter->prop);

  if (iter->idprop) {
    prim_iterator_array_end(iter);
  } else {
    cprop->end(iter);
  }
}

bool LUXO_prop_collection_is_empty(KrakenPRIM *ptr, KrakenPROP *prop)
{
  KLI_assert(LUXO_prop_type(prop) == PROP_COLLECTION);
  CollectionPropIT iter;
  LUXO_prop_collection_begin(ptr, prop, &iter);
  bool test = iter.valid;
  LUXO_prop_collection_end(&iter);
  return !test;
}

// int LUXO_prop_collection_raw_set(ReportList *reports,
//                                     KrakenPRIM *ptr,
//                                     KrakenPROP *prop,
//                                     const char *propname,
//                                     void *array,
//                                     RawPropertyType type,
//                                     int len)
// {
//   return luxo_raw_access(reports, ptr, prop, propname, array, type, len, 1);
// }

KrakenPROP *LUXO_prim_name_property(const KrakenPRIM *prim)
{ 
  KrakenPROP *prop;
  
  *prop = prim->GetAttribute(prim->GetName());

  return prop;
}

void LUXO_prop_string_get(KrakenPRIM *ptr, KrakenPROP *prop, char *value)
{
  KrakenPROPString *sprop = (KrakenPROPString *)prop;
  IDProperty *idprop;

  KLI_assert(LUXO_prop_type(prop) == PROP_STRING);

  if ((idprop = prim_idproperty_check(&prop, ptr))) {
    /* editing bytes is not 100% supported
     * since they can contain NIL chars */
    if (idprop->subtype == IDP_STRING_SUB_BYTE) {
      memcpy(value, IDP_String(idprop), idprop->len);
      value[idprop->len] = '\0';
    }
    else {
      memcpy(value, IDP_String(idprop), idprop->len);
    }
  }
  else if (sprop->get) {
    sprop->get(ptr, value);
  }
  else if (sprop->get_ex) {
    sprop->get_ex(ptr, prop, value);
  }
  else {
    strcpy(value, sprop->defaultvalue);
  }
}

char *LUXO_prop_string_get_alloc(KrakenPRIM *ptr, KrakenPROP *prop, char *fixedbuf, int fixedlen, int *r_len)
{
  char *buf;
  int length;

  KLI_assert(LUXO_prop_type(prop) == PROP_STRING);

  length = LUXO_prop_string_length(ptr, prop);

  if (length + 1 < fixedlen) {
    buf = fixedbuf;
  }
  else {
    buf = static_cast<char *>(MEM_mallocN(sizeof(char) * (length + 1), __func__));
  }

#ifndef NDEBUG
  /* safety check to ensure the string is actually set */
  buf[length] = 255;
#endif

  LUXO_prop_string_get(ptr, prop, buf);

#ifndef NDEBUG
  KLI_assert(buf[length] == '\0');
#endif

  if (r_len) {
    *r_len = length;
  }

  return buf;
}

char *LUXO_string_get_alloc(KrakenPRIM *ptr, const char *name, char *fixedbuf, int fixedlen, int *r_len)
{
  KrakenPROP *prop = LUXO_prim_find_property(ptr, name);

  if (prop) {
    return LUXO_prop_string_get_alloc(ptr, prop, fixedbuf, fixedlen, r_len);
  }
  printf("%s: %s.%s not found.\n", __func__, ptr->type->identifier.data(), name);
  if (r_len != nullptr) {
    *r_len = 0;
  }
  return nullptr;
}

KrakenFUNC *LUXO_prim_find_function(KrakenPRIM *prim, const char *identifier)
{
  KrakenFUNC *func;
  for (; prim; prim = prim->base) {
    func = (KrakenFUNC *)KLI_findstring_ptr(&prim->functions, identifier, offsetof(KrakenFUNC, identifier));
    if (func) {
      return func;
    }
  }

  return nullptr;
}

char *LUXO_prim_name_get_alloc(KrakenPRIM *ptr, char *fixedbuf, int fixedlen, int *r_len)
{
  KrakenPROP *nameprop;

  if (ptr->data && (nameprop = LUXO_prim_name_property(ptr->type))) {
    return LUXO_prop_string_get_alloc(ptr, nameprop, fixedbuf, fixedlen, r_len);
  }

  return NULL;
}

bool LUXO_prop_collection_lookup_string_has_nameprop(KrakenPROP *prop)
{
  KLI_assert(LUXO_prop_type(prop) == PROP_COLLECTION);
  CollectionPrimPROP *cprop = (CollectionPrimPROP *)prim_ensure_property(prop);
  return (cprop->item_type && cprop->item_type->GetAttribute(WM_ID_(name)));
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
    CollectionPrimPROP *cprop = (CollectionPrimPROP *)prop;
    cprop->begin(iter, ptr);
  }
}

/* ID Properties */

void prim_idproperty_touch(IDProperty *idprop)
{
  /* so the property is seen as 'set'. */
  idprop->flag &= ~IDP_FLAG_ANCHOR;
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

bool LUXO_prop_is_idprop(const KrakenPROP *prop)
{
  return (prop->magic != LUXO_MAGIC);
}

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

const ListBase *LUXO_prim_type_functions(KrakenPRIM *sprim)
{
  return &sprim->functions;
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

static void prim_property_update(kContext *C,
                                 Main *kmain,
                                 Scene *scene,
                                 KrakenPRIM *ptr,
                                 KrakenPROP *prop)
{
  const bool is_prim = (prop->GetPrim().GetParent() != UsdPrim());
  prop = prim_ensure_property(prop);

  if (is_prim) {
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

  if (!is_prim || (prop->flag & PROP_IDPROPERTY)) {

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
  return (prop->GetPrim().GetParent() == UsdPrim() || prop->update || prop->noteflag);
}

void LUXO_prop_update(kContext *C, KrakenPRIM *ptr, KrakenPROP *prop)
{
  prim_property_update(C, CTX_data_main(C), CTX_data_scene(C), ptr, prop);
}

static int prim_ensure_property_array_length(KrakenPRIM *ptr, KrakenPROP *prop)
{
  if (prop->GetPrim().GetParent() != UsdPrim()) {
    int arraylen[LUXO_MAX_ARRAY_DIMENSION];
    return (prop->getlength && ptr->data) ? prop->getlength(ptr, arraylen) :
                                            (int)prop->GetTotalArrayLength();
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
  EnumPrimPROP *eprop = (EnumPrimPROP *)prop;
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
  EnumPrimPROP *eprop = (EnumPrimPROP *)prop;
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
  EnumPrimPROP *eprop = (EnumPrimPROP *)prim_ensure_property(prop);

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
    CollectionPrimPROP *cprop = (CollectionPrimPROP *)prop;

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

int LUXO_prop_collection_lookup_string_index(KrakenPRIM *ptr,
                                             KrakenPROP *prop,
                                             const char *key,
                                             KrakenPRIM *r_ptr,
                                             int *r_index)
{
  CollectionPrimPROP *cprop = (CollectionPrimPROP *)prim_ensure_property(prop);

  KLI_assert(LUXO_prop_type(prop) == PROP_COLLECTION);

  if (cprop->lookupstring) {
    /* we have a callback defined, use it */
    return cprop->lookupstring(ptr, key, r_ptr);
  }
  /* no callback defined, compare with name properties if they exist */
  CollectionPropIT iter;
  KrakenPROP *nameprop;
  char name[256], *nameptr;
  int found = 0;
  int keylen = strlen(key);
  int namelen;
  int index = 0;

  LUXO_prop_collection_begin(ptr, prop, &iter);
  for (; iter.valid; LUXO_prop_collection_next(&iter), index++) {
    if (iter.ptr.data && iter.ptr.type->GetAttribute(TfToken("name"))) {
      *nameprop = iter.ptr.type->GetAttribute(TfToken("name"));

      nameptr = KLI_strdup(nameprop->GetName().data());

      if ((keylen == namelen) && STREQ(nameptr, key)) {
        *r_ptr = iter.ptr;
        found = 1;
      }

      if ((char *)&name != nameptr) {
        MEM_freeN(nameptr);
      }

      if (found) {
        break;
      }
    }
  }
  LUXO_prop_collection_end(&iter);

  if (!iter.valid) {
    memset(r_ptr, 0, sizeof(*r_ptr));
    *r_index = -1;
  } else {
    *r_index = index;
  }

  return iter.valid;
}

int LUXO_prop_collection_lookup_string(KrakenPRIM *ptr,
                                       KrakenPROP *prop,
                                       const char *key,
                                       KrakenPRIM *r_ptr)
{
  int index;
  return LUXO_prop_collection_lookup_string_index(ptr, prop, key, r_ptr, &index);
}

bool LUXO_prop_collection_lookup_string_has_fn(KrakenPROP *prop)
{
  KLI_assert(LUXO_prop_type(prop) == PROP_COLLECTION);
  CollectionPrimPROP *cprop = (CollectionPrimPROP *)prim_ensure_property(prop);
  return cprop->lookupstring != NULL;
}

int LUXO_prop_collection_length(KrakenPRIM *ptr, KrakenPROP *prop)
{
  CollectionPrimPROP *cprop = (CollectionPrimPROP *)prop;
  IDProperty *idprop;

  KLI_assert(LUXO_prop_type(prop) == PROP_COLLECTION);

  if ((idprop = prim_idproperty_check(&prop, ptr))) {
    return idprop->len;
  }
  if (cprop->length) {
    return cprop->length(ptr);
  }
  CollectionPropIT iter;
  int length = 0;

  LUXO_prop_collection_begin(ptr, prop, &iter);
  for (; iter.valid; LUXO_prop_collection_next(&iter)) {
    length++;
  }
  LUXO_prop_collection_end(&iter);

  return length;
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
  if (property_ptr == nullptr) {
    return nullptr;
  }

  if (create && *property_ptr == nullptr) {
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
           ptr->type->identifier.data(),
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
  CollectionPrimPROP *cprop = (CollectionPrimPROP *)prim_ensure_property(prop);
  return cprop->lookupint != nullptr;
}

int LUXO_prop_collection_lookup_int(KrakenPRIM *ptr, KrakenPROP *prop, int key, KrakenPRIM *r_ptr)
{
  CollectionPrimPROP *cprop = (CollectionPrimPROP *)prim_ensure_property(prop);

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
  if (prop->GetPrim().GetParent() != UsdPrim()) {
    if (prop->getlength) {
      prop->getlength(ptr, length);
    } else {
      memcpy(length, prop->arraylength, prop->GetArrayDimensions().size * sizeof(int));
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

int LUXO_prop_float_clamp(KrakenPRIM *ptr, KrakenPROP *prop, float *value)
{
  float min, max;

  LUXO_prop_float_range(ptr, prop, &min, &max);

  if (*value < min) {
    *value = min;
    return -1;
  }
  if (*value > max) {
    *value = max;
    return 1;
  }
  return 0;
}

int LUXO_prop_int_clamp(KrakenPRIM *ptr, KrakenPROP *prop, int *value)
{
  int min, max;

  LUXO_prop_int_range(ptr, prop, &min, &max);

  if (*value < min) {
    *value = min;
    return -1;
  }
  if (*value > max) {
    *value = max;
    return 1;
  }
  return 0;
}

int LUXO_prop_array_dimension(const KrakenPRIM *ptr, KrakenPROP *prop, int length[])
{
  KrakenPROP *rprop = prim_ensure_property(prop);

  if (length) {
    prim_ensure_property_multi_array_length(ptr, prop, length);
  }

  return rprop->GetArrayDimensions().size;
}

static void prim_property_float_fill_default_array_values(const float *defarr,
                                                          int defarr_length,
                                                          float defvalue,
                                                          int out_length,
                                                          float *r_values)
{
  if (defarr && defarr_length > 0) {
    defarr_length = MIN2(defarr_length, out_length);
    memcpy(r_values, defarr, sizeof(float) * defarr_length);
  } else {
    defarr_length = 0;
  }

  for (int i = defarr_length; i < out_length; i++) {
    r_values[i] = defvalue;
  }
}

static void prim_prop_float_get_default_array_values(KrakenPRIM *ptr,
                                                     FloatPrimPROP *fprop,
                                                     float *r_values)
{
  int length = fprop->property.GetTotalArrayLength();
  int out_length = LUXO_prop_array_length(ptr, (KrakenPROP *)fprop);

  prim_property_float_fill_default_array_values(fprop->defaultarray,
                                                length,
                                                fprop->defaultvalue,
                                                out_length,
                                                r_values);
}

static void prim_prop_boolean_fill_default_array_values(const bool *defarr,
                                                        int defarr_length,
                                                        bool defvalue,
                                                        int out_length,
                                                        bool *r_values)
{
  if (defarr && defarr_length > 0) {
    defarr_length = MIN2(defarr_length, out_length);
    memcpy(r_values, defarr, sizeof(bool) * defarr_length);
  } else {
    defarr_length = 0;
  }

  for (int i = defarr_length; i < out_length; i++) {
    r_values[i] = defvalue;
  }
}

static void prim_prop_boolean_get_default_array_values(KrakenPRIM *ptr,
                                                       BoolPrimPROP *bprop,
                                                       bool *r_values)
{
  int length = bprop->property.GetTotalArrayLength();
  int out_length = LUXO_prop_array_length(ptr, (KrakenPROP *)bprop);

  prim_prop_boolean_fill_default_array_values(bprop->defaultarray,
                                              length,
                                              bprop->defaultvalue,
                                              out_length,
                                              r_values);
}

bool LUXO_prop_boolean_get(KrakenPRIM *ptr, KrakenPROP *prop)
{
  BoolPrimPROP *bprop = (BoolPrimPROP *)prop;
  IDProperty *idprop;
  bool value;

  KLI_assert(LUXO_prop_type(prop) == PROP_BOOLEAN);
  KLI_assert(LUXO_prop_array_check(prop) == false);

  if ((idprop = prim_idproperty_check(&prop, ptr))) {
    value = IDP_Int(idprop) != 0;
  } else if (bprop->get) {
    value = bprop->get(ptr);
  } else if (bprop->get_ex) {
    value = bprop->get_ex(ptr, prop);
  } else {
    value = bprop->defaultvalue;
  }

  KLI_assert(ELEM(value, false, true));

  return value;
}

void LUXO_prop_boolean_get_array(KrakenPRIM *ptr, KrakenPROP *prop, bool *values)
{
  BoolPrimPROP *bprop = (BoolPrimPROP *)prop;
  IDProperty *idprop;

  KLI_assert(LUXO_prop_type(prop) == PROP_BOOLEAN);
  KLI_assert(LUXO_prop_array_check(prop) != false);

  if ((idprop = prim_idproperty_check(&prop, ptr))) {
    if (prop->GetArrayDimensions() == SdfTupleDimensions()) {
      values[0] = LUXO_prop_boolean_get(ptr, prop);
    } else {
      int *values_src = static_cast<int *>(IDP_Array(idprop));
      for (uint i = 0; i < idprop->len; i++) {
        values[i] = (bool)values_src[i];
      }
    }
  } else if (prop->GetArrayDimensions() == SdfTupleDimensions()) {
    values[0] = LUXO_prop_boolean_get(ptr, prop);
  } else if (bprop->getarray) {
    bprop->getarray(ptr, values);
  } else if (bprop->getarray_ex) {
    bprop->getarray_ex(ptr, prop, values);
  } else {
    prim_prop_boolean_get_default_array_values(ptr, bprop, values);
  }
}

bool LUXO_prop_boolean_get_index(KrakenPRIM *ptr, KrakenPROP *prop, int index)
{
  bool tmp[LUXO_MAX_ARRAY_LENGTH];
  int len = prim_ensure_property_array_length(ptr, prop);
  bool value;

  KLI_assert(LUXO_prop_type(prop) == PROP_BOOLEAN);
  KLI_assert(LUXO_prop_array_check(prop) != false);
  KLI_assert(index >= 0);
  KLI_assert(index < len);

  if (len <= LUXO_MAX_ARRAY_LENGTH) {
    LUXO_prop_boolean_get_array(ptr, prop, tmp);
    value = tmp[index];
  } else {
    bool *tmparray;

    tmparray = static_cast<bool *>(MEM_mallocN(sizeof(bool) * len, __func__));
    LUXO_prop_boolean_get_array(ptr, prop, tmparray);
    value = tmparray[index];
    MEM_freeN(tmparray);
  }

  KLI_assert(ELEM(value, false, true));

  return value;
}

void LUXO_prop_boolean_set_array(KrakenPRIM *ptr, KrakenPROP *prop, const bool *values)
{
  BoolPrimPROP *bprop = (BoolPrimPROP *)prop;
  IDProperty *idprop;

  KLI_assert(LUXO_prop_type(prop) == PROP_BOOLEAN);
  KLI_assert(LUXO_prop_array_check(prop) != false);

  if ((idprop = prim_idproperty_check(&prop, ptr))) {
    if (prop->GetArrayDimensions() == SdfTupleDimensions()) {
      IDP_Int(idprop) = values[0];
    } else {
      int *values_dst = static_cast<int *>(IDP_Array(idprop));
      for (uint i = 0; i < idprop->len; i++) {
        values_dst[i] = (int)values[i];
      }
    }
    prim_idproperty_touch(idprop);
  } else if (prop->GetArrayDimensions() == SdfTupleDimensions()) {
    LUXO_prop_boolean_set(ptr, prop, values[0]);
  } else if (bprop->setarray) {
    bprop->setarray(ptr, values);
  } else if (bprop->setarray_ex) {
    bprop->setarray_ex(ptr, prop, values);
  } else if (prop->flag & PROP_EDITABLE) {
    IDPropertyTemplate val = {0};
    IDProperty *group;

    val.array.len = prop->GetTotalArrayLength();
    val.array.type = IDP_INT;

    group = LUXO_prim_idprops(ptr, 1);
    if (group) {
      idprop = IDP_New(IDP_ARRAY, &val, prop->GetName());
      IDP_AddToGroup(group, idprop);
      int *values_dst = static_cast<int *>(IDP_Array(idprop));
      for (uint i = 0; i < idprop->len; i++) {
        values_dst[i] = (int)values[i];
      }
    }
  }
}

void LUXO_prop_boolean_set_index(KrakenPRIM *ptr, KrakenPROP *prop, int index, bool value)
{
  bool tmp[LUXO_MAX_ARRAY_LENGTH];
  int len = prim_ensure_property_array_length(ptr, prop);

  KLI_assert(LUXO_prop_type(prop) == PROP_BOOLEAN);
  KLI_assert(LUXO_prop_array_check(prop) != false);
  KLI_assert(index >= 0);
  KLI_assert(index < len);
  KLI_assert(ELEM(value, false, true));

  if (len <= LUXO_MAX_ARRAY_LENGTH) {
    LUXO_prop_boolean_get_array(ptr, prop, tmp);
    tmp[index] = value;
    LUXO_prop_boolean_set_array(ptr, prop, tmp);
  } else {
    bool *tmparray;

    tmparray = static_cast<bool *>(MEM_mallocN(sizeof(bool) * len, __func__));
    LUXO_prop_boolean_get_array(ptr, prop, tmparray);
    tmparray[index] = value;
    LUXO_prop_boolean_set_array(ptr, prop, tmparray);
    MEM_freeN(tmparray);
  }
}

void LUXO_prop_boolean_set(KrakenPRIM *ptr, KrakenPROP *prop, bool value)
{
  BoolPrimPROP *bprop = (BoolPrimPROP *)prop;
  IDProperty *idprop;

  KLI_assert(LUXO_prop_type(prop) == PROP_BOOLEAN);
  KLI_assert(LUXO_prop_array_check(prop) == false);
  KLI_assert(ELEM(value, false, true));

  /* just in case other values are passed */
  KLI_assert(ELEM(value, true, false));

  if ((idprop = prim_idproperty_check(&prop, ptr))) {
    IDP_Int(idprop) = (int)value;
    prim_idproperty_touch(idprop);
  } else if (bprop->set) {
    bprop->set(ptr, value);
  } else if (bprop->set_ex) {
    bprop->set_ex(ptr, prop, value);
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

static void prim_prop_int_fill_default_array_values(const int *defarr,
                                                    int defarr_length,
                                                    int defvalue,
                                                    int out_length,
                                                    int *r_values)
{
  if (defarr && defarr_length > 0) {
    defarr_length = MIN2(defarr_length, out_length);
    memcpy(r_values, defarr, sizeof(int) * defarr_length);
  } else {
    defarr_length = 0;
  }

  for (int i = defarr_length; i < out_length; i++) {
    r_values[i] = defvalue;
  }
}

static void prim_prop_int_get_default_array_values(KrakenPRIM *ptr,
                                                   IntPrimPROP *iprop,
                                                   int *r_values)
{
  int length = iprop->property.GetTotalArrayLength();
  int out_length = LUXO_prop_array_length(ptr, (KrakenPROP *)iprop);

  prim_prop_int_fill_default_array_values(iprop->defaultarray,
                                          length,
                                          iprop->defaultvalue,
                                          out_length,
                                          r_values);
}

int LUXO_prop_int_get(KrakenPRIM *ptr, KrakenPROP *prop)
{
  IntPrimPROP *iprop = (IntPrimPROP *)prop;
  IDProperty *idprop;

  KLI_assert(LUXO_prop_type(prop) == PROP_INT);
  KLI_assert(LUXO_prop_array_check(prop) == false);

  if ((idprop = prim_idproperty_check(&prop, ptr))) {
    return IDP_Int(idprop);
  }
  if (iprop->get) {
    return iprop->get(ptr);
  }
  if (iprop->get_ex) {
    return iprop->get_ex(ptr, prop);
  }
  return iprop->defaultvalue;
}

int LUXO_prop_int_get_index(KrakenPRIM *ptr, KrakenPROP *prop, int index)
{
  int tmp[LUXO_MAX_ARRAY_LENGTH];
  int len = prim_ensure_property_array_length(ptr, prop);

  KLI_assert(LUXO_prop_type(prop) == PROP_INT);
  KLI_assert(LUXO_prop_array_check(prop) != false);
  KLI_assert(index >= 0);
  KLI_assert(index < len);

  if (len <= LUXO_MAX_ARRAY_LENGTH) {
    LUXO_prop_int_get_array(ptr, prop, tmp);
    return tmp[index];
  }
  int *tmparray, value;

  tmparray = static_cast<int *>(MEM_mallocN(sizeof(int) * len, __func__));
  LUXO_prop_int_get_array(ptr, prop, tmparray);
  value = tmparray[index];
  MEM_freeN(tmparray);

  return value;
}

void LUXO_prop_int_get_array(KrakenPRIM *ptr, KrakenPROP *prop, int *values)
{
  IntPrimPROP *iprop = (IntPrimPROP *)prop;
  IDProperty *idprop;

  KLI_assert(LUXO_prop_type(prop) == PROP_INT);
  KLI_assert(LUXO_prop_array_check(prop) != false);

  if ((idprop = prim_idproperty_check(&prop, ptr))) {
    KLI_assert(idprop->len == LUXO_prop_array_length(ptr, prop) || (prop->flag & PROP_IDPROPERTY));
    if (prop->GetArrayDimensions() == SdfTupleDimensions()) {
      values[0] = LUXO_prop_int_get(ptr, prop);
    } else {
      memcpy(values, IDP_Array(idprop), sizeof(int) * idprop->len);
    }
  } else if (prop->GetArrayDimensions() == SdfTupleDimensions()) {
    values[0] = LUXO_prop_int_get(ptr, prop);
  } else if (iprop->getarray) {
    iprop->getarray(ptr, values);
  } else if (iprop->getarray_ex) {
    iprop->getarray_ex(ptr, prop, values);
  } else {
    prim_prop_int_get_default_array_values(ptr, iprop, values);
  }
}

float LUXO_prop_float_get(KrakenPRIM *ptr, KrakenPROP *prop)
{
  FloatPrimPROP *fprop = (FloatPrimPROP *)prop;
  IDProperty *idprop;

  KLI_assert(LUXO_prop_type(prop) == PROP_FLOAT);
  KLI_assert(LUXO_prop_array_check(prop) == false);

  if ((idprop = prim_idproperty_check(&prop, ptr))) {
    if (idprop->type == IDP_FLOAT) {
      return IDP_Float(idprop);
    }
    return (float)IDP_Double(idprop);
  }
  if (fprop->get) {
    return fprop->get(ptr);
  }
  if (fprop->get_ex) {
    return fprop->get_ex(ptr, prop);
  }
  return fprop->defaultvalue;
}

float LUXO_prop_float_get_index(KrakenPRIM *ptr, KrakenPROP *prop, int index)
{
  float tmp[LUXO_MAX_ARRAY_LENGTH];
  int len = prim_ensure_property_array_length(ptr, prop);

  KLI_assert(LUXO_prop_type(prop) == PROP_FLOAT);
  KLI_assert(LUXO_prop_array_check(prop) != false);
  KLI_assert(index >= 0);
  KLI_assert(index < len);

  if (len <= LUXO_MAX_ARRAY_LENGTH) {
    LUXO_prop_float_get_array(ptr, prop, tmp);
    return tmp[index];
  }
  float *tmparray, value;

  tmparray = static_cast<float *>(MEM_mallocN(sizeof(float) * len, __func__));
  LUXO_prop_float_get_array(ptr, prop, tmparray);
  value = tmparray[index];
  MEM_freeN(tmparray);

  return value;
}

void LUXO_prop_float_set_index(KrakenPRIM *ptr, KrakenPROP *prop, int index, float value)
{
  float tmp[LUXO_MAX_ARRAY_LENGTH];
  int len = prim_ensure_property_array_length(ptr, prop);

  KLI_assert(LUXO_prop_type(prop) == PROP_FLOAT);
  KLI_assert(LUXO_prop_array_check(prop) != false);
  KLI_assert(index >= 0);
  KLI_assert(index < len);

  if (len <= LUXO_MAX_ARRAY_LENGTH) {
    LUXO_prop_float_get_array(ptr, prop, tmp);
    tmp[index] = value;
    LUXO_prop_float_set_array(ptr, prop, tmp);
  } else {
    float *tmparray;

    tmparray = static_cast<float *>(MEM_mallocN(sizeof(float) * len, __func__));
    LUXO_prop_float_get_array(ptr, prop, tmparray);
    tmparray[index] = value;
    LUXO_prop_float_set_array(ptr, prop, tmparray);
    MEM_freeN(tmparray);
  }
}

void LUXO_prop_float_get_array(KrakenPRIM *ptr, KrakenPROP *prop, float *values)
{
  FloatPrimPROP *fprop = (FloatPrimPROP *)prop;
  IDProperty *idprop;
  int i;

  KLI_assert(LUXO_prop_type(prop) == PROP_FLOAT);
  KLI_assert(LUXO_prop_array_check(prop) != false);

  if ((idprop = prim_idproperty_check(&prop, ptr))) {
    KLI_assert(idprop->len == LUXO_prop_array_length(ptr, prop) || (prop->flag & PROP_IDPROPERTY));
    if (prop->GetArrayDimensions() == SdfTupleDimensions()) {
      values[0] = (float)FormFactory(ptr->GetAttribute(prop->GetName()));
    } else if (idprop->subtype == IDP_FLOAT) {
      memcpy(values, IDP_Array(idprop), sizeof(float) * idprop->len);
    } else {
      for (i = 0; i < idprop->len; i++) {
        values[i] = (float)(((double *)IDP_Array(idprop))[i]);
      }
    }
  } else if (prop->GetArrayDimensions() == SdfTupleDimensions()) {
    values[0] = (float)FormFactory(ptr->GetAttribute(prop->GetName()));
  } else if (fprop->getarray) {
    fprop->getarray(ptr, values);
  } else if (fprop->getarray_ex) {
    fprop->getarray_ex(ptr, prop, values);
  } else {
    prim_prop_float_get_default_array_values(ptr, fprop, values);
  }
}

void LUXO_prop_float_set(KrakenPRIM *ptr, KrakenPROP *prop, float value)
{
  FloatPrimPROP *fprop = (FloatPrimPROP *)prop;
  IDProperty *idprop;

  KLI_assert(LUXO_prop_type(prop) == PROP_FLOAT);
  KLI_assert(LUXO_prop_array_check(prop) == false);
  /* useful to check on bad values but set function should clamp */
  // KLI_assert(LUXO_prop_float_clamp(ptr, prop, &value) == 0);

  if ((idprop = prim_idproperty_check(&prop, ptr))) {
    LUXO_prop_float_clamp(ptr, prop, &value);
    if (idprop->type == IDP_FLOAT) {
      IDP_Float(idprop) = value;
    } else {
      IDP_Double(idprop) = value;
    }

    prim_idproperty_touch(idprop);
  } else if (fprop->set) {
    fprop->set(ptr, value);
  } else if (fprop->set_ex) {
    fprop->set_ex(ptr, prop, value);
  } else if (prop->flag & PROP_EDITABLE) {
    IDPropertyTemplate val = {0};
    IDProperty *group;

    LUXO_prop_float_clamp(ptr, prop, &value);

    val.f = value;

    group = LUXO_prim_idprops(ptr, 1);
    if (group) {
      IDP_AddToGroup(group, IDP_New(IDP_FLOAT, &val, prop->GetName()));
    }
  }
}

void LUXO_prop_float_set_array(KrakenPRIM *ptr, KrakenPROP *prop, const float *values)
{
  FloatPrimPROP *fprop = (FloatPrimPROP *)prop;
  IDProperty *idprop;
  int i;

  KLI_assert(LUXO_prop_type(prop) == PROP_FLOAT);
  KLI_assert(LUXO_prop_array_check(prop) != false);

  if ((idprop = prim_idproperty_check(&prop, ptr))) {
    KLI_assert(idprop->len == LUXO_prop_array_length(ptr, prop) || (prop->flag & PROP_IDPROPERTY));
    if (prop->GetArrayDimensions() == SdfTupleDimensions()) {
      if (idprop->type == IDP_FLOAT) {
        IDP_Float(idprop) = values[0];
      } else {
        IDP_Double(idprop) = values[0];
      }
    } else if (idprop->subtype == IDP_FLOAT) {
      memcpy(IDP_Array(idprop), values, sizeof(float) * idprop->len);
    } else {
      for (i = 0; i < idprop->len; i++) {
        ((double *)IDP_Array(idprop))[i] = values[i];
      }
    }

    prim_idproperty_touch(idprop);
  } else if (prop->GetArrayDimensions() == SdfTupleDimensions()) {
    LUXO_prop_float_set(ptr, prop, values[0]);
  } else if (fprop->setarray) {
    fprop->setarray(ptr, values);
  } else if (fprop->setarray_ex) {
    fprop->setarray_ex(ptr, prop, values);
  } else if (prop->flag & PROP_EDITABLE) {
    IDPropertyTemplate val = {0};
    IDProperty *group;

    /* TODO: LUXO_prop_float_clamp_array(ptr, prop, &value); */

    val.array.len = prop->GetTotalArrayLength();
    val.array.type = IDP_FLOAT;

    group = LUXO_prim_idprops(ptr, 1);
    if (group) {
      idprop = IDP_New(IDP_ARRAY, &val, prop->GetName());
      IDP_AddToGroup(group, idprop);
      memcpy(IDP_Array(idprop), values, sizeof(float) * idprop->len);
    }
  }
}

void LUXO_float_set_array(KrakenPRIM *ptr, const char *name, const float *values)
{
  KrakenPROP *prop = LUXO_prim_find_property(ptr, name);

  if (prop) {
    LUXO_prop_float_set_array(ptr, prop, values);
  } else {
    printf("%s: %s.%s not found.\n", __func__, ptr->type->identifier.data(), name);
  }
}

void LUXO_prop_int_set_index(KrakenPRIM *ptr, KrakenPROP *prop, int index, int value)
{
  int tmp[LUXO_MAX_ARRAY_LENGTH];
  int len = prim_ensure_property_array_length(ptr, prop);

  KLI_assert(LUXO_prop_type(prop) == PROP_INT);
  KLI_assert(LUXO_prop_array_check(prop) != false);
  KLI_assert(index >= 0);
  KLI_assert(index < len);

  if (len <= LUXO_MAX_ARRAY_LENGTH) {
    LUXO_prop_int_get_array(ptr, prop, tmp);
    tmp[index] = value;
    LUXO_prop_int_set_array(ptr, prop, tmp);
  } else {
    int *tmparray;

    tmparray = static_cast<int *>(MEM_mallocN(sizeof(int) * len, __func__));
    LUXO_prop_int_get_array(ptr, prop, tmparray);
    tmparray[index] = value;
    LUXO_prop_int_set_array(ptr, prop, tmparray);
    MEM_freeN(tmparray);
  }
}

void LUXO_prop_int_set_array(KrakenPRIM *ptr, KrakenPROP *prop, const int *values)
{
  IntPrimPROP *iprop = (IntPrimPROP *)prop;
  IDProperty *idprop;

  KLI_assert(LUXO_prop_type(prop) == PROP_INT);
  KLI_assert(LUXO_prop_array_check(prop) != false);

  if ((idprop = prim_idproperty_check(&prop, ptr))) {
    KLI_assert(idprop->len == LUXO_prop_array_length(ptr, prop) || (prop->flag & PROP_IDPROPERTY));
    if (prop->GetArrayDimensions() == SdfTupleDimensions()) {
      IDP_Int(idprop) = values[0];
    } else {
      memcpy(IDP_Array(idprop), values, sizeof(int) * idprop->len);
    }

    prim_idproperty_touch(idprop);
  } else if (prop->GetArrayDimensions() == SdfTupleDimensions()) {
    LUXO_prop_int_set(ptr, prop, values[0]);
  } else if (iprop->setarray) {
    iprop->setarray(ptr, values);
  } else if (iprop->setarray_ex) {
    iprop->setarray_ex(ptr, prop, values);
  } else if (prop->flag & PROP_EDITABLE) {
    IDPropertyTemplate val = {0};
    IDProperty *group;

    /* TODO: LUXO_prop_int_clamp_array(ptr, prop, &value); */

    val.array.len = prop->GetTotalArrayLength();
    val.array.type = IDP_INT;

    group = LUXO_prim_idprops(ptr, 1);
    if (group) {
      idprop = IDP_New(IDP_ARRAY, &val, prop->GetName());
      IDP_AddToGroup(group, idprop);
      memcpy(IDP_Array(idprop), values, sizeof(int) * idprop->len);
    }
  }
}

void LUXO_prop_int_set(KrakenPRIM *ptr, KrakenPROP *prop, int value)
{
  IntPrimPROP *iprop = (IntPrimPROP *)prop;
  IDProperty *idprop;

  KLI_assert(LUXO_prop_type(prop) == PROP_INT);
  KLI_assert(LUXO_prop_array_check(prop) == false);
  /* useful to check on bad values but set function should clamp */
  // KLI_assert(LUXO_prop_int_clamp(ptr, prop, &value) == 0);

  if ((idprop = prim_idproperty_check(&prop, ptr))) {
    LUXO_prop_int_clamp(ptr, prop, &value);
    IDP_Int(idprop) = value;
    prim_idproperty_touch(idprop);
  } else if (iprop->set) {
    iprop->set(ptr, value);
  } else if (iprop->set_ex) {
    iprop->set_ex(ptr, prop, value);
  } else if (prop->flag & PROP_EDITABLE) {
    IDPropertyTemplate val = {0};
    IDProperty *group;

    LUXO_prop_int_clamp(ptr, prop, &value);

    val.i = value;

    group = LUXO_prim_idprops(ptr, 1);
    if (group) {
      IDP_AddToGroup(group, IDP_New(IDP_INT, &val, prop->GetName()));
    }
  }
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

RawPropertyType LUXO_prop_raw_type(KrakenPROP *prop)
{
  if (prop->rawtype == PROP_RAW_UNSET) {
    /* this property has no raw access,
     * yet we try to provide a raw type to help building the array. */
    switch (prop->type) {
      case PROP_BOOLEAN:
        return PROP_RAW_BOOLEAN;
      case PROP_INT:
        return PROP_RAW_INT;
      case PROP_FLOAT:
        return PROP_RAW_FLOAT;
      case PROP_ENUM:
        return PROP_RAW_INT;
      default:
        break;
    }
  }
  return prop->rawtype;
}

int LUXO_raw_type_sizeof(RawPropertyType type)
{
  switch (type) {
    case PROP_RAW_CHAR:
      return sizeof(char);
    case PROP_RAW_SHORT:
      return sizeof(short);
    case PROP_RAW_INT:
      return sizeof(int);
    case PROP_RAW_BOOLEAN:
      return sizeof(bool);
    case PROP_RAW_FLOAT:
      return sizeof(float);
    case PROP_RAW_DOUBLE:
      return sizeof(double);
    default:
      return 0;
  }
}

static char stage_prim_state_owner[64];
void LUXO_prim_state_owner_set(const char *name)
{
  if (name) {
    KLI_strncpy(stage_prim_state_owner, name, sizeof(stage_prim_state_owner));
  }
  else {
    stage_prim_state_owner[0] = '\0';
  }
}

const char *LUXO_prim_state_owner_get(void)
{
  if (stage_prim_state_owner[0]) {
    return stage_prim_state_owner;
  }
  return NULL;
}

