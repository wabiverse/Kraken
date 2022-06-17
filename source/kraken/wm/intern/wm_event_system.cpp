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

#include "WM_event_system.h"
#include "WM_cursors_api.h"
#include "WM_debug_codes.h"
#include "WM_dragdrop.h"
#include "WM_operators.h"
#include "WM_window.h"

#include "UNI_area.h"
#include "UNI_factory.h"
#include "UNI_operator.h"
#include "UNI_region.h"
#include "UNI_screen.h"
#include "UNI_userpref.h"
#include "UNI_window.h"

#include "KKE_context.h"

#include "KLI_assert.h"
#include "KLI_string_utils.h"
#include "KLI_time.h"

#include "ED_screen.h"

#include <wabi/base/tf/stringUtils.h>

WABI_NAMESPACE_BEGIN


static bool wm_test_duplicate_notifier(wmWindowManager *wm, uint type, void *reference)
{
  for (auto const &note : wm->notifier_queue) {
    if ((note->category | note->data | note->subtype | note->action) == type &&
        note->reference == reference) {
      return true;
    }
  }

  return false;
}

bool WM_event_is_tablet(const wmEvent *event)
{
  return (event->tablet.active != EVT_TABLET_NONE);
}

int WM_event_drag_threshold(const wmEvent *event)
{
  int drag_threshold;
  if (ISMOUSE(event->prevtype)) {
    KLI_assert(event->prevtype != MOUSEMOVE);
    if (WM_event_is_tablet(event)) {
      drag_threshold = UI_DRAG_THRESHOLD_TABLET;
    } else {
      drag_threshold = UI_DRAG_THRESHOLD_MOUSE;
    }
  } else {
    /* Typically keyboard, could be NDOF button or other less common types. */
    drag_threshold = UI_DRAG_THRESHOLD;
  }
  return drag_threshold * UI_DPI_FAC;
}

bool WM_event_drag_test_with_delta(const wmEvent *event, const int drag_delta[2])
{
  const int drag_threshold = WM_event_drag_threshold(event);
  return abs(drag_delta[0]) > drag_threshold || abs(drag_delta[1]) > drag_threshold;
}

bool WM_event_drag_test(const wmEvent *event, const int prev_xy[2])
{
  const int drag_delta[2] = {
    GET_X(prev_xy) - GET_X(event->mouse_pos),
    GET_Y(prev_xy) - GET_Y(event->mouse_pos),
  };
  return WM_event_drag_test_with_delta(event, drag_delta);
}


void WM_main_add_notifier(unsigned int type, void *reference)
{
  wmWindowManager *wm = G.main->wm.at(0);

  if (!wm || wm_test_duplicate_notifier(wm, type, reference)) {
    return;
  }

  wmNotifier *note = new wmNotifier();

  wm->notifier_queue.push_back(note);

  note->category = type & NOTE_CATEGORY;
  note->data = type & NOTE_DATA;
  note->subtype = type & NOTE_SUBTYPE;
  note->action = type & NOTE_ACTION;

  note->reference = reference;

  note->Push();
}


void WM_event_add_notifier_ex(wmWindowManager *wm, wmWindow *win, uint type, void *reference)
{
  if (wm_test_duplicate_notifier(wm, type, reference)) {
    return;
  }

  wmNotifier *note = new wmNotifier();
  wm->notifier_queue.push_back(note);

  note->window = win;

  note->category = type & NOTE_CATEGORY;
  note->data = type & NOTE_DATA;
  note->subtype = type & NOTE_SUBTYPE;
  note->action = type & NOTE_ACTION;

  note->reference = reference;

  note->Push();
}

void WM_event_add_notifier(kContext *C, uint type, void *reference)
{
  WM_event_add_notifier_ex(CTX_wm_manager(C), CTX_wm_window(C), type, reference);
}


wmEvent *wm_event_add_ex(wmWindow *win,
                         const wmEvent *event_to_add,
                         const wmEvent *event_to_add_after)
{
  wmEvent *event = new wmEvent();

  *event = *event_to_add;

  if (event_to_add_after == NULL) {
    win->event_queue.push_back(event);
  } else {
    wmEventQueue::iterator it;
    for (it = win->event_queue.begin(); it != win->event_queue.end(); ++it) {
      if ((*it) == event_to_add_after) {
        it++;
        win->event_queue.insert(it, event);
      }
    }
  }
  return event;
}


void WM_event_add_mousemove(wmWindow *win)
{
  win->addmousemove = 1;
}


wmEvent *wm_event_add(wmWindow *win, const wmEvent *event_to_add)
{
  return wm_event_add_ex(win, event_to_add, NULL);
}

static void wm_event_free_last(wmWindow *win)
{
  win->event_queue.pop_back();
}

static wmEvent *wm_event_add_mousemove(wmWindow *win, wmEvent *event)
{
  wmEvent *event_last = win->event_queue.back();

  if (event_last && event_last->type == MOUSEMOVE) {
    event_last->type = INBETWEEN_MOUSEMOVE;
  }

  wmEvent *event_new = wm_event_add(win, event);
  if (event_last == NULL) {
    event_last = win->eventstate;
  }

  event_new->prev_mouse_pos = event_last->mouse_pos;
  return event_new;
}


static wmWindow *wm_event_cursor_other_windows(wmWindowManager *wm, wmWindow *win, wmEvent *event)
{
  GfVec2i mval = event->mouse_pos;

  if (wm->windows.size() <= 1) {
    return nullptr;
  }

  /* In order to use window size and mouse position (pixels), we have to use a WM function. */

  /* check if outside, include top window bar... */
  if (mval[0] < 0 || mval[1] < 0 || mval[0] > WM_window_pixels_x(win) ||
      mval[1] > WM_window_pixels_y(win) + 30) {
    wmWindow *win_other;
    if (WM_window_find_under_cursor(wm, win, win, mval, &win_other, &mval)) {
      event->mouse_pos = mval;
      return win_other;
    }
  }
  return nullptr;
}


