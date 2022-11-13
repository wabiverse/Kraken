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

#pragma once

/**
 * @file
 * Luxo.
 * The Universe Gets Animated.
 */

#include "USD_ID.h"
#include "USD_object.h"

#ifdef __cplusplus
#  include "USD_area.h"
#  include "USD_factory.h"
#  include "USD_operator.h"
#  include "USD_region.h"
#  include "USD_screen.h"
#  include "USD_types.h"
#  include "USD_userpref.h"
#  include "USD_window.h"

#  include <wabi/base/tf/token.h>
#endif /* __cplusplus */

/**
 * This struct is are typically defined in arrays which define an *enum* for RNA,
 * which is used by the RNA API both for user-interface and the Python API.
 */
#ifdef __cplusplus
typedef struct EnumPROP
{
  /** The internal value of the enum, not exposed to users. */
  int value;
  /**
   * Note that identifiers must be unique within the array,
   * by convention they're upper case with underscores for separators.
   * - An empty string is used to define menu separators.
   * - NULL denotes the end of the array of items.
   */
  wabi::TfToken identifier;
  /** Optional icon, typically 'ICON_NONE' */
  int icon;
  /** Name displayed in the interface. */
  const char *name;
  /** Longer description used in the interface. */
  const char *description;
} EnumPROP;
#endif /* __cplusplus */

enum PropScaleTYPE
{
  /** Linear scale (default). */
  PROP_SCALE_LINEAR = 0,
  /**
   * Logarithmic scale
   * - Maximum range: `0 <= x < inf`
   */
  PROP_SCALE_LOG = 1,
  /**
   * Cubic scale.
   * - Maximum range: `-inf < x < inf`
   */
  PROP_SCALE_CUBIC = 2,
};

enum StructFlag
{
  /** Indicates that this struct is an ID struct, and to use reference-counting. */
  STRUCT_ID = (1 << 0),
  STRUCT_ID_REFCOUNT = (1 << 1),
  /** defaults on, indicates when changes in members of a KrakenPRIM should trigger undo steps. */
  STRUCT_UNDO = (1 << 2),

  /* internal flags */
  STRUCT_RUNTIME = (1 << 3),
  /* STRUCT_GENERATED = (1 << 4), */ /* UNUSED */
  STRUCT_FREE_POINTERS = (1 << 5),
  /** Menus and Panels don't need properties */
  STRUCT_NO_IDPROPERTIES = (1 << 6),
  /** e.g. for Operator */
  STRUCT_NO_DATABLOCK_IDPROPERTIES = (1 << 7),
  /** for PropertyGroup which contains pointers to datablocks */
  STRUCT_CONTAINS_DATABLOCK_IDPROPERTIES = (1 << 8),
  /** Added to type-map #BlenderRNA.structs_map */
  STRUCT_PUBLIC_NAMESPACE = (1 << 9),
  /** All sub-types are added too. */
  STRUCT_PUBLIC_NAMESPACE_INHERIT = (1 << 10),
  /**
   * When the #KrakenPRIM.owner_id is NULL, this signifies the property should be accessed
   * without any context (the key-map UI and import/export for example).
   * So accessing the property should not read from the current context to derive values/limits.
   */
  STRUCT_NO_CONTEXT_WITHOUT_OWNER_ID = (1 << 11),
};

typedef enum FunctionFlag
{
  /**
   * Pass ID owning 'self' data
   * (i.e. ptr->owner_id, might be same as self in case data is an ID...).
   */
  FUNC_USE_SELF_ID = (1 << 11),

  /**
   * Do not pass the object (DNA struct pointer) from which it is called,
   * used to define static or class functions.
   */
  FUNC_NO_SELF = (1 << 0),
  /** Pass RNA type, used to define class functions, only valid when #FUNC_NO_SELF is set. */
  FUNC_USE_SELF_TYPE = (1 << 1),

  /* Pass Main, bContext and/or ReportList. */
  FUNC_USE_MAIN = (1 << 2),
  FUNC_USE_CONTEXT = (1 << 3),
  FUNC_USE_REPORTS = (1 << 4),

  /***** Registering of Python subclasses. *****/
  /**
   * This function is part of the registerable class' interface,
   * and can be implemented/redefined in Python.
   */
  FUNC_REGISTER = (1 << 5),
  /** Subclasses can choose not to implement this function. */
  FUNC_REGISTER_OPTIONAL = FUNC_REGISTER | (1 << 6),
  /**
   * If not set, the Python function implementing this call
   * is not allowed to write into data-blocks.
   * Except for WindowManager and Screen currently, see rna_id_write_error() in bpy_rna.c
   */
  FUNC_ALLOW_WRITE = (1 << 12),

  /***** Internal flags. *****/
  /** UNUSED CURRENTLY? ??? */
  FUNC_BUILTIN = (1 << 7),
  /** UNUSED CURRENTLY. ??? */
  FUNC_EXPORT = (1 << 8),
  /** Function has been defined at runtime, not statically in RNA source code. */
  FUNC_RUNTIME = (1 << 9),
  /**
   * UNUSED CURRENTLY? Function owns its identifier and description strings,
   * and has to free them when deleted.
   */
  FUNC_FREE_POINTERS = (1 << 10),
} FunctionFlag;

typedef int (*IteratorSkipFUNC)(struct CollectionPropIT *iter, void *data);

typedef struct ListBaseIterator
{
  struct Link *link;
  int flag;
  IteratorSkipFUNC skip;
} ListBaseIterator;

typedef struct ArrayIT
{
  char *ptr;
  /** Past the last valid pointer, only for comparisons, ignores skipped values. */
  char *endptr;
  /** Will be freed if set. */
  void *free_ptr;
  int itemsize;

  /**
   * Array length with no skip functions applied,
   * take care not to compare against index from animsys or Python indices.
   */
  int length;

  /**
   * Optional skip function,
   * when set the array as viewed by rna can contain only a subset of the members.
   * this changes indices so quick array index lookups are not possible when skip function is used.
   */
  IteratorSkipFUNC skip;
} ArrayIT;

typedef struct CountIT
{
  void *ptr;
  int item;
} CountIT;

#ifdef __cplusplus
typedef struct CollectionPropIT
{
  /* internal */
  KrakenPRIM parent;
  KrakenPRIM builtin_parent;
  struct KrakenPROP *prop;
  union
  {
    ArrayIT array;
    ListBaseIterator listbase;
    CountIT count;
    void *custom;
  } internal;
  int idprop;
  int level;

  /* external */
  KrakenPRIM ptr;
  int valid;
} CollectionPropIT;

typedef struct CollectionPrimLINK
{
  struct CollectionPrimLINK *next, *prev;
  KrakenPRIM ptr;
} CollectionPrimLINK;
#endif /* __cplusplus */

/** Separator for RNA enum items (shown in the UI). */
#define LUXO_ENUM_ITEM_SEPR           \
  {                                   \
    0, wabi::TfToken(), 0, NULL, NULL \
  }
