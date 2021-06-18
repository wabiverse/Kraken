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
 * Window Manager.
 * Making GUI Fly.
 */

#include "WM_operators.h"
#include "WM_window.h"

#include "UNI_screen.h"
#include "UNI_userpref.h"
#include "UNI_window.h"

#include "CKE_context.h"

WABI_NAMESPACE_BEGIN

bool WM_operator_winactive(const cContext &C)
{
  if (CTX_wm_window(C) == NULL)
  {
    return 0;
  }
  return 1;
}

static bool wm_operator_winactive_normal(const cContext &C)
{
  wmWindow win = CTX_wm_window(C);

  if (win == NULL)
  {
    return 0;
  }

  if (!(win->prims.screen))
  {
    return 0;
  }

  // TfToken alignment;
  // win->prims.screen->align.Get(&alignment);
  // if (!(alignment == UsdUITokens->none)) {
  //   return 0;
  // }

  return 1;
}

static void WM_OT_window_close(wmOperatorType *ot)
{
  ot->name = "Close Window";
  ot->idname = "WM_OT_window_close";
  ot->description = "Close the current window";

  ot->exec = wm_window_close_exec;
  ot->poll = WM_operator_winactive;
}

static void WM_OT_window_new(wmOperatorType *ot)
{
  ot->name = "New Window";
  ot->idname = "WM_OT_window_new";
  ot->description = "Create a new window";

  ot->exec = wm_window_new_exec;
  ot->poll = wm_operator_winactive_normal;
}

static void WM_OT_window_new_main(wmOperatorType *ot)
{
  ot->name = "New Main Window";
  ot->idname = "WM_OT_window_new_main";
  ot->description = "Create a new main window with its own workspace and scene selection";

  ot->exec = wm_window_new_main_exec;
  ot->poll = wm_operator_winactive_normal;
}

static void WM_OT_window_fullscreen_toggle(wmOperatorType *ot)
{
  ot->name = "Toggle Window Fullscreen";
  ot->idname = "WM_OT_window_fullscreen_toggle";
  ot->description = "Toggle the current window fullscreen";

  ot->exec = wm_window_fullscreen_toggle_exec;
  ot->poll = WM_operator_winactive;
}

static int wm_exit_covah_exec(const cContext &C, UsdAttribute *UNUSED(op))
{
  wm_exit_schedule_delayed(C);
  return OPERATOR_FINISHED;
}

static int wm_exit_covah_invoke(const cContext &C, UsdAttribute *UNUSED(op), const wmEvent *UNUSED(event))
{
  UserDef uprefs = CTX_data_uprefs(C);

  bool showsave;
  uprefs->showsave.Get(&showsave);

  if (showsave)
  {
    wm_quit_with_optional_confirmation_prompt(C, CTX_wm_window(C));
  }
  else
  {
    wm_exit_schedule_delayed(C);
  }
  return OPERATOR_FINISHED;
}

static void WM_OT_quit_covah(wmOperatorType *ot)
{
  ot->name = "Quit Covah";
  ot->idname = "WM_OT_quit_covah";
  ot->description = "Quit Covah";

  ot->invoke = wm_exit_covah_invoke;
  ot->exec = wm_exit_covah_exec;
}

WABI_NAMESPACE_END