static wmEvent *wm_event_add_trackpad(wmWindow *win, const wmEvent *event, int deltax, int deltay)
{
  wmEvent *event_last = win->event_queue.back();
  if (event_last && event_last->type == event->type) {
    deltax += GET_X(event_last->mouse_pos) - GET_X(event_last->prev_mouse_pos);
    deltay += GET_Y(event_last->mouse_pos) - GET_Y(event_last->prev_mouse_pos);

    wm_event_free_last(win);
  }

  wmEvent *event_new = wm_event_add(win, event);
  GET_X(event_new->prev_mouse_pos) = GET_X(event_new->mouse_pos) - deltax;
  GET_Y(event_new->prev_mouse_pos) = GET_Y(event_new->mouse_pos) - deltay;

  return event_new;
}

static float wm_pressure_curve(float pressure)
{
  if (UI_PRESSURE_THRESHOLD_MAX != 0.0f) {
    pressure /= UI_PRESSURE_THRESHOLD_MAX;
  }

  CLAMP(pressure, 0.0f, 1.0f);

  if (UI_PRESSURE_SOFTNESS != 0.0f) {
    pressure = powf(pressure, powf(4.0f, -UI_PRESSURE_SOFTNESS));
  }

  return pressure;
}

/* -------------------------------------------------------------------- */
/** \name Anchor Event Conversion
 * \{ */

static int convert_key(eAnchorKey key)
{
  if (key >= AnchorKeyA && key <= AnchorKeyZ) {
    return (EVT_AKEY + ((int)key - AnchorKeyA));
  }
  if (key >= AnchorKey0 && key <= AnchorKey9) {
    return (EVT_ZEROKEY + ((int)key - AnchorKey0));
  }
  if (key >= AnchorKeyNumpad0 && key <= AnchorKeyNumpad9) {
    return (EVT_PAD0 + ((int)key - AnchorKeyNumpad0));
  }
  if (key >= AnchorKeyF1 && key <= AnchorKeyF24) {
    return (EVT_F1KEY + ((int)key - AnchorKeyF1));
  }

  switch (key) {
    case AnchorKeyBackSpace:
      return EVT_BACKSPACEKEY;
    case AnchorKeyTab:
      return EVT_TABKEY;
    case AnchorKeyLinefeed:
      return EVT_LINEFEEDKEY;
    case AnchorKeyClear:
      return 0;
    case AnchorKeyEnter:
      return EVT_RETKEY;

    case AnchorKeyEsc:
      return EVT_ESCKEY;
    case AnchorKeySpace:
      return EVT_SPACEKEY;
    case AnchorKeyQuote:
      return EVT_QUOTEKEY;
    case AnchorKeyComma:
      return EVT_COMMAKEY;
    case AnchorKeyMinus:
      return EVT_MINUSKEY;
    case AnchorKeyPlus:
      return EVT_PLUSKEY;
    case AnchorKeyPeriod:
      return EVT_PERIODKEY;
    case AnchorKeySlash:
      return EVT_SLASHKEY;

    case AnchorKeySemicolon:
      return EVT_SEMICOLONKEY;
    case AnchorKeyEqual:
      return EVT_EQUALKEY;

    case AnchorKeyLeftBracket:
      return EVT_LEFTBRACKETKEY;
    case AnchorKeyRightBracket:
      return EVT_RIGHTBRACKETKEY;
    case AnchorKeyBackslash:
      return EVT_BACKSLASHKEY;
    case AnchorKeyAccentGrave:
      return EVT_ACCENTGRAVEKEY;

    case AnchorKeyLeftShift:
      return EVT_LEFTSHIFTKEY;
    case AnchorKeyRightShift:
      return EVT_RIGHTSHIFTKEY;
    case AnchorKeyLeftControl:
      return EVT_LEFTCTRLKEY;
    case AnchorKeyRightControl:
      return EVT_RIGHTCTRLKEY;
    case AnchorKeyOS:
      return EVT_OSKEY;
    case AnchorKeyLeftAlt:
      return EVT_LEFTALTKEY;
    case AnchorKeyRightAlt:
      return EVT_RIGHTALTKEY;
    case AnchorKeyApp:
      return EVT_APPKEY;

    case AnchorKeyCapsLock:
      return EVT_CAPSLOCKKEY;
    case AnchorKeyNumLock:
      return 0;
    case AnchorKeyScrollLock:
      return 0;

    case AnchorKeyLeftArrow:
      return EVT_LEFTARROWKEY;
    case AnchorKeyRightArrow:
      return EVT_RIGHTARROWKEY;
    case AnchorKeyUpArrow:
      return EVT_UPARROWKEY;
    case AnchorKeyDownArrow:
      return EVT_DOWNARROWKEY;

    case AnchorKeyPrintScreen:
      return 0;
    case AnchorKeyPause:
      return EVT_PAUSEKEY;

    case AnchorKeyInsert:
      return EVT_INSERTKEY;
    case AnchorKeyDelete:
      return EVT_DELKEY;
    case AnchorKeyHome:
      return EVT_HOMEKEY;
    case AnchorKeyEnd:
      return EVT_ENDKEY;
    case AnchorKeyUpPage:
      return EVT_PAGEUPKEY;
    case AnchorKeyDownPage:
      return EVT_PAGEDOWNKEY;

    case AnchorKeyNumpadPeriod:
      return EVT_PADPERIOD;
    case AnchorKeyNumpadEnter:
      return EVT_PADENTER;
    case AnchorKeyNumpadPlus:
      return EVT_PADPLUSKEY;
    case AnchorKeyNumpadMinus:
      return EVT_PADMINUS;
    case AnchorKeyNumpadAsterisk:
      return EVT_PADASTERKEY;
    case AnchorKeyNumpadSlash:
      return EVT_PADSLASHKEY;

    case AnchorKeyGrLess:
      return EVT_GRLESSKEY;

    case AnchorKeyMediaPlay:
      return EVT_MEDIAPLAY;
    case AnchorKeyMediaStop:
      return EVT_MEDIASTOP;
    case AnchorKeyMediaFirst:
      return EVT_MEDIAFIRST;
    case AnchorKeyMediaLast:
      return EVT_MEDIALAST;

    default:
      return EVT_UNKNOWNKEY; /* AnchorKeyUnknown */
  }
}

