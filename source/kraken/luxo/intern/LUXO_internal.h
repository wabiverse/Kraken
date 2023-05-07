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

#pragma once

#include <stdbool.h>

#include "KKE_context.h"
#include "KKE_main.h"
#include "KKE_utils.h"

#include "USD_wm_types.h"
#include "USD_listBase.h"
#include "USD_api.h"
#include "USD_types.h"
#include "USD_object.h"

#include "LUXO_runtime.h"
#include "LUXO_types.h"

#include "UI_resources.h"

#define LUXO_MAGIC ((int)~0)

struct IDProperty;
struct PropertyPRIMOrID;

typedef struct PropertyPRIMOrID
{
  KrakenPRIM ptr;

  /**
   * The PropertyPRIM passed as parameter, used to generate that structure's content:
   * - Static PRIM: The PRIM property (same as `primprop`), never NULL.
   * - Runtime PRIM: The PRIM property (same as `primprop`), never NULL.
   * - IDProperty: The IDProperty, never NULL.
   */
  KrakenPROP *rawprop;
  /**
   * The real PRIM property of this property, never NULL:
   * - Static PRIM: The prim property, also gives direct access to the data (from any matching
   *               KrakenPRIM).
   * - Runtime PRIM: The prim property, does not directly gives access to the data.
   * - IDProperty: The generic PropertyPRIM matching its type.
   */
  KrakenPROP *primprop;
  /**
   * The IDProperty storing the data of this property, may be NULL:
   * - Static PRIM: Always NULL.
   * - Runtime PRIM: The IDProperty storing the data of that property, may be NULL if never set
   * yet.
   * - IDProperty: The IDProperty, never NULL.
   */
  IDProperty *idprop;
  /** The name of the property. */
  wabi::TfToken identifier;

  /** Whether this property is a 'pure' IDProperty or not. */
  bool is_idprop;
  /**
   * For runtime PRIM properties, whether it is set, defined, or not.
   * WARNING: This DOES take into account the `IDP_FLAG_ANCHOR` flag, i.e. it matches result of
   *          `PRIM_property_is_set`. */
  bool is_set;

  bool is_array;
  uint array_len;
} PropertyPRIMOrID;

/* Function Callbacks */

typedef bool (*PropBooleanGetFunc)(struct KrakenPRIM *ptr);
typedef void (*PropBooleanSetFunc)(struct KrakenPRIM *ptr, bool value);
typedef void (*PropBooleanArrayGetFunc)(struct KrakenPRIM *ptr, bool *values);
typedef void (*PropBooleanArraySetFunc)(struct KrakenPRIM *ptr, const bool *values);

typedef int (*PropIntGetFunc)(struct KrakenPRIM *ptr);
typedef void (*PropIntSetFunc)(struct KrakenPRIM *ptr, int value);
typedef void (*PropIntArrayGetFunc)(struct KrakenPRIM *ptr, int *values);
typedef void (*PropIntArraySetFunc)(struct KrakenPRIM *ptr, const int *values);
typedef void (
  *PropIntRangeFunc)(struct KrakenPRIM *ptr, int *min, int *max, int *softmin, int *softmax);

typedef float (*PropFloatGetFunc)(struct KrakenPRIM *ptr);
typedef void (*PropFloatSetFunc)(struct KrakenPRIM *ptr, float value);
typedef void (*PropFloatArrayGetFunc)(struct KrakenPRIM *ptr, float *values);
typedef void (*PropFloatArraySetFunc)(struct KrakenPRIM *ptr, const float *values);
typedef void (*PropFloatRangeFunc)(struct KrakenPRIM *ptr,
                                   float *min,
                                   float *max,
                                   float *softmin,
                                   float *softmax);

typedef int (*PropEnumGetFunc)(KrakenPRIM *ptr);
typedef void (*PropEnumSetFunc)(KrakenPRIM *ptr, int value);
typedef const EnumPROP *(*PropEnumItemFunc)(kContext *C,
                                            KrakenPRIM *ptr,
                                            KrakenPROP *prop,
                                            bool *r_free);

/* extended versions with PropertyRNA argument */

typedef bool (*PropBooleanGetFuncEx)(struct KrakenPRIM *ptr, struct KrakenPROP *prop);
typedef void (*PropBooleanSetFuncEx)(struct KrakenPRIM *ptr, struct KrakenPROP *prop, bool value);
typedef void (*PropBooleanArrayGetFuncEx)(struct KrakenPRIM *ptr,
                                          struct KrakenPROP *prop,
                                          bool *values);
