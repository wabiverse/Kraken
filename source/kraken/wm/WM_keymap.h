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
 * Window Manager.
 * Making GUI Fly.
 */

#pragma once

#include "kraken/kraken.h"

/* usd-savable wmPrims here */
#include "KLI_utildefines.h"
#include "USD_wm_types.h"

struct IDProperty;

wmKeyMap *WM_keymap_list_find(ListBase *keymaps, const wabi::TfToken &idname, int spaceid, int regionid);

wmKeyMap *WM_keymap_active(const wmWindowManager *wm, wmKeyMap *keymap);

int WM_keymap_item_raw_to_string(const short shift,
                                 const short ctrl,
                                 const short alt,
                                 const short oskey,
                                 const short keymodifier,
                                 const short val,
                                 const short type,
                                 const bool compact,
                                 char *result,
                                 const int result_len);

char *WM_key_event_operator_string(const struct kContext *C,
                                   const wabi::TfToken &opname,
                                   eWmOperatorContext opcontext,
                                   IDProperty *properties,
                                   bool is_strict,
                                   char *result,
                                   int result_len);

int WM_keymap_item_to_string(const struct wmKeyMapItem *kmi,
                             const bool compact,
                             char *result,
                             const int result_len);

const char *WM_key_event_string(const short type, const bool compact);