static void wm_eventemulation(wmEvent *event, bool test_only)
{
  static int emulating_event = EVENT_NONE;

  /* Middle-mouse emulation. */
  if (UI_FLAG & USER_TWOBUTTONMOUSE) {

    if (event->type == LEFTMOUSE) {
      short *mod = (
#if !defined(WIN32) && defined(USER_EMU_MMB_MOD_OSKEY)
        (UI_MOUSE_EMULATE_3BUTTON_MODIFIER == USER_EMU_MMB_MOD_OSKEY) ? &event->oskey : &event->alt
#else
        /* Disable for WIN32 for now because it accesses the start menu. */
        &event->alt
#endif
      );

      if (event->val == KM_PRESS) {
        if (*mod) {
          *mod = 0;
          event->type = MIDDLEMOUSE;

          if (!test_only) {
            emulating_event = MIDDLEMOUSE;
          }
        }
      } else if (event->val == KM_RELEASE) {
        /* Only send middle-mouse release if emulated. */
        if (emulating_event == MIDDLEMOUSE) {
          event->type = MIDDLEMOUSE;
          *mod = 0;
        }

        if (!test_only) {
          emulating_event = EVENT_NONE;
        }
      }
    }
  }

  /* Numpad emulation. */
  if (UI_FLAG & USER_NONUMPAD) {
    switch (event->type) {
      case EVT_ZEROKEY:
        event->type = EVT_PAD0;
        break;
      case EVT_ONEKEY:
        event->type = EVT_PAD1;
        break;
      case EVT_TWOKEY:
        event->type = EVT_PAD2;
        break;
      case EVT_THREEKEY:
        event->type = EVT_PAD3;
        break;
      case EVT_FOURKEY:
        event->type = EVT_PAD4;
        break;
      case EVT_FIVEKEY:
        event->type = EVT_PAD5;
        break;
      case EVT_SIXKEY:
        event->type = EVT_PAD6;
        break;
      case EVT_SEVENKEY:
        event->type = EVT_PAD7;
        break;
      case EVT_EIGHTKEY:
        event->type = EVT_PAD8;
        break;
      case EVT_NINEKEY:
        event->type = EVT_PAD9;
        break;
      case EVT_MINUSKEY:
        event->type = EVT_PADMINUS;
        break;
      case EVT_EQUALKEY:
        event->type = EVT_PADPLUSKEY;
        break;
      case EVT_BACKSLASHKEY:
        event->type = EVT_PADSLASHKEY;
        break;
    }
  }
}

static const wmTabletData wm_event_tablet_data_default = {
  /*.active==*/
  EVT_TABLET_NONE,
  /*.pressure==*/
  1.0f,
  /*.x_tilt==*/
  0.0f,
  /*.y_tilt==*/
  0.0f,
  /*.is_motion_absolute==*/
  false,
};

void WM_event_tablet_data_default_set(wmTabletData *tablet_data)
{
  *tablet_data = wm_event_tablet_data_default;
}

static void wm_tablet_data_from_anchor(const AnchorTabletData *tablet_data, wmTabletData *wmtab)
{
  if ((tablet_data != NULL) && tablet_data->Active != AnchorTabletModeNone) {
    wmtab->active = static_cast<eWmTabletEventType>(tablet_data->Active);
    wmtab->pressure = wm_pressure_curve(tablet_data->Pressure);
    wmtab->x_tilt = tablet_data->Xtilt;
    wmtab->y_tilt = tablet_data->Ytilt;
    wmtab->is_motion_absolute = true;
  } else {
    *wmtab = wm_event_tablet_data_default;
  }
}

/**
 * Copy the current state to the previous event state. */
static void wm_event_prev_values_set(wmEvent *event, wmEvent *event_state)
{
  event->prevval = event_state->prevval = event_state->val;
  event->prevtype = event_state->prevtype = event_state->type;
}

static void wm_event_prev_click_set(wmEvent *event, wmEvent *event_state)
{
  event->prevclicktime = event_state->prevclicktime = PIL_check_seconds_timer();
  event->prevclickx = event_state->prevclickx = GET_X(event_state->mouse_pos);
  event->prevclicky = event_state->prevclicky = GET_Y(event_state->mouse_pos);
}

static bool wm_event_is_double_click(const wmEvent *event)
{
  if ((event->type == event->prevtype) && (event->prevval == KM_RELEASE) &&
      (event->val == KM_PRESS)) {
    if (ISMOUSE(event->type) && WM_event_drag_test(event, &event->prevclickx)) {
      /* Pass. */
    } else {
      if ((PIL_check_seconds_timer() - event->prevclicktime) * 1000 < UI_DOUBLE_CLICK_TIME) {
        return true;
      }
    }
  }

  return false;
}

void WM_event_init_from_window(wmWindow *win, wmEvent *event)
{
  *event = *(win->eventstate);
}

wmEventHandlerUI *WM_event_add_ui_handler(const kContext *C,
                                          std::vector<wmEventHandler *> handlers,
                                          wmUIHandlerFunc handle_fn,
                                          wmUIHandlerRemoveFunc remove_fn,
                                          void *user_data,
                                          const char flag)
{
  wmEventHandlerUI *handler = new wmEventHandlerUI();
  handler->type = WM_HANDLER_TYPE_UI;
  handler->handle_fn = handle_fn;
  handler->remove_fn = remove_fn;
  handler->user_data = user_data;
  if (C) {
    handler->context.area = CTX_wm_area(C);
    handler->context.region = CTX_wm_region(C);
    handler->context.menu = CTX_wm_menu(C);
  } else {
    handler->context.area = NULL;
    handler->context.region = NULL;
    handler->context.menu = NULL;
  }

  KLI_assert((flag & WM_HANDLER_DO_FREE) == 0);
  handler->flag = flag;

  handlers.insert(handlers.begin(), handler);

  return handler;
}

