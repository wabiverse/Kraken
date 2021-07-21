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
 * Copyright 2021, Wabi.
 */

/**
 * @file
 * Universe.
 * Set the Stage.
 */

#pragma once

#include <wabi/usd/sdf/path.h>
#include <wabi/wabi.h>

#include "KLI_string_utils.h"

#include "KKE_context.h"

#include "UNI_window.h"
#include "UNI_wm_types.h"

WABI_NAMESPACE_BEGIN

void WM_drag_add_local_ID(wmDrag *drag, SdfPath id, SdfPath from_parent)
{
  /* Don't drag the same ID twice. */
  UNIVERSE_MUTABLE_FOR_ALL(drag_id, drag->ids)
  {
    if (drag_id->id == id)
    {
      if (drag_id->from_parent.IsEmpty())
      {
        drag_id->from_parent = from_parent;
      }
      return;
    }
    if (drag_id->id.GetName() != id.GetName())
    {
      KLI_assert(!"All dragged IDs must have the same type");
      return;
    }
  }

  /* Add to list. */
  wmDragID *drag_id = new wmDragID();
  drag_id->id = id;
  drag_id->from_parent = from_parent;
  drag->ids.push_back(drag_id);
}

wmDrag *WM_event_start_drag(kContext *C, int icon, int type, void *poin, double value, unsigned int flags)
{
  wmWindowManager *wm = CTX_wm_manager(C);
  wmDrag *drag = new wmDrag();

  /* keep track of future multitouch drag too, add a mousepointer id or so */
  /* if multiple drags are added, they're drawn as list */

  wm->drags.push_back(drag);
  drag->flags = flags;
  drag->icon = icon;
  drag->type = type;
  switch (type)
  {
    case WM_DRAG_PATH:
      KLI_strncpy(drag->path, (const char *)poin, FILE_MAX);
      /* As the path is being copied, free it immediately as `drag` wont "own" the data. */
      if (flags & WM_DRAG_FREE_DATA)
      {
        delete poin;
      }
      break;
    case WM_DRAG_ID:
      if (poin)
      {
        WM_drag_add_local_ID(drag, SdfPath((const char *)poin), SdfPath(""));
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

void WM_drag_free_list(std::vector<wmDrag *> &drags)
{
  for (auto &drag : drags)
  {
    free(drag);
  }

  drags.clear();
}

WABI_NAMESPACE_END