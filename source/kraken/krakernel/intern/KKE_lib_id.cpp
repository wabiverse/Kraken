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

#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CLG_log.h"

#include "MEM_guardedalloc.h"

#include "USD_ID.h"

#include "KLI_utildefines.h"
// #include "KLI_alloca.h"
#include "KLI_kraklib.h"
#include "KLI_string_utils.h"

#include "KKE_utils.h"
#include "KKE_context.h"
#include "KKE_main.h"
#include "KKE_idprop.h"
#include "KKE_idtype.h"
#include "KKE_lib_id.h"

#include "LUXO_access.h"

#include "atomic_ops.h"

static CLG_LogRef LOG = {.identifier = "kke.lib_id"};

IDTypeInfo IDType_ID_LINK_PLACEHOLDER = {
  .id_code = ID_LINK_PLACEHOLDER,
  .id_filter = 0,
  .main_listbase_index = INDEX_ID_NULL,
  .struct_size = sizeof(ID),
  .name = "LinkPlaceholder",
  .name_plural = "link_placeholders",
  .translation_context = "ID",
  .flags = IDTYPE_FLAGS_NO_COPY | IDTYPE_FLAGS_NO_LIBLINKING,
  .asset_type_info = NULL,

  .init_data = NULL,
  .copy_data = NULL,
  .free_data = NULL,
  .make_local = NULL,
  .foreach_id = NULL,
  .foreach_cache = NULL,
  .foreach_path = NULL,
  .owner_pointer_get = NULL,

  .usd_write = NULL,
  .usd_read_data = NULL,
  .usd_read_lib = NULL,
  .usd_read_expand = NULL,

  .usd_read_undo_preserve = NULL,

  .lib_override_apply_post = NULL,
};

void id_us_plus(ID *id)
{
  if (id) {
    id_us_plus_no_lib(id);
    id_lib_extern(id);
  }
}

void id_us_min(ID *id)
{
  if (id) {
    const int limit = ID_FAKE_USERS(id);

    if (id->us <= limit) {
      if (!ID_TYPE_IS_DEPRECATED(GS(id->name))) {
        /* Do not assert on deprecated ID types, we cannot really ensure that their ID refcounting
         * is valid... */
        CLOG_ERROR(&LOG,
                   "ID user decrement error: %s (from '%s'): %d <= %d",
                   id->name,
                   id->lib ? id->lib->filepath_abs : "[Main]",
                   id->us,
                   limit);
      }
      id->us = limit;
    }
    else {
      id->us--;
    }

    if ((id->us == limit) && (id->tag & LIB_TAG_EXTRAUSER)) {
      /* We need an extra user here, but never actually incremented user count for it so far,
       * do it now. */
      id_us_ensure_real(id);
    }
  }
}

void id_us_ensure_real(ID *id)
{
  if (id) {
    const int limit = ID_FAKE_USERS(id);
    id->tag |= LIB_TAG_EXTRAUSER;
    if (id->us <= limit) {
      if (id->us < limit || ((id->us == limit) && (id->tag & LIB_TAG_EXTRAUSER_SET))) {
        CLOG_ERROR(&LOG,
                   "ID user count error: %s (from '%s')",
                   id->name,
                   id->lib ? id->lib->filepath_abs : "[Main]");
      }
      id->us = limit + 1;
      id->tag |= LIB_TAG_EXTRAUSER_SET;
    }
  }
}

void id_us_plus_no_lib(ID *id)
{
  if (id) {
    if ((id->tag & LIB_TAG_EXTRAUSER) && (id->tag & LIB_TAG_EXTRAUSER_SET)) {
      KLI_assert(id->us >= 1);
      /* No need to increase count, just tag extra user as no more set.
       * Avoids annoying & inconsistent +1 in user count. */
      id->tag &= ~LIB_TAG_EXTRAUSER_SET;
    }
    else {
      KLI_assert(id->us >= 0);
      id->us++;
    }
  }
}

void id_lib_extern(ID *id)
{
  if (id && ID_IS_LINKED(id)) {
    KLI_assert(KKE_idtype_idcode_is_linkable(GS(id->name)));
    if (id->tag & LIB_TAG_INDIRECT) {
      id->tag &= ~LIB_TAG_INDIRECT;
      id->flag &= ~LIB_INDIRECT_WEAK_LINK;
      id->tag |= LIB_TAG_EXTERN;
      id->lib->parent = NULL;
    }
  }
}
