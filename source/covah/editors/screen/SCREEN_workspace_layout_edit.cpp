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
 * Editors.
 * Tools for Artists.
 */

#include "ED_defines.h"

#include "UNI_area.h"
#include "UNI_context.h"
#include "UNI_object.h"
#include "UNI_operator.h"
#include "UNI_pixar_utils.h"
#include "UNI_region.h"
#include "UNI_screen.h"
#include "UNI_space_types.h"
#include "UNI_userpref.h"
#include "UNI_window.h"
#include "UNI_wm_types.h"
#include "UNI_workspace.h"

#include "CKE_context.h"
#include "CKE_main.h"
#include "CKE_screen.h"
#include "CKE_workspace.h"

#include "WM_window.h"

#include "ED_screen.h"

WABI_NAMESPACE_BEGIN


static cScreen *screen_fullscreen_find_associated_normal_screen(const Main *cmain, cScreen *screen)
{
  UNIVERSE_FOR_ALL(screen_iter, cmain->screens)
  {
    if ((screen_iter != screen))
    {
      ScrArea *area = screen_iter->areas.at(0);
      if (area)
      {
        return screen_iter;
      }
    }
  }

  return screen;
}


static bool screen_is_used_by_other_window(const wmWindow *win, const cScreen *screen)
{
  return CKE_screen_is_used(screen) && (screen->winid != win->winid);
}


WorkSpaceLayout *ED_workspace_screen_change_ensure_unused_layout(Main *cmain,
                                                                 WorkSpace *workspace,
                                                                 WorkSpaceLayout *layout_new,
                                                                 const WorkSpaceLayout *layout_fallback_base,
                                                                 wmWindow *win)
{
  WorkSpaceLayout *layout_temp = layout_new;
  cScreen *screen_temp = CKE_workspace_layout_screen_get(layout_new);

  screen_temp = screen_fullscreen_find_associated_normal_screen(cmain, screen_temp);
  layout_temp = CKE_workspace_layout_find(workspace, screen_temp);

  // if (screen_is_used_by_other_window(win, screen_temp))
  // {
  //   /* Screen is already used, try to find a free one. */
  //   layout_temp = CKE_workspace_layout_iter_circular(
  //     workspace, layout_new, workspace_change_find_new_layout_cb, NULL, false);
  //   screen_temp = layout_temp ? CKE_workspace_layout_screen_get(layout_temp) : NULL;

  //   if (!layout_temp || screen_is_used_by_other_window(win, screen_temp))
  //   {
  //     /* Fallback solution: duplicate layout. */
  //     layout_temp = ED_workspace_layout_duplicate(cmain, workspace, layout_fallback_base, win);
  //   }
  // }

  return layout_temp;
}


WorkSpaceLayout *ED_workspace_layout_add(cContext *C,
                                         WorkSpace *workspace,
                                         wmWindow *win,
                                         const char *name)
{
  cScreen *screen;
  GfRect2i screen_rect;

  WM_window_screen_rect_calc(win, &screen_rect);
  screen = screen_add(C, name, &screen_rect);

  Main *cmain = CTX_data_main(C);

  return CKE_workspace_layout_add(cmain, workspace, screen, name);
}

WABI_NAMESPACE_END