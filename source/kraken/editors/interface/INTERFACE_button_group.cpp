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

#include "USD_wm_types.h"
#include "USD_factory.h"
#include "USD_operator.h"
#include "USD_userpref.h"

#include "WM_cursors_api.h"
#include "WM_window.h"
#include "WM_event_system.h"
#include "WM_tooltip.h"

#include "ED_screen.h"

#include "UI_interface.h"

#include "KLI_rect.h"

#include "KKE_context.h"

#include "interface_intern.h"

KRAKEN_NAMESPACE_BEGIN

/* -------------------------------------------------------------------- */
/** \name Button Groups
 * \{ */

void UI_block_new_button_group(uiBlock *block, uiButtonGroupFlag flag)
{
  /* Don't create a new group if there is a "lock" on new groups. */
  if (!block->button_groups.empty()) {
    uiButtonGroup *last_button_group = block->button_groups.back();
    if (last_button_group->flag & UI_BUTTON_GROUP_LOCK) {
      return;
    }
  }

  uiButtonGroup *new_group = new uiButtonGroup();
  new_group->buttons.clear();
  new_group->flag = flag;
  block->button_groups.push_back(new_group);
}

/** \} */

KRAKEN_NAMESPACE_END