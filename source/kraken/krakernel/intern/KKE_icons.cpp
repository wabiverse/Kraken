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

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <mutex>

#include "CLG_log.h"

#include "MEM_guardedalloc.h"

#include "USD_scene_types.h"
#include "USD_space_types.h"
#include "USD_wm_types.h"
#include "USD_texture_types.h"

#include "KLI_fileops.h"
#include "KLI_rhash.h"
// #include "KLI_linklist_lockfree.h"
#include "KLI_string.h"
#include "KLI_threads.h"
#include "KLI_utildefines.h"
#include "KLI_vector.hh"

#include "KKE_global.h" /* only for G.background test */
#include "KKE_icons.h"
// #include "KKE_studiolight.h"

#include <wabi/base/tf/hash.h>

#include "KLI_sys_types.h" /* for intptr_t support */

#include "GPU_texture.h"

#include "IMB_imbuf.h"
// #include "IMB_imbuf_types.h"
// #include "IMB_thumbs.h"

#include "atomic_ops.h"

/**
 * Only allow non-managed icons to be removed (by Python for eg).
 * Previews & ID's have their own functions to remove icons.
 */
enum
{
  ICON_FLAG_MANAGED = (1 << 0),
};

/* GLOBALS */

static CLG_LogRef LOG = {"kke.icons"};

/* Protected by gIconMutex. */
static RHash *gIcons = nullptr;

/* Protected by gIconMutex. */
static int gNextIconId = 1;

/* Protected by gIconMutex. */
static int gFirstIconId = 1;

static std::mutex gIconMutex;

/* Not mutex-protected! */
static RHash *gCachedPreviews = nullptr;

/* Queue of icons for deferred deletion. */
struct DeferredIconDeleteNode
{
  struct DeferredIconDeleteNode *next;
  int icon_id;
};

/* Protected by gIconMutex. */
static LockfreeLinkList g_icon_delete_queue;

static void icon_free(void *val)
{
  Icon *icon = (Icon *)val;

  if (icon) {
    if (icon->obj_type == ICON_DATA_GEOM) {
      struct Icon_Geom *obj = (struct Icon_Geom *)icon->obj;
      if (obj->mem) {
        /* coords & colors are part of this memory. */
        delete obj->mem;
      } else {
        delete obj->coords;
        delete obj->colors;
      }
      delete icon->obj;
    }

    if (icon->drawinfo_free) {
      icon->drawinfo_free(icon->drawinfo);
    } else if (icon->drawinfo) {
      delete icon->drawinfo;
    }
    delete icon;
  }
}

static void icon_free_data(int icon_id, Icon *icon)
{
  switch (icon->obj_type) {
    case ICON_DATA_ID:
      ((ID *)(icon->obj))->icon_id = 0;
      break;
    case ICON_DATA_IMBUF: {
      // ImBuf *imbuf = (ImBuf *)icon->obj;
      // if (imbuf) {
      //   IMB_freeImBuf(imbuf);
      // }
      break;
    }
    case ICON_DATA_PREVIEW:
      // ((PreviewImage *)(icon->obj))->icon_id = 0;
      break;
    case ICON_DATA_GEOM:
      ((struct Icon_Geom *)(icon->obj))->icon_id = 0;
      break;
    case ICON_DATA_STUDIOLIGHT: {
      // StudioLight *sl = (StudioLight *)icon->obj;
      // if (sl != nullptr) {
      // KKE_studiolight_unset_icon_id(sl, icon_id);
      // }
      break;
    }
    default:
      KLI_assert_unreachable();
  }
}

static Icon *icon_rhash_lookup(int icon_id)
{
  std::scoped_lock lock(gIconMutex);
  return (Icon *)KLI_rhash_lookup(gIcons, POINTER_FROM_INT(icon_id));
}

void KKE_previewimg_freefunc(void *link)
{
  PreviewImage *prv = (PreviewImage *)link;
  if (prv) {
    for (int i = 0; i < NUM_ICON_SIZES; i++) {
      if (prv->rect[i]) {
        MEM_freeN(prv->rect[i]);
      }
      if (prv->gputexture[i]) {
        GPU_texture_free(prv->gputexture[i]);
      }
    }

    MEM_freeN(prv);
  }
}

Icon *KKE_icon_get(const int icon_id)
{
  KLI_assert(KLI_thread_is_main());

  Icon *icon = nullptr;

  icon = icon_rhash_lookup(icon_id);

  if (!icon) {
    CLOG_ERROR(&LOG, "no icon for icon ID: %d", icon_id);
    return nullptr;
  }

  return icon;
}

void KKE_icon_set(const int icon_id, struct Icon *icon)
{
  void **val_p;

  std::scoped_lock lock(gIconMutex);
  if (KLI_rhash_ensure_p(gIcons, POINTER_FROM_INT(icon_id), &val_p)) {
    CLOG_ERROR(&LOG, "icon already set: %d", icon_id);
    return;
  }

  *val_p = icon;
}

void KKE_icons_free()
{
  KLI_assert(KLI_thread_is_main());

  if (gIcons) {
    KLI_rhash_free(gIcons, nullptr, icon_free);
    gIcons = nullptr;
  }

  if (gCachedPreviews) {
    KLI_rhash_free(gCachedPreviews, MEM_freeN, KKE_previewimg_freefunc);
    gCachedPreviews = nullptr;
  }

  // KLI_linklist_lockfree_free(&g_icon_delete_queue, MEM_freeN);
  /* NOTE: We start from a first user-added node. */
  LockfreeLinkNode *node = (&g_icon_delete_queue)->head->next;
  while (node != NULL) {
    LockfreeLinkNode *node_next = node->next;
    MEM_freeN(node);
    node = node_next;
  }
}

void KKE_icons_init(int first_dyn_id)
{
  KLI_assert(KLI_thread_is_main());

  gNextIconId = first_dyn_id;
  gFirstIconId = first_dyn_id;

  if (!gIcons) {
    gIcons = KLI_rhash_int_new(__func__);

    /* lockfree initialized. */
    LockfreeLinkList *queue = &g_icon_delete_queue;
    queue->dummy_node.next = NULL;
    queue->head = queue->tail = &queue->dummy_node;
  }

  if (!gCachedPreviews) {
    gCachedPreviews = KLI_rhash_str_new(__func__);
  }
}