void WM_event_add_anchorevent(wmWindowManager *wm, wmWindow *win, int type, void *customdata)
{
  wmEvent event, *event_state = win->eventstate;

  event = *event_state;
  event.is_repeat = false;

  event.prevtype = event.type;
  event.prevval = event.val;

#ifndef NDEBUG
  if ((event_state->type || event_state->val) &&
      !(ISMOUSE_BUTTON(event_state->type) || ISKEYBOARD(event_state->type) ||
        (event_state->type == EVENT_NONE))) {
    TF_WARN("WM -- Non-keyboard/mouse button found in 'win->eventstate->type = %d'",
            event_state->type);
  }
  if ((event_state->prevtype || event_state->prevval) && /* Ignore cleared event state. */
      !(ISMOUSE_BUTTON(event_state->prevtype) || ISKEYBOARD(event_state->prevtype) ||
        (event_state->type == EVENT_NONE))) {
    TF_WARN("WM -- Non-keyboard/mouse button found in 'win->eventstate->prevtype = %d'",
            event_state->prevtype);
  }
#endif

  switch (type) {
    case AnchorEventTypeCursorMove: {
      AnchorEventCursorData *cd = (AnchorEventCursorData *)customdata;
      event.mouse_pos = GfVec2i(cd->x, cd->y);

      event.type = MOUSEMOVE;
      {
        wmEvent *event_new = wm_event_add_mousemove(win, &event);
        event_state->mouse_pos = event_new->mouse_pos;
      }

      wmWindow *win_other = wm_event_cursor_other_windows(wm, win, &event);
      if (win_other) {
        wmEvent event_other = *win_other->eventstate;

        event_other.prevtype = event_other.type;
        event_other.prevval = event_other.val;

        event_other.mouse_pos = event.mouse_pos;
        event_other.type = MOUSEMOVE;
        {
          wmEvent *event_new = wm_event_add_mousemove(win_other, &event_other);
          win_other->eventstate->mouse_pos = event_new->mouse_pos;
          win_other->eventstate->tablet.is_motion_absolute = event_new->tablet.is_motion_absolute;
        }
      }

      break;
    }

    case AnchorEventTypeTrackpad: {
      AnchorEventTrackpadData *pd = (AnchorEventTrackpadData *)customdata;
      switch (pd->subtype) {
        case ANCHOR_TrackpadEventMagnify:
          event.type = MOUSEZOOM;
          pd->deltaX = -pd->deltaX;
          pd->deltaY = -pd->deltaY;
          break;
        case ANCHOR_TrackpadEventSmartMagnify:
          event.type = MOUSESMARTZOOM;
          break;
        case ANCHOR_TrackpadEventRotate:
          event.type = MOUSEROTATE;
          break;
        case ANCHOR_TrackpadEventScroll:
        default:
          event.type = MOUSEPAN;
          break;
      }

      event.mouse_pos = event_state->mouse_pos = GfVec2i(pd->x, pd->y);
      event.val = KM_NOTHING;

      /* The direction is inverted from the device due to system preferences. */
      event.is_direction_inverted = pd->isDirectionInverted;

      wm_event_add_trackpad(win, &event, pd->deltaX, -pd->deltaY);
      break;
    }

    /* Mouse button. */
    case AnchorEventTypeButtonDown:
    case AnchorEventTypeButtonUp: {
      AnchorEventButtonData *bd = (AnchorEventButtonData *)customdata;

      /* Get value and type from Anchor. */
      event.val = (type == AnchorEventTypeButtonDown) ? KM_PRESS : KM_RELEASE;

      if (bd->button == ANCHOR_BUTTON_MASK_LEFT) {
        event.type = LEFTMOUSE;
      } else if (bd->button == ANCHOR_BUTTON_MASK_RIGHT) {
        event.type = RIGHTMOUSE;
      } else if (bd->button == ANCHOR_BUTTON_MASK_BUTTON_4) {
        event.type = BUTTON4MOUSE;
      } else if (bd->button == ANCHOR_BUTTON_MASK_BUTTON_5) {
        event.type = BUTTON5MOUSE;
      } else if (bd->button == ANCHOR_BUTTON_MASK_BUTTON_6) {
        event.type = BUTTON6MOUSE;
      } else if (bd->button == ANCHOR_BUTTON_MASK_BUTTON_7) {
        event.type = BUTTON7MOUSE;
      } else {
        event.type = MIDDLEMOUSE;
      }

      /* Get tablet data. */
      wm_tablet_data_from_anchor(&bd->tablet, &event.tablet);

      wm_eventemulation(&event, false);
      wm_event_prev_values_set(&event, event_state);

      /* Copy to event state. */
      event_state->val = event.val;
      event_state->type = event.type;

      /* Double click test. */
      if (wm_event_is_double_click(&event)) {
        TF_WARN("WM -- Send double click");
        event.val = KM_DBL_CLICK;
      }
      if (event.val == KM_PRESS) {
        wm_event_prev_click_set(&event, event_state);
      }

      /* Add to other window if event is there (not to both!). */
      wmWindow *win_other = wm_event_cursor_other_windows(wm, win, &event);
      if (win_other) {
        wmEvent event_other = *win_other->eventstate;

        event_other.prevtype = event_other.type;
        event_other.prevval = event_other.val;

        event_other.mouse_pos = event.mouse_pos;

        event_other.type = event.type;
        event_other.val = event.val;
        event_other.tablet = event.tablet;

        wm_event_add(win_other, &event_other);
      } else {
        wm_event_add(win, &event);
      }

      break;
    }
    /* Keyboard. */
    case AnchorEventTypeKeyDown:
    case AnchorEventTypeKeyUp: {
      AnchorEventKeyData *kd = (AnchorEventKeyData *)customdata;
      short keymodifier = KM_NOTHING;
      event.type = static_cast<eWmEventType>(convert_key(kd->key));
      event.ascii = kd->ascii;
      memcpy(event.utf8_buf, kd->utf8_buf, sizeof(event.utf8_buf));
      event.is_repeat = kd->is_repeat;
      event.val = (type == AnchorEventTypeKeyDown) ? KM_PRESS : KM_RELEASE;

      wm_eventemulation(&event, false);
      wm_event_prev_values_set(&event, event_state);

      /* Copy to event state. */
      event_state->val = event.val;
      event_state->type = event.type;
      event_state->is_repeat = event.is_repeat;

      /* Exclude arrow keys, esc, etc from text input. */
      if (type == AnchorEventTypeKeyUp) {
        event.ascii = '\0';

        /* Anchor should do this already for key up. */
        if (event.utf8_buf[0]) {
          TF_WARN("WM -- Anchor on your platform is misbehaving, utf8 events on key up!");
        }
        event.utf8_buf[0] = '\0';
      } else {
        if (event.ascii < 32 && event.ascii > 0) {
          event.ascii = '\0';
        }
        if (event.utf8_buf[0] < 32 && event.utf8_buf[0] > 0) {
          event.utf8_buf[0] = '\0';
        }
      }

      if (event.utf8_buf[0]) {
        if (KLI_str_utf8_size(event.utf8_buf) == -1) {
          TF_WARN("WM -- Anchor detected an invalid unicode character '%d'",
                  (int)(unsigned char)event.utf8_buf[0]);
          event.utf8_buf[0] = '\0';
        }
      }

      switch (event.type) {
        case EVT_LEFTSHIFTKEY:
        case EVT_RIGHTSHIFTKEY:
          if (event.val == KM_PRESS) {
            if (event_state->ctrl || event_state->alt || event_state->oskey) {
              keymodifier = (KM_MOD_FIRST | KM_MOD_SECOND);
            } else {
              keymodifier = KM_MOD_FIRST;
            }
          }
          event.shift = event_state->shift = keymodifier;
          break;
        case EVT_LEFTCTRLKEY:
        case EVT_RIGHTCTRLKEY:
          if (event.val == KM_PRESS) {
            if (event_state->shift || event_state->alt || event_state->oskey) {
              keymodifier = (KM_MOD_FIRST | KM_MOD_SECOND);
            } else {
              keymodifier = KM_MOD_FIRST;
            }
          }
          event.ctrl = event_state->ctrl = keymodifier;
          break;
        case EVT_LEFTALTKEY:
        case EVT_RIGHTALTKEY:
          if (event.val == KM_PRESS) {
            if (event_state->ctrl || event_state->shift || event_state->oskey) {
              keymodifier = (KM_MOD_FIRST | KM_MOD_SECOND);
            } else {
              keymodifier = KM_MOD_FIRST;
            }
          }
          event.alt = event_state->alt = keymodifier;
          break;
        case EVT_OSKEY:
          if (event.val == KM_PRESS) {
            if (event_state->ctrl || event_state->alt || event_state->shift) {
              keymodifier = (KM_MOD_FIRST | KM_MOD_SECOND);
            } else {
              keymodifier = KM_MOD_FIRST;
            }
          }
          event.oskey = event_state->oskey = keymodifier;
          break;
        default:
          if (event.val == KM_PRESS && event.keymodifier == 0) {
            /* Only set in eventstate, for next event. */
            event_state->keymodifier = event.type;
          } else if (event.val == KM_RELEASE && event.keymodifier == event.type) {
            event.keymodifier = event_state->keymodifier = 0;
          }
          break;
      }

      /* Double click test. */
      /* If previous event was same type, and previous was release, and now it presses... */
      if (wm_event_is_double_click(&event)) {
        TF_WARN("WM -- Send double click");
        event.val = KM_DBL_CLICK;
      }

      /* This case happens on holding a key pressed,
       * it should not generate press events with the same key as modifier. */
      if (event.keymodifier == event.type) {
        event.keymodifier = 0;
      }

      /* This case happens with an external numpad, and also when using 'dead keys'
       * (to compose complex latin characters e.g.), it's not really clear why.
       * Since it's impossible to map a key modifier to an unknown key,
       * it shouldn't harm to clear it. */
      if (event.keymodifier == EVT_UNKNOWNKEY) {
        event_state->keymodifier = event.keymodifier = 0;
      }

      /* If test_break set, it catches this. Do not set with modifier presses.
       * XXX Keep global for now? */
      if ((event.type == EVT_ESCKEY && event.val == KM_PRESS) &&
          /* Check other modifiers because ms-windows uses these to bring up the task manager. */
          (event.shift == 0 && event.ctrl == 0 && event.alt == 0)) {
        G.is_break = true;
      }

      /* Double click test - only for press. */
      if (event.val == KM_PRESS) {
        /* Don't reset timer & location when holding the key generates repeat events. */
        if (event.is_repeat == false) {
          wm_event_prev_click_set(&event, event_state);
        }
      }

      wm_event_add(win, &event);

      break;
    }

    case AnchorEventTypeWheel: {
      AnchorEventWheelData *wheelData = (AnchorEventWheelData *)customdata;

      if (wheelData->z > 0) {
        event.type = WHEELUPMOUSE;
      } else {
        event.type = WHEELDOWNMOUSE;
      }

      event.val = KM_PRESS;
      wm_event_add(win, &event);

      break;
    }
    case AnchorEventTypeTimer: {
      event.type = TIMER;
      event.custom = EVT_DATA_TIMER;
      event.customdata = customdata;
      event.val = KM_NOTHING;
      event.keymodifier = 0;
      wm_event_add(win, &event);

      break;
    }

    case AnchorEventTypeUnknown:
    case ANCHOR_NumEventTypes:
      break;

    case AnchorEventTypeWindowDeactivate: {
      event.type = WINDEACTIVATE;
      wm_event_add(win, &event);

      break;
    }
  }
}