typedef void (*PropBooleanArraySetFuncEx)(struct KrakenPRIM *ptr,
                                          struct KrakenPROP *prop,
                                          const bool *values);

typedef int (*PropIntGetFuncEx)(struct KrakenPRIM *ptr, struct KrakenPROP *prop);
typedef void (*PropIntSetFuncEx)(struct KrakenPRIM *ptr, struct KrakenPROP *prop, int value);
typedef void (*PropIntArrayGetFuncEx)(struct KrakenPRIM *ptr,
                                      struct KrakenPROP *prop,
                                      int *values);
typedef void (*PropIntArraySetFuncEx)(struct KrakenPRIM *ptr,
                                      struct KrakenPROP *prop,
                                      const int *values);
typedef void (*PropIntRangeFuncEx)(struct KrakenPRIM *ptr,
                                   struct KrakenPROP *prop,
                                   int *min,
                                   int *max,
                                   int *softmin,
                                   int *softmax);

typedef float (*PropFloatGetFuncEx)(struct KrakenPRIM *ptr, struct KrakenPROP *prop);
typedef void (*PropFloatSetFuncEx)(struct KrakenPRIM *ptr, struct KrakenPROP *prop, float value);
typedef void (*PropFloatArrayGetFuncEx)(struct KrakenPRIM *ptr,
                                        struct KrakenPROP *prop,
                                        float *values);
typedef void (*PropFloatArraySetFuncEx)(struct KrakenPRIM *ptr,
                                        struct KrakenPROP *prop,
                                        const float *values);
typedef void (*PropFloatRangeFuncEx)(struct KrakenPRIM *ptr,
                                     struct KrakenPROP *prop,
                                     float *min,
                                     float *max,
                                     float *softmin,
                                     float *softmax);

typedef int (*PropEnumGetFuncEx)(KrakenPRIM *ptr, KrakenPROP *prop);
typedef void (*PropEnumSetFuncEx)(KrakenPRIM *ptr, KrakenPROP *prop, int value);

typedef struct EnumPrimPROP
{
  KrakenPROP property;

  PropEnumGetFunc get;
  PropEnumSetFunc set;
  PropEnumItemFunc item_fn;

  PropEnumGetFuncEx get_ex;
  PropEnumSetFuncEx set_ex;

  const EnumPROP *item;
  int totitem;

  int defaultvalue;
  const char *native_enum_type;
} EnumPrimPROP;

typedef struct BoolPrimPROP
{
  KrakenPROP property;

  PropBooleanGetFunc get;
  PropBooleanSetFunc set;
  PropBooleanArrayGetFunc getarray;
  PropBooleanArraySetFunc setarray;

  PropBooleanGetFuncEx get_ex;
  PropBooleanSetFuncEx set_ex;
  PropBooleanArrayGetFuncEx getarray_ex;
  PropBooleanArraySetFuncEx setarray_ex;

  bool defaultvalue;
  const bool *defaultarray;
} BoolPrimPROP;

typedef struct IntPrimPROP
{
  KrakenPROP property;

  PropIntGetFunc get;
  PropIntSetFunc set;
  PropIntArrayGetFunc getarray;
  PropIntArraySetFunc setarray;
  PropIntRangeFunc range;

  PropIntGetFuncEx get_ex;
  PropIntSetFuncEx set_ex;
  PropIntArrayGetFuncEx getarray_ex;
  PropIntArraySetFuncEx setarray_ex;
  PropIntRangeFuncEx range_ex;

  PropertyScaleType ui_scale_type;
  int softmin, softmax;
  int hardmin, hardmax;
  int step;

  int defaultvalue;
  const int *defaultarray;
} IntPrimPROP;

typedef struct FloatPrimPROP
{
  KrakenPROP property;

  PropFloatGetFunc get;
  PropFloatSetFunc set;
  PropFloatArrayGetFunc getarray;
  PropFloatArraySetFunc setarray;
  PropFloatRangeFunc range;

  PropFloatGetFuncEx get_ex;
  PropFloatSetFuncEx set_ex;
  PropFloatArrayGetFuncEx getarray_ex;
  PropFloatArraySetFuncEx setarray_ex;
  PropFloatRangeFuncEx range_ex;

  PropertyScaleType ui_scale_type;
  float softmin, softmax;
  float hardmin, hardmax;
  float step;
  int precision;

  float defaultvalue;
  const float *defaultarray;
} FloatPrimPROP;

