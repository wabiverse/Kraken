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

#pragma once

/**
 * @file
 * Window Manager.
 * Making GUI Fly.
 */

#include "USD_listBase.h"
#include "USD_wm_types.h"

#ifdef __cplusplus
#  include <wabi/base/tf/token.h>
#endif /* _cplusplus */

struct kContext;
struct KrakemPRIM;
struct KrakenPROP;
struct wmOperator;
struct wmOperatorType;
struct wmEvent;
struct wmEventHandler;
struct wmEventHandler_Keymap;
struct wmEventHandler_KeymapResult;

#define WM_HANDLER_CONTINUE 0
#define WM_HANDLER_BREAK 1
#define WM_HANDLER_HANDLED 2
#define WM_HANDLER_MODAL 4

#define ISMOUSE_MOTION(event_type) ELEM(event_type, MOUSEMOVE, INBETWEEN_MOUSEMOVE)

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

/** Test whether the event is a NDOF event. */
#define ISNDOF(event_type) ((event_type) >= _NDOF_MIN && (event_type) <= _NDOF_MAX)
#define ISNDOF_BUTTON(event_type) \
  ((event_type) >= _NDOF_BUTTON_MIN && (event_type) <= _NDOF_BUTTON_MAX)

#define ISMOUSE_WHEEL(event_type) ((event_type) >= WHEELUPMOUSE && (event_type) <= WHEELOUTMOUSE)
#define ISMOUSE_GESTURE(event_type) ((event_type) >= MOUSEPAN && (event_type) <= MOUSEROTATE)
#define ISMOUSE_BUTTON(event_type)                                                           \
  ((event_type) == LEFTMOUSE || (event_type) == MIDDLEMOUSE || (event_type) == RIGHTMOUSE || \
   (event_type) == BUTTON4MOUSE || (event_type) == BUTTON5MOUSE ||                           \
   (event_type) == BUTTON6MOUSE || (event_type) == BUTTON7MOUSE)

/** Test whether event type is acceptable as hotkey (excluding modifiers). */
#define ISHOTKEY(event_type)                                                             \
  ((ISKEYBOARD(event_type) || ISMOUSE_BUTTON(event_type) || ISMOUSE_WHEEL(event_type) || \
    ISNDOF_BUTTON(event_type)) &&                                                        \
   (ISKEYMODIFIER(event_type) == false))

void WM_event_do_handlers(struct kContext *C);
void WM_event_free_all(struct wmWindow *win);
void wm_event_free_handler(struct wmEventHandler *handler);
void WM_event_remove_handlers(struct kContext *C, ListBase *handlers);
wmEventHandlerUI *WM_event_add_ui_handler(const struct kContext *C,
                                          std::vector<wmEventHandlerUI *> handlers,
                                          wmUIHandlerFunc handle_fn,
                                          wmUIHandlerRemoveFunc remove_fn,
                                          void *user_data,
                                          const char flag);

void WM_event_add_anchorevent(struct wmWindowManager *wm,
                              struct wmWindow *win,
                              int type,
                              void *customdata);
wmEvent *wm_event_add(struct wmWindow *win, const wmEvent *event_to_add);
void WM_main_add_notifier(unsigned int type, void *reference);
void WM_event_add_notifier_ex(struct wmWindowManager *wm,
                              struct wmWindow *win,
                              uint type,
                              void *reference);
void WM_event_add_notifier(struct kContext *C, uint type, void *reference);
void WM_event_add_mousemove(struct wmWindow *win);

bool WM_event_is_tablet(const wmEvent *event);
int WM_event_drag_threshold(const wmEvent *event);
bool WM_event_drag_test_with_delta(const wmEvent *event, const int drag_delta[2]);
bool WM_event_drag_test(const wmEvent *event, const int prev_xy[2]);

int WM_operator_name_call_ptr(struct kContext *C,
                              struct wmOperatorType *ot,
                              short context,
                              struct KrakenPRIM *properties);

void WM_event_init_from_window(wmWindow *win, wmEvent *event);

int WM_operator_name_call(struct kContext *C,
                          const wabi::TfToken &optoken,
                          short context,
                          struct KrakenPRIM *properties);

void WM_event_do_refresh_wm(struct kContext *C);

void WM_event_get_keymaps_from_handler(struct wmWindowManager *wm,
                                       struct wmWindow *win,
                                       struct wmEventHandler_Keymap *handler,
                                       struct wmEventHandler_KeymapResult *km_result);

bool WM_operator_poll_context(struct kContext *C, struct wmOperatorType *ot, short context);

void WM_reportf(eReportType type, const char *format, ...);
void WM_report(eReportType type, const char *message);
void WM_report_banner_show(void);