void wm_event_free_handler(wmEventHandler *handler)
{
  delete handler;
}

void WM_event_remove_handlers(kContext *C, std::vector<wmEventHandler *> handlers)
{
  wmWindowManager *wm = CTX_wm_manager(C);
  std::vector<wmEventHandler *>::iterator iter;

  for (iter = handlers.begin(); iter != handlers.end(); ++iter) {
    KLI_assert((*iter)->type != 0);
    if ((*iter)->type == WM_HANDLER_TYPE_UI) {
      wmEventHandlerUI *handler = (wmEventHandlerUI *)(*iter);

      if (handler->remove_fn) {
        ScrArea *area = CTX_wm_area(C);
        ARegion *region = CTX_wm_region(C);
        ARegion *menu = CTX_wm_menu(C);

        if (handler->context.area) {
          CTX_wm_area_set(C, handler->context.area);
        }
        if (handler->context.region) {
          CTX_wm_region_set(C, handler->context.region);
        }
        if (handler->context.menu) {
          CTX_wm_menu_set(C, handler->context.menu);
        }

        handler->remove_fn(C, handler->user_data);

        CTX_wm_area_set(C, area);
        CTX_wm_region_set(C, region);
        CTX_wm_menu_set(C, menu);
      }
    }

    wm_event_free_handler((*iter));
  }

  handlers.clear();
}


bool WM_operator_poll(kContext *C, wmOperatorType *ot)
{
  // TF_FOR_ALL(macro, &ot->macro)
  // {
  //   wmOperatorType *ot_macro = WM_operatortype_find(macro->idname);

  //   if (!WM_operator_poll(C, ot_macro))
  //   {
  //     return false;
  //   }
  // }

  if (ot->poll) {
    return ot->poll(C);
  }

  return true;
}


