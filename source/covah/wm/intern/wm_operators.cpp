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

#include "CKE_context.h"

WABI_NAMESPACE_BEGIN

int wm_window_close_exec(const cContext &C, UsdAttribute *UNUSED(op))
{
  // wmWindowManager *wm = CTX_wm_manager(C);
  // wmWindow *win = CTX_wm_window(C);
  // wm_window_close(C, wm, win);
  // return OPERATOR_FINISHED;
  return 0;
}

static void WM_OT_window_close(wmOperatorType *ot)
{
  ot->name = "Close Window";
  ot->idname = "WM_OT_window_close";
  ot->description = "Close the current window";

  // ot->exec = wm_window_close_exec;
  // ot->poll = WM_operator_winactive;
}

void WM_operatortype_append(void (*opfunc)(wmOperatorType *))
{
  // wmOperatorType *ot = wm_operatortype_append__begin();
  // opfunc(ot);
  // wm_operatortype_append__end(ot);
}

void WM_operatortypes_register(void)
{}

WABI_NAMESPACE_END