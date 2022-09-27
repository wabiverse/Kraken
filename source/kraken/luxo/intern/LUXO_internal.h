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

#include "KKE_context.h"
#include "KKE_main.h"
#include "KKE_utils.h"

#include "USD_listBase.h"
#include "USD_api.h"
#include "USD_types.h"
#include "USD_wm_types.h"
#include "USD_object.h"

#include "LUXO_runtime.h"
#include "LUXO_types.h"

KRAKEN_NAMESPACE_BEGIN

struct PropertyPRIMOrID {
  KrakenPRIM ptr;

  /** 
   * The PropertyRNA passed as parameter, used to generate that structure's content:
   * - Static RNA: The RNA property (same as `rnaprop`), never NULL.
   * - Runtime RNA: The RNA property (same as `rnaprop`), never NULL.
   * - IDProperty: The IDProperty, never NULL.
   */
  KrakenPROP *rawprop;
  /** 
   * The real RNA property of this property, never NULL:
   * - Static RNA: The rna property, also gives direct access to the data (from any matching
   *               PointerRNA).
   * - Runtime RNA: The rna property, does not directly gives access to the data.
   * - IDProperty: The generic PropertyRNA matching its type.
   */
  KrakenPROP *rnaprop;
  /** 
   * The IDProperty storing the data of this property, may be NULL:
   * - Static RNA: Always NULL.
   * - Runtime RNA: The IDProperty storing the data of that property, may be NULL if never set yet.
   * - IDProperty: The IDProperty, never NULL.
   */
  IDProperty *idprop;
  /** The name of the property. */
  wabi::TfToken identifier;

  /** Whether this property is a 'pure' IDProperty or not. */
  bool is_idprop;
  /** 
   * For runtime RNA properties, whether it is set, defined, or not.
   * WARNING: This DOES take into account the `IDP_FLAG_GHOST` flag, i.e. it matches result of
   *          `RNA_property_is_set`. */
  bool is_set;

  bool is_array;
  uint array_len;
};

typedef int (*PropEnumGetFunc)(struct KrakenPRIM *ptr);
typedef void (*PropEnumSetFunc)(struct KrakenPRIM *ptr, int value);
typedef const EnumPropertyItem *(*PropEnumItemFunc)(struct kContext *C,
                                                    struct KrakenPRIM *ptr,
                                                    struct KrakenPROP *prop,
                                                    bool *r_free);
typedef int (*PropEnumGetFuncEx)(struct KrakenPRIM *ptr, struct KrakenPROP *prop);
typedef void (*PropEnumSetFuncEx)(struct KrakenPRIM *ptr, struct KrakenPROP *prop, int value);

struct EnumPropertyPRIM {
  KrakenPROP property;

  PropEnumGetFunc get;
  PropEnumSetFunc set;
  PropEnumItemFunc item_fn;

  PropEnumGetFuncEx get_ex;
  PropEnumSetFuncEx set_ex;

  const EnumPropertyItem *item;
  int totitem;

  int defaultvalue;
  const char *native_enum_type;
};

void PRIM_def_info(const KrakenSTAGE &kstage);
void PRIM_def_wm(const KrakenSTAGE &kstage);

KRAKEN_NAMESPACE_END