static wmOperator *wm_operator_create(wmWindowManager *wm,
                                      wmOperatorType *ot,
                                      PointerLUXO *properties,
                                      ReportList *reports)
{
  wmOperator *op = new wmOperator();

  op->type = ot;
  op->idname = ot->idname;

  /* Initialize properties. */
  if (properties && !properties->props.empty()) {
    op->properties = properties;
  } else {
    // op->properties
  }

  /* Initialize error reports. */
  if (reports) {
    op->reports = reports;
  } else {
    op->reports = new ReportList();
  }

  return op;
}


static void wm_region_mouse_co(kContext *C, wmEvent *event)
{
  ARegion *region = CTX_wm_region(C);
  if (region) {
    GfVec2f region_pos = FormFactory(region->pos);

    /* Compatibility convention. */
    GET_X(event->mval) = GET_X(event->mouse_pos) - GET_X(region_pos);
    GET_Y(event->mval) = GET_Y(event->mouse_pos) - GET_Y(region_pos);
  } else {
    /* These values are invalid (avoid odd behavior by relying on old mval values). */
    GET_X(event->mval) = -1;
    GET_Y(event->mval) = -1;
  }
}


static void wm_operator_finished(kContext *C, wmOperator *op, const bool repeat, const bool store)
{
  wmWindowManager *wm = CTX_wm_manager(C);
  enum
  {
    NOP,
    SET,
    CLEAR,
  } hud_status = NOP;

  op->customdata = NULL;

  if (store) {
    // WM_operator_last_properties_store(op);
  }

  if (wm->op_undo_depth == 0) {
    if (op->type->flag & OPTYPE_UNDO) {
      // ED_undo_push_op(C, op);
      if (repeat == 0) {
        hud_status = CLEAR;
      }
    } else if (op->type->flag & OPTYPE_UNDO_GROUPED) {
      // ED_undo_grouped_push_op(C, op);
      if (repeat == 0) {
        hud_status = CLEAR;
      }
    }
  }

  if (repeat == 0) {
    // if (wm_operator_register_check(wm, op->type)) {
    //   /* take ownership of reports (in case python provided own) */
    //   op->reports->flag |= RPT_FREE;

    //   wm_operator_register(C, op);
    //   WM_operator_region_active_win_set(C);

    //   if (WM_operator_last_redo(C) == op) {
    //     /* Show the redo panel. */
    //     hud_status = SET;
    //   }
    // }
    // else {
    //   WM_operator_free(op);
    // }
  }

  if (hud_status != NOP) {
    if (hud_status == SET) {
      ScrArea *area = CTX_wm_area(C);
      if (area) {
        // ED_area_type_hud_ensure(C, area);
      }
    } else if (hud_status == CLEAR) {
      // ED_area_type_hud_clear(wm, NULL);
    } else {
      KLI_assert_unreachable();
    }
  }
}


static bool isect_pt_v(const GfVec4i &rect, const GfVec2i &xy)
{
  if (GET_X(xy) < GET_X(rect)) {
    return false;
  }
  if (GET_X(xy) > GET_Z(rect)) {
    return false;
  }
  if (GET_Y(xy) < GET_Y(rect)) {
    return false;
  }
  if (GET_Y(xy) > GET_W(rect)) {
    return false;
  }
  return true;
}


void WM_operator_free(wmOperator *op)
{
  if (op && !op->properties->props.empty()) {
    // IDP_FreeProperty(op->properties);
  }

  if (op->reports && (op->reports->flag & RPT_FREE)) {
    // KKE_reports_clear(op->reports);
    delete op->reports;
  }

  // if (op->macro.first) {
  //   wmOperator *opm, *opmnext;
  //   for (opm = op->macro.first; opm; opm = opmnext) {
  //     opmnext = opm->next;
  //     WM_operator_free(opm);
  //   }
  // }

  delete op;
}


void wm_event_handler_ui_cancel_ex(kContext *C,
                                   wmWindow *win,
                                   ARegion *region,
                                   bool reactivate_button)
{
  if (!region) {
    return;
  }

  // TF_FOR_ALL (handler_base, &region->handlers)
  // {
  //   if (handler_base->type == WM_HANDLER_TYPE_UI)
  //   {
  //     wmEventHandlerUI *handler = (wmEventHandlerUI *)handler_base;
  //     KLI_assert(handler->handle_fn != NULL);
  //     wmEvent event;
  //     wm_event_init_from_window(win, &event);
  //     event.type = EVT_BUT_CANCEL;
  //     event.val = reactivate_button ? 0 : 1;
  //     event.is_repeat = false;
  //     handler->handle_fn(C, &event, handler->user_data);
  //   }
  // }
}


static void wm_event_handler_ui_cancel(kContext *C)
{
  wmWindow *win = CTX_wm_window(C);
  ARegion *region = CTX_wm_region(C);
  wm_event_handler_ui_cancel_ex(C, win, region, true);
}


