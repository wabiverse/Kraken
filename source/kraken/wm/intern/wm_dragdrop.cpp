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
 * Universe.
 * Set the Stage.
 */

#include "MEM_guardedalloc.h"

#include <wabi/usd/sdf/path.h>
#include <wabi/wabi.h>

#include "KLI_compiler_attrs.h"

#include "MEM_guardedalloc.h"

#include "KLI_listbase.h"
#include "KLI_string.h"

#include "KKE_context.h"

#include "WM_inline_tools.h"
#include "WM_window.hh"
#include "WM_dragdrop.h"

#include "USD_ID.h"
#include "USD_ID_enums.h"
#include "USD_listBase.h"
#include "USD_wm_types.h"
#include "USD_space_types.h"
#include "USD_area.h"
#include "USD_region.h"
#include "USD_screen.h"
#include "USD_window.h"

static ListBase dropboxes = {nullptr, nullptr};

struct wmDropBoxMap {
  struct wmDropBoxMap *next, *prev;

  ListBase dropboxes;
  short spaceid, regionid;
  char idname[KMAP_MAX_NAME];
};

void WM_drag_add_local_ID(wmDrag *drag, ID *id, ID *from_parent)
{
  /* Don't drag the same ID twice. */
  LISTBASE_FOREACH(wmDragID *, drag_id, &drag->ids)
  {
    if (drag_id->id == id) {
      if (drag_id->from_parent == nullptr) {
        drag_id->from_parent = from_parent;
      }
      return;
    }
    if (GS(drag_id->id->name) != GS(id->name)) {
      KLI_assert_msg(0, "All dragged IDs must have the same type");
      return;
    }
  }

  /* Add to list. */
  wmDragID *drag_id = MEM_cnew<wmDragID>(__func__);
  drag_id->id = id;
  drag_id->from_parent = from_parent;
  KLI_addtail(&drag->ids, drag_id);
}

wmDrag *WM_event_start_drag(kContext *C,
                            int icon,
                            int type,
                            void *poin,
                            double value,
                            unsigned int flags)
{
  wmDrag *drag = MEM_new<wmDrag>(__func__);

  /* keep track of future multitouch drag too, add a mousepointer id or so */
  /* if multiple drags are added, they're drawn as list */

  drag->flags = static_cast<eWmDragFlags>(flags);
  drag->icon = icon;
  drag->type = type;
  switch (type) {
    case WM_DRAG_PATH:
      KLI_strncpy(drag->path, (const char *)poin, FILE_MAX);
      /* As the path is being copied, free it immediately as `drag` wont "own" the data. */
      if (flags & WM_DRAG_FREE_DATA) {
        delete poin;
      }
      break;
    case WM_DRAG_ID:
      if (poin) {
        WM_drag_add_local_ID(drag, static_cast<ID *>(poin), nullptr);
      }
      break;
    case WM_DRAG_ASSET:
      /* Move ownership of poin to wmDrag. */
      drag->poin = poin;
      drag->flags |= WM_DRAG_FREE_DATA;
      break;
    default:
      drag->poin = poin;
      break;
  }
  drag->value = value;

  return drag;
}

static void wm_dropbox_invoke(kContext *C, wmDrag *drag)
{
  wmWindowManager *wm = CTX_wm_manager(C);

  /* Create a bitmap flag matrix of all currently visible region and area types.
   * Everything that isn't visible in the current window should not prefetch any data. */
  bool area_region_tag[SPACE_TYPE_NUM][RGN_TYPE_NUM] = {{false}};

  for (auto &win : wm->windows) {
    kScreen *screen = WM_window_get_active_screen(VALUE(win));
    for (auto &area : screen->areas) {
      for (auto &region : area->regions) {
        if (region->visible) {
          wabi::TfToken spacetype;
          area->spacetype.Get(&spacetype);
          KLI_assert(wm_spacetype_enum_from_token(spacetype) < SPACE_TYPE_NUM);
          KLI_assert(region->regiontype < RGN_TYPE_NUM);
          area_region_tag[wm_spacetype_enum_from_token(spacetype)][region->regiontype] = true;
        }
      }
    }
  }

  LISTBASE_FOREACH (wmDropBoxMap *, dm, &dropboxes) {
    if (!area_region_tag[dm->spaceid][dm->regionid]) {
      continue;
    }
    LISTBASE_FOREACH (wmDropBox *, drop, &dm->dropboxes) {
      if (drag->drop_state.ui_context) {
        CTX_store_set(C, drag->drop_state.ui_context);
      }

      if (drop->on_drag_start) {
        drop->on_drag_start(C, drag);
      }
      CTX_store_set(C, nullptr);
    }
  }
}

