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
 * Editors.
 * Tools for Artists.
 */

#include "ED_defines.h"

#include "USD_area.h"
#include "USD_context.h"
#include "USD_object.h"
#include "USD_operator.h"
#include "USD_pixar_utils.h"
#include "USD_region.h"
#include "USD_screen.h"
#include "USD_space_types.h"
#include "USD_userpref.h"
#include "USD_window.h"
#include "USD_wm_types.h"
#include "USD_workspace.h"

#include "KLI_listbase.h"
#include "KLI_rect.h"

#include "KKE_context.h"
#include "KKE_main.h"
#include "KKE_screen.h"
#include "KKE_workspace.h"

#include "WM_window.hh"

#include "ED_screen.h"


static kScreen *screen_fullscreen_find_associated_normal_screen(const Main *kmain, kScreen *screen)
{
  LISTBASE_FOREACH(kScreen *, screen_iter, &kmain->screens)
  {
    if ((screen_iter != screen) /*&& ELEM(screen_iter->state, SCREENMAXIMIZED, SCREENFULL)*/) {
      ScrArea *area = (ScrArea *)screen_iter->areas.first;
      if (area /*&& area->full == screen*/) {
        return screen_iter;
      }
    }
  }

  return screen;
}


static bool screen_is_used_by_other_window(const wmWindow *win, const kScreen *screen)
{
  return KKE_screen_is_used(screen) && (screen->winid != win->winid);
}


WorkSpaceLayout *ED_workspace_screen_change_ensure_unused_layout(
  Main *kmain,
  WorkSpace *workspace,
  WorkSpaceLayout *layout_new,
  WorkSpaceLayout *layout_fallback_base,
  wmWindow *win)
{
  WorkSpaceLayout *layout_temp = layout_new;
  kScreen *screen_temp = KKE_workspace_layout_screen_get(layout_new);

  screen_temp = screen_fullscreen_find_associated_normal_screen(kmain, screen_temp);
  layout_temp = KKE_workspace_layout_find(workspace, screen_temp);

  // if (screen_is_used_by_other_window(win, screen_temp))
  // {
  //   /* Screen is already used, try to find a free one. */
  //   layout_temp = KKE_workspace_layout_iter_circular(
  //     workspace, layout_new, workspace_change_find_new_layout_cb, NULL, false);
  //   screen_temp = layout_temp ? KKE_workspace_layout_screen_get(layout_temp) : NULL;

  //   if (!layout_temp || screen_is_used_by_other_window(win, screen_temp))
  //   {
  //     /* Fallback solution: duplicate layout. */
  //     layout_temp = ED_workspace_layout_duplicate(kmain, workspace, layout_fallback_base, win);
  //   }
  // }

  return layout_temp;
}


WorkSpaceLayout *ED_workspace_layout_add(kContext *C,
                                         WorkSpace *workspace,
                                         wmWindow *win,
                                         const char *name)
{
  kScreen *screen;
  wabi::GfRect2i screen_rect;

  WM_window_screen_rect_calc(win, &screen_rect);

  rcti from_gf;

  from_gf.xmin = screen_rect.GetMinX();
  from_gf.xmax = screen_rect.GetMaxX();
  from_gf.ymin = screen_rect.GetMinY();
  from_gf.ymax = screen_rect.GetMaxY();

  screen = screen_add(C, name, &from_gf);

  Main *kmain = CTX_data_main(C);

  return KKE_workspace_layout_add(C, kmain, workspace, screen, name);
}
