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

#include <wabi/base/tf/stringUtils.h>

WABI_NAMESPACE_BEGIN


static bool wm_test_duplicate_notifier(wmWindowManager *wm, uint type, void *reference)
{
  for (auto const &note : wm->notifier_queue)
  {
    if ((note->category | note->data | note->subtype | note->action) == type &&
        note->reference == reference)
    {
      return true;
    }
  }

  return false;
}


void WM_main_add_notifier(unsigned int type, void *reference)
{
  wmWindowManager *wm = G.main->wm.at(0);

  if (!wm || wm_test_duplicate_notifier(wm, type, reference))
  {
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
  if (wm_test_duplicate_notifier(wm, type, reference))
  {
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

  if (event_to_add_after == NULL)
  {
    win->event_queue.push_back(event);
  }
  else
  {
    wmEventQueue::iterator it;
    for (it = win->event_queue.begin(); it != win->event_queue.end(); ++it)
    {
      if ((*it) == event_to_add_after)
      {
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


wmEvent *wm_event_add(wmWindow *win, wmEvent *event_to_add)
{
  return wm_event_add_ex(win, event_to_add, NULL);
}


static wmEvent *wm_event_add_mousemove(wmWindow *win, wmEvent *event)
{
  wmEvent *event_last = win->event_queue.back();

  if (event_last && event_last->type == MOUSEMOVE)
  {
    event_last->type = INBETWEEN_MOUSEMOVE;
  }

  wmEvent *event_new = wm_event_add(win, event);
  if (event_last == NULL)
  {
    event_last = win->eventstate;
  }

  event_new->prev_mouse_pos = event_last->mouse_pos;
  return event_new;
}


static wmWindow *wm_event_cursor_other_windows(wmWindowManager *wm, wmWindow *win, wmEvent *event)
{
  GfVec2i mval = event->mouse_pos;

  if (wm->windows.size() <= 1)
  {
    return nullptr;
  }

  /* In order to use window size and mouse position (pixels), we have to use a WM function. */

  /* check if outside, include top window bar... */
  if (mval[0] < 0 || mval[1] < 0 || mval[0] > WM_window_pixels_x(win) ||
      mval[1] > WM_window_pixels_y(win) + 30)
  {
    wmWindow *win_other;
    if (WM_window_find_under_cursor(wm, win, win, mval, &win_other, &mval))
    {
      event->mouse_pos = mval;
      return win_other;
    }
  }
  return nullptr;
}


void WM_event_init_from_window(wmWindow *win, wmEvent *event)
{
  *event = *(win->eventstate);
}


void WM_event_add_anchorevent(wmWindowManager *wm, wmWindow *win, int type, void *customdata)
{
  wmEvent event, *event_state = win->eventstate;

  event = *event_state;
  event.is_repeat = false;

  event.prevtype = event.type;
  event.prevval = event.val;

  switch (type)
  {
    case AnchorEventTypeCursorMove: {
      AnchorEventCursorData *cd = (AnchorEventCursorData *)customdata;
      event.mouse_pos = GfVec2i(cd->x, cd->y);

      event.type = MOUSEMOVE;
      {
        wmEvent *event_new = wm_event_add_mousemove(win, &event);
        event_state->mouse_pos = event_new->mouse_pos;
      }

      wmWindow *win_other = wm_event_cursor_other_windows(wm, win, &event);
    }
  }
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

  if (ot->poll)
  {
    return ot->poll(C);
  }

  return true;
}


static wmOperator *wm_operator_create(wmWindowManager *wm,
                                      wmOperatorType *ot,
                                      PointerUNI *properties,
                                      ReportList *reports)
{
  wmOperator *op = new wmOperator();

  op->type = ot;
  op->idname = ot->idname;

  /* Initialize properties. */
  if (properties && !properties->props.empty())
  {
    op->properties = properties;
  }
  else
  {
    // op->properties
  }

  /* Initialize error reports. */
  if (reports)
  {
    op->reports = reports;
  }
  else
  {
    op->reports = new ReportList();
  }

  return op;
}


static void wm_region_mouse_co(kContext *C, wmEvent *event)
{
  ARegion *region = CTX_wm_region(C);
  if (region)
  {
    GfVec2f region_pos = FormFactory(region->pos);

    /* Compatibility convention. */
    GET_X(event->mval) = GET_X(event->mouse_pos) - GET_X(region_pos);
    GET_Y(event->mval) = GET_Y(event->mouse_pos) - GET_Y(region_pos);
  }
  else
  {
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

  if (store)
  {
    // WM_operator_last_properties_store(op);
  }

  if (wm->op_undo_depth == 0)
  {
    if (op->type->flag & OPTYPE_UNDO)
    {
      // ED_undo_push_op(C, op);
      if (repeat == 0)
      {
        hud_status = CLEAR;
      }
    }
    else if (op->type->flag & OPTYPE_UNDO_GROUPED)
    {
      // ED_undo_grouped_push_op(C, op);
      if (repeat == 0)
      {
        hud_status = CLEAR;
      }
    }
  }

  if (repeat == 0)
  {
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

  if (hud_status != NOP)
  {
    if (hud_status == SET)
    {
      ScrArea *area = CTX_wm_area(C);
      if (area)
      {
        // ED_area_type_hud_ensure(C, area);
      }
    }
    else if (hud_status == CLEAR)
    {
      // ED_area_type_hud_clear(wm, NULL);
    }
    else
    {
      KLI_assert_unreachable();
    }
  }
}


static bool isect_pt_v(const GfVec4i &rect, const GfVec2i &xy)
{
  if (GET_X(xy) < GET_X(rect))
  {
    return false;
  }
  if (GET_X(xy) > GET_Z(rect))
  {
    return false;
  }
  if (GET_Y(xy) < GET_Y(rect))
  {
    return false;
  }
  if (GET_Y(xy) > GET_W(rect))
  {
    return false;
  }
  return true;
}


void WM_operator_free(wmOperator *op)
{
  if (op && !op->properties->props.empty())
  {
    // IDP_FreeProperty(op->properties);
  }

  if (op->reports && (op->reports->flag & RPT_FREE))
  {
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
  if (!region)
  {
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
                              PointerUNI *properties,
                              ReportList *reports,
                              const bool poll_only,
                              bool use_last_properties)
{
  int retval = OPERATOR_PASS_THROUGH;

  if (poll_only)
  {
    return WM_operator_poll(C, ot);
  }

  if (WM_operator_poll(C, ot))
  {
    wmWindowManager *wm = CTX_wm_manager(C);

    /* If reports == NULL, they'll be initialized. */
    wmOperator *op = wm_operator_create(wm, ot, properties, reports);

    const bool is_nested_call = (wm->op_undo_depth != 0);

    if (event != NULL)
    {
      op->flag |= OP_IS_INVOKE;
    }

    /* Initialize setting from previous run. */
    if (!is_nested_call && use_last_properties)
    {
      // WM_operator_last_properties_init(op);
    }

    if ((event == NULL) || (event->type != MOUSEMOVE))
    {
      TF_DEBUG(KRAKEN_DEBUG_OPERATORS).Msg("handle evt %d win %p op %s", event ? event->type : 0, CTX_wm_screen(C)->active_region, ot->idname);
    }

    if (op->type->invoke && event)
    {
      wm_region_mouse_co(C, event);

      if (op->type->flag & OPTYPE_UNDO)
      {
        wm->op_undo_depth++;
      }

      retval = op->type->invoke(C, op, event);

      if (op->type->flag & OPTYPE_UNDO && CTX_wm_manager(C) == wm)
      {
        wm->op_undo_depth--;
      }
    }
    else if (op->type->exec)
    {
      if (op->type->flag & OPTYPE_UNDO)
      {
        wm->op_undo_depth++;
      }

      retval = op->type->exec(C, op);
      if (op->type->flag & OPTYPE_UNDO && CTX_wm_manager(C) == wm)
      {
        wm->op_undo_depth--;
      }
    }
    else
    {
      TF_DEBUG(KRAKEN_DEBUG_OPERATORS).Msg("invalid operator call '%s'", op->idname);
    }

    if (!(retval & OPERATOR_HANDLED) && (retval & (OPERATOR_FINISHED | OPERATOR_CANCELLED)))
    {
      /* Only show the report if the report list was not given in the function. */
      // wm_operator_reports(C, op, retval, (reports != NULL));
    }

    if (retval & OPERATOR_HANDLED)
    {
      /* Do nothing, wm_operator_exec() has been called somewhere. */
    }
    else if (retval & OPERATOR_FINISHED)
    {
      const bool store = !is_nested_call && use_last_properties;
      wm_operator_finished(C, op, false, store);
    }
    else if (retval & OPERATOR_RUNNING_MODAL)
    {
      /* Take ownership of reports (in case python provided own). */
      op->reports->flag |= RPT_FREE;

      /* Grab cursor during blocking modal ops (X11)
       * Also check for macro.
       */
      if (ot->flag & OPTYPE_BLOCKING || (op->opm && op->opm->type->flag & OPTYPE_BLOCKING))
      {
        int bounds[4] = {-1, -1, -1, -1};
        int wrap = WM_CURSOR_WRAP_NONE;

        UserDef *uprefs = CTX_data_prefs(C);

        if (event && (uprefs->uiflag & USER_CONTINUOUS_MOUSE))
        {
          const wmOperator *op_test = op->opm ? op->opm : op;
          const wmOperatorType *ot_test = op_test->type;
          if ((ot_test->flag & OPTYPE_GRAB_CURSOR_XY) ||
              (op_test->flag & OP_IS_MODAL_GRAB_CURSOR))
          {
            wrap = WM_CURSOR_WRAP_XY;
          }
          else if (ot_test->flag & OPTYPE_GRAB_CURSOR_X)
          {
            wrap = WM_CURSOR_WRAP_X;
          }
          else if (ot_test->flag & OPTYPE_GRAB_CURSOR_Y)
          {
            wrap = WM_CURSOR_WRAP_Y;
          }
        }

        if (wrap)
        {
          GfVec2i regionpos;
          GfVec2i regionsize;
          ARegion *region = CTX_wm_region(C);
          if (region)
          {
            region->pos.Get(&regionpos);
            region->size.Get(&regionsize);
          }

          GfVec2i areapos;
          GfVec2i areasize;
          ScrArea *area = CTX_wm_area(C);
          if (area)
          {
            region->pos.Get(&areapos);
            region->size.Get(&areasize);
          }

          GfVec4i *winrect = NULL;

          /* Wrap only in X for header. */
          if (region && RGN_TYPE_IS_HEADER_ANY(region->regiontype))
          {
            wrap = WM_CURSOR_WRAP_X;
          }

          if (region && region->regiontype == RGN_TYPE_WINDOW &&
              isect_pt_v(GfVec4i(GET_X(regionpos),
                                 GET_Y(regionpos),
                                 GET_X(regionsize),
                                 GET_Y(regionsize)),
                         event->mouse_pos))
          {
            winrect = new GfVec4i(GET_X(regionpos),
                                  GET_Y(regionpos),
                                  GET_X(regionsize),
                                  GET_Y(regionsize));
          }
          else if (area && isect_pt_v(GfVec4i(GET_X(areapos),
                                              GET_Y(areapos),
                                              GET_X(areasize),
                                              GET_Y(areasize)),
                                      event->mouse_pos))
          {
            winrect = new GfVec4i(GET_X(areapos),
                                  GET_Y(areapos),
                                  GET_X(areasize),
                                  GET_Y(areasize));
          }

          if (winrect)
          {
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
    }
    else
    {
      WM_operator_free(op);
    }
  }

  return retval;
}


static int wm_operator_call_internal(kContext *C,
                                     wmOperatorType *ot,
                                     PointerUNI *properties,
                                     ReportList *reports,
                                     const short context,
                                     const bool poll_only,
                                     wmEvent *event)
{
  int retval;

  CTX_wm_operator_poll_msg_clear(C);

  /* Dummy test. */
  if (ot)
  {
    wmWindow *window = CTX_wm_window(C);

    if (event)
    {
      switch (context)
      {
        case WM_OP_INVOKE_DEFAULT:
        case WM_OP_INVOKE_REGION_WIN:
        case WM_OP_INVOKE_REGION_PREVIEW:
        case WM_OP_INVOKE_REGION_CHANNELS:
        case WM_OP_INVOKE_AREA:
        case WM_OP_INVOKE_SCREEN:
          /* Window is needed for invoke and cancel operators. */
          if (window == NULL)
          {
            if (poll_only)
            {
              CTX_wm_operator_poll_msg_set(C, "Missing 'window' in context");
            }
            return 0;
          }
          else
          {
            event = window->eventstate;
          }
          break;
        default:
          event = NULL;
          break;
      }
    }
    else
    {
      switch (context)
      {
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

    switch (context)
    {
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

        switch (context)
        {
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

        if (!(region && region->regiontype == type) && area)
        {
          // ARegion *region_other = (type == RGN_TYPE_WINDOW) ?
          //                         KKE_area_find_region_active_win(area) :
          //                         KKE_area_find_region_type(area, type);
          // if (region_other)
          // {
          //   CTX_wm_region_set(C, region_other);
          // }
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
                              PointerUNI *properties)
{
  return wm_operator_call_internal(C, ot, properties, NULL, context, false, NULL);
}


int WM_operator_name_call(kContext *C, const TfToken &optoken, short context, PointerUNI *properties)
{
  wmOperatorType *ot = WM_operatortype_find(optoken);
  if (ot)
  {
    return WM_operator_name_call_ptr(C, ot, context, properties);
  }

  return 0;
}


void WM_event_do_refresh_wm(kContext *C)
{}


static int wm_handler_fileselect()
{}


WABI_NAMESPACE_END