typedef void (*PropCollectionBeginFunc)(struct CollectionPropIT *iter, struct KrakenPRIM *ptr);
typedef void (*PropCollectionNextFunc)(struct CollectionPropIT *iter);
typedef void (*PropCollectionEndFunc)(struct CollectionPropIT *iter);
typedef KrakenPRIM (*PropCollectionGetFunc)(struct CollectionPropIT *iter);
typedef int (*PropCollectionLengthFunc)(struct KrakenPRIM *ptr);
typedef int (*PropCollectionLookupIntFunc)(struct KrakenPRIM *ptr,
                                           int key,
                                           struct KrakenPRIM *r_ptr);
typedef int (*PropCollectionLookupStringFunc)(struct KrakenPRIM *ptr,
                                              const char *key,
                                              struct KrakenPRIM *r_ptr);
typedef int (*PropCollectionAssignIntFunc)(struct KrakenPRIM *ptr,
                                           int key,
                                           const struct KrakenPRIM *assign_ptr);

typedef KrakenPRIM (*PropPointerGetFunc)(struct KrakenPRIM *ptr);
typedef KrakenPRIM *(*PropPointerTypeFunc)(struct KrakenPRIM *ptr);
typedef void (*PropPointerSetFunc)(struct KrakenPRIM *ptr,
                                   const KrakenPRIM value,
                                   struct ReportList *reports);
typedef bool (*PropPointerPollFunc)(struct KrakenPRIM *ptr, const KrakenPRIM value);
typedef bool (*PropPointerPollFuncPy)(struct KrakenPRIM *ptr,
                                      const KrakenPRIM value,
                                      const KrakenPROP *prop);

typedef struct KrakenPrimPROP
{
  KrakenPROP property;

  PropPointerGetFunc get;
  PropPointerSetFunc set;
  PropPointerTypeFunc type_fn;
  /** unlike operators, 'set' can still run if poll fails, used for filtering display. */
  PropPointerPollFunc poll;

  struct KrakenPRIM *type;
} KrakenPrimPROP;

typedef struct CollectionPrimPROP
{
  KrakenPROP property;

  PropCollectionBeginFunc begin;
  PropCollectionNextFunc next;
  PropCollectionEndFunc end; /* optional */
  PropCollectionGetFunc get;
  PropCollectionLengthFunc length;             /* optional */
  PropCollectionLookupIntFunc lookupint;       /* optional */
  PropCollectionLookupStringFunc lookupstring; /* optional */
  PropCollectionAssignIntFunc assignint;       /* optional */

  KrakenPRIM *item_type; /* the type of this item */
} CollectionPrimPROP;

/* internal flags WARNING! 16bits only! */
typedef enum PropertyFlagIntern
{
  PROP_INTERN_BUILTIN = (1 << 0),
  PROP_INTERN_RUNTIME = (1 << 1),
  PROP_INTERN_RAW_ACCESS = (1 << 2),
  PROP_INTERN_RAW_ARRAY = (1 << 3),
  PROP_INTERN_FREE_POINTERS = (1 << 4),
  /* Negative mirror of PROP_PTR_NO_OWNERSHIP, used to prevent automatically setting that one in
   * makesrna when pointer is an ID... */
  PROP_INTERN_PTR_OWNERSHIP_FORCED = (1 << 5),
} PropertyFlagIntern;

/**
 * This function initializes a #PropertyPRIMOrID with all required info, from a given #PropertyPRIM
 * and #KrakenPRIM data. It deals properly with the three cases
 * (static PRIM, runtime PRIM, and #IDProperty).
 * @warning given `ptr` #KrakenPRIM is assumed to be a valid data one here, calling code is
 * responsible to ensure that.
 */
void prim_property_prim_or_id_get(KrakenPROP *prop,
                                  KrakenPRIM *ptr,
                                  PropertyPRIMOrID *r_prop_rna_or_id);
void prim_idproperty_touch(IDProperty *idprop);
IDProperty *prim_idproperty_find(KrakenPRIM *ptr, const TfToken &name);

struct KrakenPRIM *prim_ID_refine(KrakenPRIM *ptr);

void PRIM_def_context(const KrakenSTAGE &kstage);
void PRIM_def_info(const KrakenSTAGE &kstage);
void PRIM_def_wm(const KrakenSTAGE &kstage);
