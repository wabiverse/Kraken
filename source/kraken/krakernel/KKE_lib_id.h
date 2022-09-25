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
 * KRAKEN Kernel.
 * Purple Underground.
 */

#include "KLI_compiler_attrs.h"
#include "KLI_utildefines.h"

/**
 * New ID creation/copying options.
 */
enum {
  /* *** Generic options (should be handled by all ID types copying, ID creation, etc.). *** */
  /** Create data-block outside of any main database -
   * similar to 'localize' functions of materials etc. */
  LIB_ID_CREATE_NO_MAIN = 1 << 0,
  /** Do not affect user refcount of data-blocks used by new one
   * (which also gets zero user-count then).
   * Implies LIB_ID_CREATE_NO_MAIN. */
  LIB_ID_CREATE_NO_USER_REFCOUNT = 1 << 1,
  /** Assume given 'newid' already points to allocated memory for whole data-block
   * (ID + data) - USE WITH CAUTION!
   * Implies LIB_ID_CREATE_NO_MAIN. */
  LIB_ID_CREATE_NO_ALLOCATE = 1 << 2,

  /** Do not tag new ID for update in depsgraph. */
  LIB_ID_CREATE_NO_DEG_TAG = 1 << 8,

  /** Very similar to #LIB_ID_CREATE_NO_MAIN, and should never be used with it (typically combined
   * with #LIB_ID_CREATE_LOCALIZE or #LIB_ID_COPY_LOCALIZE in fact).
   * It ensures that IDs created with it will get the #LIB_TAG_LOCALIZED tag, and uses some
   * specific code in some copy cases (mostly for node trees). */
  LIB_ID_CREATE_LOCAL = 1 << 9,

  /** Create for the depsgraph, when set #LIB_TAG_COPIED_ON_WRITE must be set.
   * Internally this is used to share some pointers instead of duplicating them. */
  LIB_ID_COPY_SET_COPIED_ON_WRITE = 1 << 10,

  /* *** Specific options to some ID types or usages. *** */
  /* *** May be ignored by unrelated ID copying functions. *** */
  /** Object only, needed by make_local code. */
  /* LIB_ID_COPY_NO_PROXY_CLEAR = 1 << 16, */ /* UNUSED */
  /** Do not copy preview data, when supported. */
  LIB_ID_COPY_NO_PREVIEW = 1 << 17,
  /** Copy runtime data caches. */
  LIB_ID_COPY_CACHES = 1 << 18,
  /** Don't copy `id->adt`, used by ID data-block localization routines. */
  LIB_ID_COPY_NO_ANIMDATA = 1 << 19,
  /** Mesh: Reference CD data layers instead of doing real copy - USE WITH CAUTION! */
  LIB_ID_COPY_CD_REFERENCE = 1 << 20,
  /** Do not copy id->override_library, used by ID data-block override routines. */
  LIB_ID_COPY_NO_LIB_OVERRIDE = 1 << 21,
  /** When copying local sub-data (like constraints or modifiers), do not set their "library
   * override local data" flag. */
  LIB_ID_COPY_NO_LIB_OVERRIDE_LOCAL_DATA_FLAG = 1 << 22,

  /* *** XXX Hackish/not-so-nice specific behaviors needed for some corner cases. *** */
  /* *** Ideally we should not have those, but we need them for now... *** */
  /** EXCEPTION! Deep-copy actions used by animation-data of copied ID. */
  LIB_ID_COPY_ACTIONS = 1 << 24,
  /** Keep the library pointer when copying data-block outside of bmain. */
  LIB_ID_COPY_KEEP_LIB = 1 << 25,
  /** EXCEPTION! Deep-copy shape-keys used by copied obdata ID. */
  LIB_ID_COPY_SHAPEKEY = 1 << 26,
  /** EXCEPTION! Specific deep-copy of node trees used e.g. for rendering purposes. */
  LIB_ID_COPY_NODETREE_LOCALIZE = 1 << 27,
  /**
   * EXCEPTION! Specific handling of RB objects regarding collections differs depending whether we
   * duplicate scene/collections, or objects.
   */
  LIB_ID_COPY_RIGID_BODY_NO_COLLECTION_HANDLING = 1 << 28,

  /* *** Helper 'defines' gathering most common flag sets. *** */
  /** Shape-keys are not real ID's, more like local data to geometry IDs. */
  LIB_ID_COPY_DEFAULT = LIB_ID_COPY_SHAPEKEY,

  /** Create a local, outside of bmain, data-block to work on. */
  LIB_ID_CREATE_LOCALIZE = LIB_ID_CREATE_NO_MAIN | LIB_ID_CREATE_NO_USER_REFCOUNT |
                           LIB_ID_CREATE_NO_DEG_TAG,
  /** Generate a local copy, outside of bmain, to work on (used by COW e.g.). */
  LIB_ID_COPY_LOCALIZE = LIB_ID_CREATE_LOCALIZE | LIB_ID_COPY_NO_PREVIEW | LIB_ID_COPY_CACHES |
                         LIB_ID_COPY_NO_LIB_OVERRIDE,
};

void id_lib_extern(struct ID *id);
void id_us_plus_no_lib(struct ID *id);
void id_us_plus(struct ID *id);