static int wm_operator_invoke(kContext *C,
                              wmOperatorType *ot,
                              wmEvent *event,
                              PointerLUXO *properties,
                              ReportList *reports,
                              const bool poll_only,
                              bool use_last_properties)
{
  int retval = OPERATOR_PASS_THROUGH;

  if (poll_only) {
    return WM_operator_poll(C, ot);
  }

  if (WM_operator_poll(C, ot)) {
    wmWindowManager *wm = CTX_wm_manager(C);

    /* If reports == NULL, they'll be initialized. */
    wmOperator *op = wm_operator_create(wm, ot, properties, reports);

    const bool is_nested_call = (wm->op_undo_depth != 0);

    if (event != NULL) {
      op->flag |= OP_IS_INVOKE;
    }

    /* Initialize setting from previous run. */
    if (!is_nested_call && use_last_properties) {
      // WM_operator_last_properties_init(op);
    }

    if ((event == NULL) || (event->type != MOUSEMOVE)) {
      TF_DEBUG(KRAKEN_DEBUG_OPERATORS)
        .Msg("handle evt %d win %p op %s",
             event ? event->type : 0,
             CTX_wm_screen(C)->active_region,
             ot->idname.GetText());
    }

    if (op->type->invoke && event) {
      wm_region_mouse_co(C, event);

      if (op->type->flag & OPTYPE_UNDO) {
        wm->op_undo_depth++;
      }

      retval = op->type->invoke(C, op, event);

      if (op->type->flag & OPTYPE_UNDO && CTX_wm_manager(C) == wm) {
        wm->op_undo_depth--;
      }
    } else if (op->type->exec) {
      if (op->type->flag & OPTYPE_UNDO) {
        wm->op_undo_depth++;
      }

      retval = op->type->exec(C, op);
      if (op->type->flag & OPTYPE_UNDO && CTX_wm_manager(C) == wm) {
        wm->op_undo_depth--;
      }
    } else {
      TF_DEBUG(KRAKEN_DEBUG_OPERATORS).Msg("invalid operator call '%s'", op->idname.GetText());
    }

    if (!(retval & OPERATOR_HANDLED) && (retval & (OPERATOR_FINISHED | OPERATOR_CANCELLED))) {
      /* Only show the report if the report list was not given in the function. */
      // wm_operator_reports(C, op, retval, (reports != NULL));
    }

    if (retval & OPERATOR_HANDLED) {
      /* Do nothing, wm_operator_exec() has been called somewhere. */
    } else if (retval & OPERATOR_FINISHED) {
      const bool store = !is_nested_call && use_last_properties;
      wm_operator_finished(C, op, false, store);
    } else if (retval & OPERATOR_RUNNING_MODAL) {
      /* Take ownership of reports (in case python provided own). */
      op->reports->flag |= RPT_FREE;

      /* Grab cursor during blocking modal ops (X11)
       * Also check for macro.
       */
      if (ot->flag & OPTYPE_BLOCKING || (op->opm && op->opm->type->flag & OPTYPE_BLOCKING)) {
        int bounds[4] = {-1, -1, -1, -1};
        int wrap = WM_CURSOR_WRAP_NONE;

        UserDef *uprefs = CTX_data_prefs(C);

        if (event && (uprefs->uiflag & USER_CONTINUOUS_MOUSE)) {
          const wmOperator *op_test = op->opm ? op->opm : op;
          const wmOperatorType *ot_test = op_test->type;
          if ((ot_test->flag & OPTYPE_GRAB_CURSOR_XY) ||
              (op_test->flag & OP_IS_MODAL_GRAB_CURSOR)) {
            wrap = WM_CURSOR_WRAP_XY;
          } else if (ot_test->flag & OPTYPE_GRAB_CURSOR_X) {
            wrap = WM_CURSOR_WRAP_X;
          } else if (ot_test->flag & OPTYPE_GRAB_CURSOR_Y) {
            wrap = WM_CURSOR_WRAP_Y;
          }
        }

        if (wrap) {
          GfVec2i regionpos;
          GfVec2i regionsize;
          ARegion *region = CTX_wm_region(C);
          if (region) {
            region->pos.Get(&regionpos);
            region->size.Get(&regionsize);
          }

          GfVec2i areapos;
          GfVec2i areasize;
          ScrArea *area = CTX_wm_area(C);
          if (area) {
            region->pos.Get(&areapos);
            region->size.Get(&areasize);
          }

          GfVec4i *winrect = NULL;

          /* Wrap only in X for header. */
          if (region && RGN_TYPE_IS_HEADER_ANY(region->regiontype)) {
            wrap = WM_CURSOR_WRAP_X;
          }

          if (region && region->regiontype == RGN_TYPE_WINDOW &&
              isect_pt_v(
                GfVec4i(GET_X(regionpos), GET_Y(regionpos), GET_X(regionsize), GET_Y(regionsize)),
                event->mouse_pos)) {
            winrect = new GfVec4i(GET_X(regionpos),
                                  GET_Y(regionpos),
                                  GET_X(regionsize),
                                  GET_Y(regionsize));
          } else if (area &&
                     isect_pt_v(
                       GfVec4i(GET_X(areapos), GET_Y(areapos), GET_X(areasize), GET_Y(areasize)),
                       event->mouse_pos)) {
            winrect = new GfVec4i(GET_X(areapos),
                                  GET_Y(areapos),
                                  GET_X(areasize),
                                  GET_Y(areasize));
          }

          if (winrect) {
            GET_X(bounds) = GET_X(winrect->GetArray());
            GET_Y(bounds) = GET_Y(winrect->GetArray());
            GET_Z(bounds) = GET_Z(winrect->GetArray());
            GET_W(bounds) = GET_W(winrect->GetArray());
          }
        }

        WM_cursor_grab_enable(CTX_wm_window(C), wrap, false, bounds);
      }

      /* Cancel UI handlers, typically tooltips that can hang around
       * while dragging the view or worse, that stay there permanently
       * after the modal operator has swallowed all events and passed
       * none to the UI handler. */
      wm_event_handler_ui_cancel(C);
    } else {
      WM_operator_free(op);
    }
  }

  return retval;
}