void WM_event_start_prepared_drag(kContext *C, wmDrag *drag)
{
  wmWindowManager *wm = CTX_wm_manager(C);

  wm->drags.push_back(drag);

  wm_dropbox_invoke(C, drag);
}

wmDrag *WM_drag_data_create(kContext *C, int icon, int type, void *poin, double value, uint flags)
{
  wmDrag *drag = MEM_cnew<wmDrag>(__func__);

  /* Keep track of future multi-touch drag too, add a mouse-pointer id or so. */
  /* if multiple drags are added, they're drawn as list */

  drag->flags = static_cast<eWmDragFlags>(flags);
  drag->icon = icon;
  drag->type = type;
  switch (type) {
    case WM_DRAG_PATH:
      KLI_strncpy(drag->path, static_cast<const char *>(poin), FILE_MAX);
      /* As the path is being copied, free it immediately as `drag` won't "own" the data. */
      if (flags & WM_DRAG_FREE_DATA) {
        MEM_freeN(poin);
      }
      break;
    case WM_DRAG_ID:
      if (poin) {
        WM_drag_add_local_ID(drag, static_cast<ID *>(poin), nullptr);
      }
      break;
    case WM_DRAG_ASSET:
    case WM_DRAG_ASSET_CATALOG:
      /* Move ownership of poin to wmDrag. */
      drag->poin = poin;
      drag->flags |= WM_DRAG_FREE_DATA;
      break;
      /* The asset-list case is special: We get multiple assets from context and attach them to the
       * drag item. */
    case WM_DRAG_ASSET_LIST: {
      // todo. handle this with usd.
      // const AssetLibraryReference *asset_library = CTX_wm_asset_library_ref(C);
      // ListBase asset_file_links = CTX_data_collection_get(C, "selected_asset_files");
      // LISTBASE_FOREACH (const CollectionPointerLink *, link, &asset_file_links) {
      //   const FileDirEntry *asset_file = static_cast<const FileDirEntry *>(link->ptr.data);
      //   const AssetHandle asset_handle = {asset_file};
      //   WM_drag_add_asset_list_item(drag, C, asset_library, &asset_handle);
      // }
      // KLI_freelistN(&asset_file_links);
      break;
    }
    default:
      drag->poin = poin;
      break;
  }
  drag->value = value;

  return drag;
}

wmDragAsset *WM_drag_create_asset_data(const AssetHandle *asset,
                                       AssetMetaData *metadata,
                                       const char *path,
                                       int import_type)
{
  wmDragAsset *asset_drag = MEM_new<wmDragAsset>(__func__);

  KLI_strncpy(asset_drag->name, asset->file_data->name, sizeof(asset_drag->name));
  asset_drag->metadata = metadata;
  asset_drag->path = path;
  asset_drag->id_type = static_cast<ID_Type>(asset->file_data->usdtype);
  asset_drag->import_type = import_type;

  return asset_drag;
}

static void wm_drag_free_asset_data(wmDragAsset **asset_data)
{
  MEM_freeN((char *)(*asset_data)->path);
  MEM_SAFE_FREE(*asset_data);
}

void WM_drag_data_free(int dragtype, void *poin)
{
  /* Don't require all the callers to have a nullptr-check, just allow passing nullptr. */
  if (!poin) {
    return;
  }

  /* Not too nice, could become a callback. */
  if (dragtype == WM_DRAG_ASSET) {
    wmDragAsset *asset_data = static_cast<wmDragAsset *>(poin);
    wm_drag_free_asset_data(&asset_data);
  } else {
    MEM_freeN(poin);
  }
}

void WM_drag_free(wmDrag *drag)
{
  MEM_freeN(drag);
}

void WM_drag_free_list(ListBase *lb)
{
  wmDrag *drag;
  while ((drag = static_cast<wmDrag *>(KLI_pophead(lb)))) {
    WM_drag_free(drag);
  }
}
