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

#include "kraken/kraken.h"

#include "USD_operator.h"

#include "WM_event_system.h"

#include "ED_screen.h"

#include "UI_interface.h"

#include "KKE_context.h"

#include "interface_intern.h"

void UI_popup_block_invoke(kContext *C, uiBlockCreateFunc func, void *arg, uiFreeArgFunc arg_free)
{
  // UI_popup_block_invoke_ex(C, func, arg, arg_free, true);
}

void UI_popup_block_ex(kContext *C,
                       uiBlockCreateFunc func,
                       uiBlockHandleFunc popup_func,
                       uiBlockCancelFunc cancel_func,
                       void *arg,
                       wmOperator *op)
{
  wmWindow *window = CTX_wm_window(C);
  uiPopupBlockHandle *handle;

  handle = ui_popup_block_create(C, nullptr, nullptr, func, nullptr, arg, nullptr);
  handle->popup = true;
  handle->retvalue = 1;
  handle->can_refresh = true;

  handle->popup_op = op;
  handle->popup_arg = arg;
  handle->popup_func = popup_func;
  handle->cancel_func = cancel_func;
  // handle->opcontext = opcontext;

  UI_popup_handlers_add(C, window->modalhandlers, handle, 0);
  UI_block_active_only_flagged_buttons(C, handle->region, (uiBlock *)handle->region->uiblocks.first);
  WM_event_add_mousemove(window);
}

bool UI_popup_block_name_exists(const kScreen *screen, const char *name)
{
  LISTBASE_FOREACH(const ARegion *, region, &screen->regions)
  {
    LISTBASE_FOREACH(const uiBlock *, block, &region->uiblocks)
    {
      if (STREQ(block->name, name)) {
        return true;
      }
    }
  }
  return false;
}
