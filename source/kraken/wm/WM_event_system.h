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

#include <wabi/usd/usd/attribute.h>
#include <wabi/wabi.h>

#include "KKE_context.h"
#include "KKE_idtype.h"

#include "USD_object.h"

#include "WM_operators.h"
#include "WM_keymap.h"

KRAKEN_NAMESPACE_BEGIN

struct ARegion;
struct ScrArea;
struct wmEvent;

#define WM_HANDLER_CONTINUE 0
#define WM_HANDLER_BREAK 1
#define WM_HANDLER_HANDLED 2
#define WM_HANDLER_MODAL 4

/* test whether the event is a key on the keyboard */
#define ISKEYBOARD(event_type)                           \
  (((event_type) >= 0x0020 && (event_type) <= 0x00ff) || \
   ((event_type) >= 0x012c && (event_type) <= 0x0143))

/* test whether the event is a modifier key */
#define ISKEYMODIFIER(event_type)                                           \
  (((event_type) >= EVT_LEFTCTRLKEY && (event_type) <= EVT_LEFTSHIFTKEY) || \
   (event_type) == EVT_OSKEY)

/* test whether the event is a mouse button */
#define ISMOUSE(event_type) \
  (((event_type) >= LEFTMOUSE && (event_type) <= BUTTON7MOUSE) || (event_type) == MOUSESMARTZOOM)

#define ISMOUSE_WHEEL(event_type) ((event_type) >= WHEELUPMOUSE && (event_type) <= WHEELOUTMOUSE)
#define ISMOUSE_GESTURE(event_type) ((event_type) >= MOUSEPAN && (event_type) <= MOUSEROTATE)
#define ISMOUSE_BUTTON(event_type)                                                           \
  ((event_type) == LEFTMOUSE || (event_type) == MIDDLEMOUSE || (event_type) == RIGHTMOUSE || \
   (event_type) == BUTTON4MOUSE || (event_type) == BUTTON5MOUSE ||                           \
   (event_type) == BUTTON6MOUSE || (event_type) == BUTTON7MOUSE)

void wm_event_free_handler(wmEventHandler *handler);
void WM_event_remove_handlers(kContext *C, std::vector<wmEventHandler *> handlers);
wmEventHandlerUI *WM_event_add_ui_handler(const kContext *C,
                                          std::vector<wmEventHandler *> handlers,
                                          wmUIHandlerFunc handle_fn,
                                          wmUIHandlerRemoveFunc remove_fn,
                                          void *user_data,
                                          const char flag);

void WM_event_add_anchorevent(wmWindowManager *wm, wmWindow *win, int type, void *customdata);
wmEvent *wm_event_add(wmWindow *win, const wmEvent *event_to_add);
void WM_main_add_notifier(unsigned int type, void *reference);
void WM_event_add_notifier_ex(wmWindowManager *wm, wmWindow *win, uint type, void *reference);
void WM_event_add_notifier(kContext *C, uint type, void *reference);
void WM_event_add_mousemove(wmWindow *win);

bool WM_event_is_tablet(const wmEvent *event);
int WM_event_drag_threshold(const wmEvent *event);
bool WM_event_drag_test_with_delta(const wmEvent *event, const int drag_delta[2]);
bool WM_event_drag_test(const wmEvent *event, const int prev_xy[2]);

int WM_operator_name_call_ptr(kContext *C,
                              wmOperatorType *ot,
                              short context,
                              KrakenPRIM *properties);

void WM_event_init_from_window(wmWindow *win, wmEvent *event);

int WM_operator_name_call(kContext *C,
                          const TfToken &optoken,
                          short context,
                          KrakenPRIM *properties);

void WM_event_do_refresh_wm(kContext *C);

void WM_event_get_keymaps_from_handler(wmWindowManager *wm,
                                       wmWindow *win,
                                       wmEventHandler_Keymap *handler,
                                       wmEventHandler_KeymapResult *km_result);

bool WM_operator_poll_context(kContext *C, wmOperatorType *ot, short context);

void WM_report_banner_show(void);

KRAKEN_NAMESPACE_END