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
#include "USD_wm_types.h"

#include "WM_window.h"
#include "WM_event_system.h"

#include "ED_screen.h"

#include "UI_interface.h"
#include "UI_resources.h"

#include "KLI_string_utils.h"

#include "KKE_context.h"

#include "interface_intern.h"

KRAKEN_NAMESPACE_BEGIN

/* -------------------------------------------------------------------- */
/** \name Button (#uiBut) State
 * \{ */

static wmOperatorType *g_ot_tool_set_by_id = nullptr;
bool UI_but_is_tool(const uiBut *but)
{
  /* very evil! */
  if (but->optype != nullptr) {
    if (g_ot_tool_set_by_id == nullptr) {
      g_ot_tool_set_by_id = WM_operatortype_find(TfToken("WM_OT_tool_set_by_id"));
    }
    if (but->optype == g_ot_tool_set_by_id) {
      return true;
    }
  }
  return false;
}

bool UI_but_has_tooltip_label(const uiBut *but)
{
  if ((but->drawstr[0] == '\0') && !UI_block_is_popover(but->block)) {
    return UI_but_is_tool(but);
  }
  return false;
}

int ui_but_icon(const uiBut *but)
{
  if (!(but->flag & UI_HAS_ICON)) {
    return ICON_NONE;
  }

  /* Consecutive icons can be toggle between. */
  if (but->drawflag & UI_BUT_ICON_REVERSE) {
    return but->icon - but->iconadd;
  }
  return but->icon + but->iconadd;
}

/** \} */

/* -------------------------------------------------------------------- */
/** \name Button (#uiBut) Text
 * \{ */

size_t UI_but_drawstr_len_without_sep_char(const uiBut *but)
{
  if (but->flag & UI_BUT_HAS_SEP_CHAR) {
    const char *str_sep = strrchr(but->drawstr, UI_SEP_CHAR);
    if (str_sep != nullptr) {
      return (str_sep - but->drawstr);
    }
  }
  return strlen(but->drawstr);
}

size_t UI_but_drawstr_without_sep_char(const uiBut *but, char *str, size_t str_maxlen)
{
  size_t str_len_clip = UI_but_drawstr_len_without_sep_char(but);
  return KLI_strncpy_rlen(str, but->drawstr, min_zz(str_len_clip + 1, str_maxlen));
}

/** \} */

/* -------------------------------------------------------------------- */
/** \name Block (#uiBlock) State
 * \{ */

uiBut *UI_block_active_but_get(const uiBlock *block)
{
  for (auto &but : block->buttons) {
    if (but->active) {
      return but;
    }
  }

  return nullptr;
}

bool UI_block_is_menu(const uiBlock *block)
{
  return (((block->flag & UI_BLOCK_LOOP) != 0) &&
          /* non-menu popups use keep-open, so check this is off */
          ((block->flag & UI_BLOCK_KEEP_OPEN) == 0));
}

bool UI_block_is_popover(const uiBlock *block)
{
  return (block->flag & UI_BLOCK_POPOVER) != 0;
}

/** \} */

/* -------------------------------------------------------------------- */
/** \name Region (#ARegion) State
 * \{ */

uiBut *UI_region_find_active_but(ARegion *region)
{
  for (auto &block : region->uiblocks) {
    uiBut *but = UI_block_active_but_get(block);
    if (but) {
      return but;
    }
  }

  return nullptr;
}

/** \} */

KRAKEN_NAMESPACE_END