static int wm_operator_call_internal(kContext *C,
                                     wmOperatorType *ot,
                                     PointerLUXO *properties,
                                     ReportList *reports,
                                     const short context,
                                     const bool poll_only,
                                     wmEvent *event)
{
  int retval;

  CTX_wm_operator_poll_msg_clear(C);

  /* Dummy test. */
  if (ot) {
    wmWindow *window = CTX_wm_window(C);

    if (event) {
      switch (context) {
        case WM_OP_INVOKE_DEFAULT:
        case WM_OP_INVOKE_REGION_WIN:
        case WM_OP_INVOKE_REGION_PREVIEW:
        case WM_OP_INVOKE_REGION_CHANNELS:
        case WM_OP_INVOKE_AREA:
        case WM_OP_INVOKE_SCREEN:
          /* Window is needed for invoke and cancel operators. */
          if (window == NULL) {
            if (poll_only) {
              CTX_wm_operator_poll_msg_set(C, "Missing 'window' in context");
            }
            return 0;
          } else {
            event = window->eventstate;
          }
          break;
        default:
          event = NULL;
          break;
      }
    } else {
      switch (context) {
        case WM_OP_EXEC_DEFAULT:
        case WM_OP_EXEC_REGION_WIN:
        case WM_OP_EXEC_REGION_PREVIEW:
        case WM_OP_EXEC_REGION_CHANNELS:
        case WM_OP_EXEC_AREA:
        case WM_OP_EXEC_SCREEN:
          event = NULL;
        default:
          break;
      }
    }

    switch (context) {
      case WM_OP_EXEC_REGION_WIN:
      case WM_OP_INVOKE_REGION_WIN:
      case WM_OP_EXEC_REGION_CHANNELS:
      case WM_OP_INVOKE_REGION_CHANNELS:
      case WM_OP_EXEC_REGION_PREVIEW:
      case WM_OP_INVOKE_REGION_PREVIEW: {
        /* Forces operator to go to the region window/channels/preview, for header menus,
         * but we stay in the same region if we are already in one.
         */
        ARegion *region = CTX_wm_region(C);
        ScrArea *area = CTX_wm_area(C);
        eRegionType type = RGN_TYPE_WINDOW;

        switch (context) {
          case WM_OP_EXEC_REGION_CHANNELS:
          case WM_OP_INVOKE_REGION_CHANNELS:
            type = RGN_TYPE_CHANNELS;
            break;

          case WM_OP_EXEC_REGION_PREVIEW:
          case WM_OP_INVOKE_REGION_PREVIEW:
            type = RGN_TYPE_PREVIEW;
            break;

          case WM_OP_EXEC_REGION_WIN:
          case WM_OP_INVOKE_REGION_WIN:
          default:
            type = RGN_TYPE_WINDOW;
            break;
        }

        if (!(region && region->regiontype == type) && area) {
          ARegion *region_other = (type == RGN_TYPE_WINDOW) ?
                                    KKE_area_find_region_active_win(area) :
                                    KKE_area_find_region_type(area, type);
          if (region_other) {
            CTX_wm_region_set(C, region_other);
          }
        }

        retval = wm_operator_invoke(C, ot, event, properties, reports, poll_only, true);

        /* Set region back. */
        CTX_wm_region_set(C, region);

        return retval;
      }
      case WM_OP_EXEC_AREA:
      case WM_OP_INVOKE_AREA: {
        /* Remove region from context. */
        ARegion *region = CTX_wm_region(C);

        CTX_wm_region_set(C, NULL);
        retval = wm_operator_invoke(C, ot, event, properties, reports, poll_only, true);
        CTX_wm_region_set(C, region);

        return retval;
      }
      case WM_OP_EXEC_SCREEN:
      case WM_OP_INVOKE_SCREEN: {
        /* Remove region + area from context. */
        ARegion *region = CTX_wm_region(C);
        ScrArea *area = CTX_wm_area(C);

        CTX_wm_region_set(C, NULL);
        CTX_wm_area_set(C, NULL);
        retval = wm_operator_invoke(C, ot, event, properties, reports, poll_only, true);
        CTX_wm_area_set(C, area);
        CTX_wm_region_set(C, region);

        return retval;
      }
      case WM_OP_EXEC_DEFAULT:
      case WM_OP_INVOKE_DEFAULT:
        return wm_operator_invoke(C, ot, event, properties, reports, poll_only, true);
    }
  }

  return 0;
}

int WM_operator_name_call_ptr(kContext *C,
                              wmOperatorType *ot,
                              short context,
                              PointerLUXO *properties)
{
  return wm_operator_call_internal(C, ot, properties, NULL, context, false, NULL);
}


int WM_operator_name_call(kContext *C,
                          const TfToken &optoken,
                          short context,
                          PointerLUXO *properties)
{
  wmOperatorType *ot = WM_operatortype_find(optoken);
  if (ot) {
    return WM_operator_name_call_ptr(C, ot, context, properties);
  }

  return 0;
}


void WM_event_do_refresh_wm(kContext *C)
{
  wmWindowManager *wm = CTX_wm_manager(C);

  UNIVERSE_FOR_ALL (win, wm->windows) {

    const kScreen *screen = WM_window_get_active_screen(VALUE(win));

    CTX_wm_window_set(C, VALUE(win));

    UNIVERSE_FOR_ALL (area, screen->areas) {
      if (area->do_refresh) {
        CTX_wm_area_set(C, area);
        ED_area_do_refresh(C, area);
      }
    }
  }

  CTX_wm_window_set(C, NULL);
}

static void wm_event_free(wmEvent *event)
{
  if (event->customdata) {
    if (event->customdatafree) {
      if (event->custom == EVT_DATA_DRAGDROP) {
        WM_drag_free_list((std::vector<wmDrag *> &)event->customdata);
      } else {
        free(event->customdata);
      }
    }
  }

  free(event);
}

static void wm_event_free_all(wmWindow *win)
{
  wmEvent *event;
  while (!win->event_queue.empty()) {
    event = win->event_queue.front();
    wm_event_free(event);
  }

  win->event_queue.clear();
}

/* -------------------------------------------------------------------- */
/** \name Main Event Queue (Every Window)
 *
 * Handle events for all windows, run from the #WM_main event loop.
 * \{ */

/* Called in main loop. */
/* Goes over entire hierarchy:  events -> window -> screen -> area -> region. */
void WM_event_do_handlers(kContext *C)
{
  wmWindowManager *wm = CTX_wm_manager(C);
  // KLI_assert(ED_undo_is_state_valid(C));

  /* Update key configuration before handling events. */
  // WM_keyconfig_update(wm);
  // WM_gizmoconfig_update(CTX_data_main(C));

  UNIVERSE_FOR_ALL (win, wm->windows) {
    kScreen *screen = WM_window_get_active_screen(VALUE(win));

    /* Some safety checks - these should always be set! */
    KLI_assert(WM_window_get_active_scene(VALUE(win)));
    KLI_assert(WM_window_get_active_screen(VALUE(win)));
    KLI_assert(WM_window_get_active_workspace(VALUE(win)));

    if (screen == NULL) {
      wm_event_free_all(VALUE(win));
    } else {
      Scene *scene = WM_window_get_active_scene(VALUE(win));
    }

    wmEvent *event;
    while ((event = (*VALUE(win)->event_queue.begin()))) {
      int action = WM_HANDLER_CONTINUE;

      screen = WM_window_get_active_screen(VALUE(win));

      if (G.debug & (G_DEBUG_HANDLERS | G_DEBUG_EVENTS) &&
          ((event->type != MOUSEMOVE) || (event->type != INBETWEEN_MOUSEMOVE))) {
        TF_WARN("\n%s: Handling event\n", __func__);
        // WM_event_print(event);
      }

      CTX_wm_window_set(C, VALUE(win));
    }
  }
}


static int wm_handler_fileselect() {}


WABI_NAMESPACE_END