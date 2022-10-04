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

#include "USD_wm_types.h"
#include "USD_window.h"


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
