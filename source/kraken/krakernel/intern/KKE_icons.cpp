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

#include "KKE_icons.h"
#include "KKE_utils.h"

#include "KLI_rhash.h"
#include "KLI_threads.h"
#include "KLI_assert.h"

#include <wabi/base/tf/hash.h>

/**
 * Only allow non-managed icons to be removed (by Python for eg).
 * Previews & ID's have their own functions to remove icons.
 */
enum
{
  ICON_FLAG_MANAGED = (1 << 0),
};

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
