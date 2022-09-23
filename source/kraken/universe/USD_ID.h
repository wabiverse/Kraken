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
 * KRAKEN Kernel.
 * Purple Underground.
 */

#ifndef KRAKEN_KERNEL_ID_H
#define KRAKEN_KERNEL_ID_H

#include "USD_ID_enums.h"
// #include "USD_defs.h"
#include "USD_listBase.h"

struct FileData;
struct GHash;
struct GPUTexture;
struct ID;
struct Library;
struct PackedFile;
struct UniqueName_Map;

/* Runtime display data */
struct DrawData;
typedef void (*DrawDataInitCb)(struct DrawData *engine_data);
typedef void (*DrawDataFreeCb)(struct DrawData *engine_data);


typedef struct DrawData {
  struct DrawData *next, *prev;
  struct DrawEngineType *engine_type;
  /* Only nested data, NOT the engine data itself. */
  DrawDataFreeCb free;
  /* Accumulated recalc flags, which corresponds to ID->recalc flags. */
  unsigned int recalc;
} DrawData;

typedef struct DrawDataList {
  struct DrawData *first, *last;
} DrawDataList;

typedef struct IDPropertyUIData {
  /** Tooltip / property description pointer. Owned by the IDProperty. */
  char *description;
  /** RNA subtype, used for every type except string properties (PropertySubType). */
  int rna_subtype;

  char _pad[4];
} IDPropertyUIData;

/* IDP_UI_DATA_TYPE_INT */
typedef struct IDPropertyUIDataInt {
  IDPropertyUIData base;
  int *default_array; /* Only for array properties. */
  int default_array_len;
  char _pad[4];

  int min;
  int max;
  int soft_min;
  int soft_max;
  int step;
  int default_value;
} IDPropertyUIDataInt;

/* IDP_UI_DATA_TYPE_FLOAT */
typedef struct IDPropertyUIDataFloat {
  IDPropertyUIData base;
  double *default_array; /* Only for array properties. */
  int default_array_len;
  char _pad[4];

  float step;
  int precision;

  double min;
  double max;
  double soft_min;
  double soft_max;
  double default_value;
} IDPropertyUIDataFloat;

/* IDP_UI_DATA_TYPE_STRING */
typedef struct IDPropertyUIDataString {
  IDPropertyUIData base;
  char *default_value;
} IDPropertyUIDataString;

/* IDP_UI_DATA_TYPE_ID */
typedef struct IDPropertyUIDataID {
  IDPropertyUIData base;
} IDPropertyUIDataID;

typedef struct IDPropertyData {
  void *pointer;
  ListBase group;
  /** NOTE: we actually fit a double into these two 32bit integers. */
  int val, val2;
} IDPropertyData;

typedef struct IDProperty {
  struct IDProperty *next, *prev;
  char type, subtype;
  short flag;
  /** MAX_IDPROP_NAME. */
  char name[64];

  /* saved is used to indicate if this struct has been saved yet.
   * seemed like a good idea as a '_pad' var was needed anyway :) */
  int saved;
  /** NOTE: alignment for 64 bits. */
  IDPropertyData data;

  /* Array length, also (this is important!) string length + 1.
   * the idea is to be able to reuse array realloc functions on strings. */
  int len;

  /* Strings and arrays are both buffered, though the buffer isn't saved. */
  /* totallen is total length of allocated array/string, including a buffer.
   * Note that the buffering is mild; the code comes from python's list implementation. */
  int totallen;

  IDPropertyUIData *ui_data;
} IDProperty;

#define MAX_IDPROP_NAME 64
#define DEFAULT_ALLOC_FOR_NULL_STRINGS 64

/*->type*/
typedef enum eIDPropertyType {
  IDP_STRING = 0,
  IDP_INT = 1,
  IDP_FLOAT = 2,
  /** Array containing int, floats, doubles or groups. */
  IDP_ARRAY = 5,
  IDP_GROUP = 6,
  IDP_ID = 7,
  IDP_DOUBLE = 8,
  IDP_IDPARRAY = 9,
} eIDPropertyType;
#define IDP_NUMTYPES 10

/** Used by some IDP utils, keep values in sync with type enum above. */
enum {
  IDP_TYPE_FILTER_STRING = 1 << 0,
  IDP_TYPE_FILTER_INT = 1 << 1,
  IDP_TYPE_FILTER_FLOAT = 1 << 2,
  IDP_TYPE_FILTER_ARRAY = 1 << 5,
  IDP_TYPE_FILTER_GROUP = 1 << 6,
  IDP_TYPE_FILTER_ID = 1 << 7,
  IDP_TYPE_FILTER_DOUBLE = 1 << 8,
  IDP_TYPE_FILTER_IDPARRAY = 1 << 9,
};

/*->subtype */

/* IDP_STRING */
enum {
  IDP_STRING_SUB_UTF8 = 0, /* default */
  IDP_STRING_SUB_BYTE = 1, /* arbitrary byte array, _not_ null terminated */
};

/*->flag*/
enum {
  /** This IDProp may be statically overridden.
   * Should only be used/be relevant for custom properties. */
  IDP_FLAG_OVERRIDABLE_LIBRARY = 1 << 0,

  /** This collection item IDProp has been inserted in a local override.
   * This is used by internal code to distinguish between library-originated items and
   * local-inserted ones, as many operations are not allowed on the former. */
  IDP_FLAG_OVERRIDELIBRARY_LOCAL = 1 << 1,

  /** This means the property is set but RNA will return false when checking
   * 'RNA_property_is_set', currently this is a runtime flag */
  IDP_FLAG_GHOST = 1 << 7,
};

struct ID
{
  int icon_id;
};

#endif /* KRAKEN_KERNEL_ID_H */