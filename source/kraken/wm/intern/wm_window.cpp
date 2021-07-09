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

#include "WM_window.h"
#include "WM_cursors_api.h"
#include "WM_debug_codes.h"
#include "WM_dragdrop.h"
#include "WM_event_system.h"
#include "WM_inline_tools.h"
#include "WM_operators.h"
#include "WM_tokens.h"

#include "UNI_area.h"
#include "UNI_context.h"
#include "UNI_factory.h"
#include "UNI_object.h"
#include "UNI_operator.h"
#include "UNI_userpref.h"
#include "UNI_window.h"
#include "UNI_workspace.h"

#include "ANCHOR_api.h"
#include "ANCHOR_event_consumer.h"
#include "ANCHOR_system.h"

#include "KKE_context.h"
#include "KKE_main.h"
#include "KKE_screen.h"
#include "KKE_workspace.h"

#include "KLI_icons.h"
#include "KLI_math_inline.h"
#include "KLI_string_utils.h"
#include "KLI_time.h"

#include "ED_fileselect.h"
#include "ED_screen.h"

#include <wabi/base/gf/vec2f.h>

WABI_NAMESPACE_BEGIN


/* handle to anchor system. */
static ANCHOR_SystemHandle anchor_system = NULL;


enum eModifierKeyType
{
  SHIFT = 's',
  CONTROL = 'c',
  ALT = 'a',
  OS = 'C',
};


static int query_qual(eModifierKeyType qual)
{
  eAnchorModifierKeyMask left, right;
  switch (qual)
  {
    case SHIFT:
      left = ANCHOR_ModifierKeyLeftShift;
      right = ANCHOR_ModifierKeyRightShift;
      break;
    case CONTROL:
      left = ANCHOR_ModifierKeyLeftControl;
      right = ANCHOR_ModifierKeyRightControl;
      break;
    case OS:
      left = right = ANCHOR_ModifierKeyOS;
      break;
    case ALT:
    default:
      left = ANCHOR_ModifierKeyLeftAlt;
      right = ANCHOR_ModifierKeyRightAlt;
      break;
  }

  int val = 0;
  ANCHOR::GetModifierKeyState(anchor_system, left, &val);
  if (!val)
  {
    ANCHOR::GetModifierKeyState(anchor_system, right, &val);
  }

  return val;
}


void wm_cursor_position_get(wmWindow *win, int *r_x, int *r_y)
{
  ANCHOR::GetCursorPosition(anchor_system, r_x, r_y);
  WM_cursor_position_from_anchor(win, r_x, r_y);
}


static void wm_window_set_drawable(wmWindowManager *wm, wmWindow *win, bool activate)
{
  KLI_assert((wm->windrawable == NULL) && (wm->windrawable == win));

  wm->windrawable = win;
  if (activate) {
    ANCHOR::ActivateWindowDrawingContext((ANCHOR_SystemWindowHandle)win->anchorwin);
  }
  // GPU_context_active_set(win->gpuctx);
}


static void wm_window_set_dpi(const wmWindow *win)
{
  float win_scale = FormFactory(win->scale);
  float win_linewidth = FormFactory(win->linewidth);

  float auto_dpi = ANCHOR::GetDPIHint((ANCHOR_SystemWindowHandle)win->anchorwin);

  auto_dpi = max_ff(auto_dpi, 96.0f);
  auto_dpi *= ANCHOR::GetNativePixelSize((ANCHOR_SystemWindowHandle)win->anchorwin);
  int dpi = auto_dpi * win_scale * (72.0 / 96.0f);

  int pixelsize = max_ii(1, (int)(dpi / 64));
  pixelsize = max_ii(1, pixelsize + win_linewidth);

  float dpiadj = dpi / pixelsize;
  float dpifac = (pixelsize * (float)(dpiadj)) / 72.0f;
  float wunit = (pixelsize * (dpiadj / pixelsize) * 20 + 36) / 72;

  /* ----- */

  /** Change it globally. */
  UI_DPI_FAC = dpifac;

  FormFactory(win->pixelsz, float(pixelsize));
  FormFactory(win->dpi, float(dpiadj));
  FormFactory(win->widgetunit, float(wunit += 2 * ((int)pixelsize - (int)dpifac)));

  /* ----- */

  /* update font drawing */
  ANCHOR::GetIO().FontGlobalScale = pixelsize * dpiadj;
}


void wm_window_clear_drawable(wmWindowManager *wm)
{
  if (wm->windrawable)
  {
    wm->windrawable = NULL;
  }
}

void wm_window_make_drawable(wmWindowManager *wm, wmWindow *win)
{
  if (win != wm->windrawable && win->anchorwin)
  {
    wm_window_clear_drawable(wm);

    wm_window_set_drawable(wm, win, true);

    /* this can change per window */
    wm_window_set_dpi(win);
  }
}


WorkSpaceLayout *WM_window_get_active_layout(const wmWindow *win)
{
  const WorkSpace *workspace = WM_window_get_active_workspace(win);
  return (ARCH_LIKELY(workspace != NULL) ? KKE_workspace_active_layout_get(win->workspace_hook) : NULL);
}


void WM_window_set_active_layout(wmWindow *win, WorkSpace *workspace, WorkSpaceLayout *layout)
{
  KKE_workspace_active_layout_set(win->workspace_hook, win->winid, workspace, layout);
}


WorkSpace *WM_window_get_active_workspace(const wmWindow *win)
{
  return KKE_workspace_active_get(win->workspace_hook);
}


cScreen *WM_window_get_active_screen(const wmWindow *win)
{
  const WorkSpace *workspace = WM_window_get_active_workspace(win);
  return (ARCH_LIKELY(workspace != NULL) ? KKE_workspace_active_screen_get(win->workspace_hook) : NULL);
}


static void wm_window_update_eventstate(wmWindow *win)
{
  /* Update mouse position when a window is activated. */
  wm_cursor_position_get(win, &GET_X(win->eventstate->mouse_pos), &GET_Y(win->eventstate->mouse_pos));
}


/**
 * This is called by anchor, and this is where
 * we handle events for windows or send them to
 * the event system. */
static int anchor_event_proc(ANCHOR_EventHandle evt, ANCHOR_UserPtr C_void_ptr)
{
  cContext *C = (cContext *)C_void_ptr;
  wmWindowManager *wm = CTX_wm_manager(C);
  eAnchorEventType type = ANCHOR::GetEventType(evt);

  if (type == ANCHOR_EventTypeQuitRequest)
  {
    /* Find an active window to display quit dialog in. */
    ANCHOR_SystemWindowHandle anchorwin = ANCHOR::GetEventWindow(evt);

    wmWindow *win = nullptr;
    if (anchorwin && ANCHOR::ValidWindow(anchor_system, anchorwin))
    {
      win = wm->windows.begin()->second;
      win->anchorwin = anchorwin;
    }
    else
    {
      win = wm->winactive;
    }

    /* Display quit dialog or quit immediately. */
    if (win)
    {
      wm_quit_with_optional_confirmation_prompt(C, win);
    }
    else
    {
      wm_exit_schedule_delayed(C);
    }
  }
  else
  {
    ANCHOR_SystemWindowHandle anchorwin = ANCHOR::GetEventWindow(evt);
    ANCHOR_EventDataPtr data = ANCHOR::GetEventData(evt);

    if (!anchorwin)
    {
      puts("<!> event has no window");
      return 1;
    }

    if (!ANCHOR::ValidWindow(anchor_system, anchorwin))
    {
      puts("<!> event has invalid window");
      return 1;
    }

    wmWindow *win = CTX_wm_window(C);
    win->anchorwin = anchorwin;

    switch (type)
    {
      case ANCHOR_EventTypeWindowDeactivate:
        WM_event_add_anchorevent(wm, win, type, data);
        win->active = false;

        win->eventstate->alt = 0;
        win->eventstate->ctrl = 0;
        win->eventstate->shift = 0;
        win->eventstate->oskey = 0;
        win->eventstate->keymodifier = 0;
        break;
      case ANCHOR_EventTypeWindowActivate: {
        ANCHOR_EventKeyData kdata;
        const int keymodifier = ((query_qual(SHIFT) ? KM_SHIFT : 0) |
                                 (query_qual(CONTROL) ? KM_CTRL : 0) |
                                 (query_qual(ALT) ? KM_ALT : 0) | (query_qual(OS) ? KM_OSKEY : 0));
        wm->winactive = win;
        win->active = true;

        kdata.ascii = '\0';
        kdata.utf8_buf[0] = '\0';

        if (win->eventstate->shift)
        {
          if ((keymodifier & KM_SHIFT) == 0)
          {
            kdata.key = ANCHOR_KeyLeftShift;
            WM_event_add_anchorevent(wm, win, ANCHOR_EventTypeKeyUp, &kdata);
          }
        }

        if (win->eventstate->ctrl)
        {
          if ((keymodifier & KM_CTRL) == 0)
          {
            kdata.key = ANCHOR_KeyLeftControl;
            WM_event_add_anchorevent(wm, win, ANCHOR_EventTypeKeyUp, &kdata);
          }
        }

        if (win->eventstate->alt)
        {
          if ((keymodifier & KM_ALT) == 0)
          {
            kdata.key = ANCHOR_KeyLeftAlt;
            WM_event_add_anchorevent(wm, win, ANCHOR_EventTypeKeyUp, &kdata);
          }
        }

        if (win->eventstate->oskey)
        {
          if ((keymodifier & KM_OSKEY) == 0)
          {
            kdata.key = ANCHOR_KeyOS;
            WM_event_add_anchorevent(wm, win, ANCHOR_EventTypeKeyUp, &kdata);
          }
        }

        /* keymodifier zero, it hangs on hotkeys that open windows otherwise */
        win->eventstate->keymodifier = 0;

        /* entering window, update mouse pos. but no event */
        wm_window_update_eventstate(win);

        win->addmousemove = true; /* enables highlighted buttons */

        wm_window_make_drawable(wm, win);

        wmEvent event;
        WM_event_init_from_window(win, &event);
        event.type = MOUSEMOVE;
        event.prev_mouse_pos = event.mouse_pos;
        event.is_repeat = false;
        wm_event_add(win, &event);
        break;
      }
      case ANCHOR_EventTypeWindowClose: {
        wm_window_close(C, wm, win);
        break;
      }
      case ANCHOR_EventTypeWindowUpdate: {
        wm_window_make_drawable(wm, win);
        WM_event_add_notifier(C, NC_WINDOW, NULL);
        break;
      }
      case ANCHOR_EventTypeWindowSize:
      case ANCHOR_EventTypeWindowMove: {
        eAnchorWindowState state = ANCHOR::GetWindowState((ANCHOR_SystemWindowHandle)win->anchorwin);
        win->windowstate = state;
        wm_window_set_dpi(win);
        break;
      }
      case ANCHOR_EventTypeWindowDPIHintChanged: {
        wm_window_set_dpi(win);

        WM_main_add_notifier(C, NC_WINDOW, NULL);             /* full redraw */
        WM_main_add_notifier(C, NC_SCREEN | NA_EDITED, NULL); /* refresh region sizes */
        break;
      }
      case ANCHOR_EventTypeOpenMainFile: {
        const char *path = (const char *)ANCHOR::GetEventData(evt);

        if (path)
        {
          wmOperatorType *ot = WM_operatortype_find(IDNAME(WM_OT_open_mainfile));
          CTX_wm_window_set(C, win);

          PointerUNI props_ptr;
          WM_operator_properties_create_ptr(&props_ptr, ot);
          CreationFactory::STR::Set(&props_ptr, "filepath", path);
          CreationFactory::BOOL::Set(&props_ptr, "display_file_selector", false);
          WM_operator_name_call_ptr(C, ot, WM_OP_INVOKE_DEFAULT, &props_ptr);
          WM_operator_properties_free(&props_ptr);

          CTX_wm_window_set(C, NULL);
        }
        break;
      }
      case ANCHOR_EventTypeDraggingDropDone: {
        Anchor_EventDragnDropData *ddd = (Anchor_EventDragnDropData *)ANCHOR::GetEventData(evt);

        /* entering window, update mouse pos */
        wm_window_update_eventstate(win);

        wmEvent event;
        WM_event_init_from_window(win, &event); /* copy last state, like mouse coords */

        /* activate region */
        event.type = MOUSEMOVE;
        event.prev_mouse_pos = event.mouse_pos;
        event.is_repeat = false;

        /* No context change! C->wm->windrawable is drawable, or for area queues. */
        wm->winactive = win;

        win->active = true;

        wm_event_add(win, &event);

        /* make kraken drop event with custom data pointing to wm drags */
        event.type = EVT_DROP;
        event.val = KM_RELEASE;
        event.custom = EVT_DATA_DRAGDROP;
        event.customdata = wm->drags.data();
        event.customdatafree = 1;

        wm_event_add(win, &event);

        /* printf("Drop detected\n"); */

        /* add drag data to wm for paths: */

        if (ddd->dataType == ANCHOR_DragnDropTypeFilenames)
        {
          ANCHOR_StringArray *stra = (ANCHOR_StringArray *)ddd->data;

          for (int a = 0; a < stra->count; a++)
          {
            printf("drop file %s\n", stra->strings[a]);
            /* try to get icon type from extension */
            int icon = ED_file_extension_icon((char *)stra->strings[a]);

            WM_event_start_drag(C, icon, WM_DRAG_PATH, stra->strings[a], 0.0, WM_DRAG_NOP);
            /* void poin should point to string, it makes a copy */
            break; /* only one drop element supported now */
          }
        }

        break;
      }
      case ANCHOR_EventTypeNativeResolutionChange: {
        float pixelsize = FormFactory(win->pixelsz);
        float prev_pixelsize = pixelsize;
        wm_window_set_dpi(win);

        if (pixelsize != prev_pixelsize)
        {
          // KKE_icon_changed(WM_window_get_active_screen(win)->id.icon_id);

          /* Close all popups since they are positioned with the pixel
           * size baked in and it's difficult to correct them. */
          CTX_wm_window_set(C, win);
          // UI_popup_handlers_remove_all(C, &win->modalhandlers);
          CTX_wm_window_set(C, NULL);

          wm_window_make_drawable(wm, win);
          WM_event_add_notifier(C, NC_SCREEN | NA_EDITED, NULL);
          WM_event_add_notifier(C, NC_WINDOW | NA_EDITED, NULL);
        }

        break;
      }
      default: {
        // WM_event_add_anchorevent(wm, win, type, data);
        break;
      }
    }
  }

  return 1;
}

static void wm_window_anchorwindow_add(wmWindowManager *wm, wmWindow *win, bool is_dialog)
{

  /* ----- */

  /**
   * This comes direct
   * from Pixar Stage. */

  TfToken win_title = FormFactory(win->title);
  SdfAssetPath win_icon = FormFactory(win->icon);
  GfVec2f win_pos = FormFactory(win->pos);
  GfVec2f win_size = FormFactory(win->size);

  /* ----- */

  ANCHOR_SystemWindowHandle anchorwin = ANCHOR::CreateSystemWindow(anchor_system,
                                                                   (win->parent) ? (ANCHOR_SystemWindowHandle)win->parent->anchorwin : NULL,
                                                                   CHARALL(win_title),
                                                                   CHARALL(win_icon.GetAssetPath()),
                                                                   GET_X(win_pos),
                                                                   GET_Y(win_pos),
                                                                   GET_X(win_size),
                                                                   GET_Y(win_size),
                                                                   ANCHOR_WindowStateFullScreen,
                                                                   is_dialog,
                                                                   ANCHOR_DrawingContextTypeVulkan,
                                                                   0);
  if (anchorwin)
  {
    win->anchorwin = anchorwin;
  }
}


static void wm_window_title(wmWindowManager *wm, wmWindow *win)
{
  if (WM_window_is_temp_screen(win)) {
    /* Nothing to do for 'temp' windows,
     * because #WM_window_open always sets window title. */
  }
  else if (win->anchorwin) {
    /* this is set to 1 if you don't have startup.usd open */
    // if (G.save_over && KKE_main_pixarfile_path_from_global()[0]) {
    //   char str[sizeof(((Main *)NULL)->name) + 24];
    //   KLI_snprintf(str,
    //                sizeof(str),
    //                "Kraken%s [%s%s]",
    //                wm->file_saved ? "" : "*",
    //                KKE_main_pixarfile_path_from_global(),
    //                G_MAIN->recovered ? " (Recovered)" : "");
    //   ANCHOR::SetTitle((ANCHOR_SystemWindowHandle)win->anchorwin, str);
    // }
    // else {
    ANCHOR::SetTitle((ANCHOR_SystemWindowHandle)win->anchorwin, "Kraken");
    // }

    /**
     * Informs ANCHOR of unsaved changes, to set window modified visual indicator and to
     * give hint of unsaved changes for a user warning mechanism in case of OS application
     * terminate request (e.g. OS Shortcut Alt+F4, Command+Q, (...), or session end). */
    // ANCHOR::SetWindowModifiedState(win->anchorwin, (AnchorU8)!wm->file_saved);
  }
}


static void wm_window_anchorwindow_ensure(wmWindowManager *wm, wmWindow *win, bool is_dialog)
{
  if (!win->anchorwin)
  {
    /* ----- */

    /**
     * This comes direct
     * from Pixar Stage. */

    TfToken win_title = FormFactory(win->title);
    SdfAssetPath win_icon = FormFactory(win->icon);
    TfToken win_cursor = FormFactory(win->cursor);
    GfVec2f win_pos = FormFactory(win->pos);
    GfVec2f win_size = FormFactory(win->size);

    /* ----- */

    if (GET_X(win_size) <= 0)
    {
      FormFactory(win->pos, GfVec2f(0.0, 0.0));
      FormFactory(win->size, GfVec2f(1920, 1080));
      FormFactory(win->state, UsdUITokens->maximized);
      FormFactory(win->alignment, UsdUITokens->alignAbsolute);

      if (win_cursor.IsEmpty())
      {
        FormFactory(win->cursor, UsdUITokens->default_);
      }

      if (win_title.IsEmpty())
      {
        FormFactory(win->title, TfToken("Kraken"));
      }

      if (win_icon.GetAssetPath().empty())
      {
        FormFactory(win->icon, SdfAssetPath(KLI_icon(ICON_KRAKEN)));
      }
    }

    wm_window_anchorwindow_add(wm, win, is_dialog);
  }

  if (win->anchorwin)
  {
    // wm_window_ensure_eventstate(win);
    wm_window_set_dpi(win);
  }

  /* add keymap handlers (1 handler for all keys in map!) */
  // wmKeyMap *keymap = WM_keymap_ensure(wm->defaultconf, "Window", 0, 0);
  // WM_event_add_keymap_handler(&win->handlers, keymap);

  // keymap = WM_keymap_ensure(wm->defaultconf, "Screen", 0, 0);
  // WM_event_add_keymap_handler(&win->handlers, keymap);

  // keymap = WM_keymap_ensure(wm->defaultconf, "Screen Editing", 0, 0);
  // WM_event_add_keymap_handler(&win->modalhandlers, keymap);

  /* add drop boxes */
  // {
    // ListBase *lb = WM_dropboxmap_find("Window", 0, 0);
    // WM_event_add_dropbox_handler(&win->handlers, lb);
  // }
  wm_window_title(wm, win);

  /* add topbar */
  // ED_screen_global_areas_refresh(win);  
}

static void wm_get_screensize(int *r_width, int *r_height)
{
  unsigned int uiwidth;
  unsigned int uiheight;

  ANCHOR::GetMainDisplayDimensions(anchor_system, &uiwidth, &uiheight);
  *r_width = uiwidth;
  *r_height = uiheight;
}


void WM_window_rect_calc(const wmWindow *win, GfRect2i *r_rect)
{
  r_rect->SetMinX(0);
  r_rect->SetMinY(0);
  r_rect->SetMaxX(WM_window_pixels_x(win));
  r_rect->SetMaxY(WM_window_pixels_y(win));
}


void WM_window_screen_rect_calc(const wmWindow *win, GfRect2i *r_rect)
{
  GfRect2i window_rect, screen_rect;

  WM_window_rect_calc(win, &window_rect);
  screen_rect = window_rect;

  /* Subtract global areas from screen rectangle. */
  UNIVERSE_FOR_ALL(global_area, win->global_areas.areas)
  {
    int height = ED_area_global_size_y(global_area) - 1;

    if (global_area->global->flag & GLOBAL_AREA_IS_HIDDEN)
    {
      continue;
    }

    switch (global_area->global->align)
    {
      case GLOBAL_AREA_ALIGN_TOP:
        screen_rect.SetMaxY(screen_rect.GetMaxY() - height);
        break;
      case GLOBAL_AREA_ALIGN_BOTTOM:
        screen_rect.SetMinY(screen_rect.GetMinY() - height);
        break;
      default:
        KLI_assert_unreachable();
        break;
    }
  }

  KLI_assert(screen_rect.IsValid());

  *r_rect = screen_rect;
}


void WM_window_anchorwindows_ensure(wmWindowManager *wm)
{
  UNIVERSE_FOR_ALL(win, wm->windows)
  {
    wm_window_anchorwindow_ensure(wm, VALUE(win), false);
  }
}


/* keeps size within monitor bounds */
static void wm_window_check_size(GfVec4i *rect)
{
  int width, height;
  wm_get_screensize(&width, &height);

  int xmin = GET_X(rect->GetArray());
  int ymin = GET_Y(rect->GetArray());
  int xmax = GET_Z(rect->GetArray());
  int ymax = GET_W(rect->GetArray());

  int sizex = (xmax - xmin);
  int sizey = (ymax - ymin);

  if (sizex > width)
  {
    int centx = (xmin + xmax) / 2;
    xmin = centx - (width / 2);
    xmax = xmin + width;
  }

  if (sizey > height)
  {
    int centy = (ymin + ymax) / 2;
    ymin = centy - (height / 2);
    ymax = ymin + height;
  }

  rect->Set(xmin, ymin, xmax, ymax);
}


/* size of all screens (desktop), useful since the mouse is bound by this */
void wm_get_desktopsize(int *r_width, int *r_height)
{
  unsigned int uiwidth;
  unsigned int uiheight;

  ANCHOR::GetAllDisplayDimensions(anchor_system, &uiwidth, &uiheight);
  *r_width = uiwidth;
  *r_height = uiheight;
}

void wm_window_set_size(wmWindow *win, int width, int height)
{
  ANCHOR::SetClientSize((ANCHOR_SystemWindowHandle)win->anchorwin, width, height);
}


static bool wm_window_update_size_position(wmWindow *win)
{
  ANCHOR_RectangleHandle client_rect = ANCHOR::GetClientBounds((ANCHOR_SystemWindowHandle)win->anchorwin);
  int l, t, r, b;
  ANCHOR::GetRectangle(client_rect, &l, &t, &r, &b);

  ANCHOR::DisposeRectangle(client_rect);

  GfVec2f size = FormFactory(win->size);
  GfVec2f pos = FormFactory(win->pos);

  int scr_w, scr_h;
  wm_get_desktopsize(&scr_w, &scr_h);
  int sizex = r - l;
  int sizey = b - t;
  int posx = l;
  int posy = scr_h - t - GET_Y(size);

  if (GET_X(size) != sizex || GET_Y(size) != sizey || GET_X(pos) != posx || GET_Y(pos) != posy)
  {
    FormFactory(win->size, GfVec2f(sizex, sizey));
    FormFactory(win->pos, GfVec2f(posx, posy));
    return true;
  }
  return false;
}

bool WM_window_is_temp_screen(const wmWindow *win)
{
  const cScreen *screen = WM_window_get_active_screen(win);
  return (screen && screen->temp != 0);
}


/**
 * @param space_type: SPACE_VIEW3D, SPACE_INFO, ... (eSpace_Type)
 * @param dialog: whether this should be made as a dialog-style window
 * @param temp: whether this is considered a short-lived window
 * @param alignment: how this window is positioned relative to its parent
 * @return the window or NULL in case of failure. */
wmWindow *WM_window_open(cContext *C,
                         const char *title,
                         const char *icon,
                         int x,
                         int y,
                         int sizex,
                         int sizey,
                         TfToken space_type,
                         TfToken alignment,
                         bool dialog,
                         bool temp)
{
  Main *cmain = CTX_data_main(C);
  wmWindowManager *wm = CTX_wm_manager(C);
  wmWindow *win_prev = CTX_wm_window(C);
  Scene *scene = CTX_data_scene(C);
  GfVec4i rect;

  GfVec2f pos = FormFactory(win_prev->pos);
  GfVec2f size = FormFactory(win_prev->size);

  const float native_pixel_size = ANCHOR::GetNativePixelSize((ANCHOR_SystemWindowHandle)win_prev->anchorwin);
  /* convert to native OS window coordinates */
  rect[0] = pos[0] + (x / native_pixel_size);
  rect[1] = pos[1] + (y / native_pixel_size);
  sizex /= native_pixel_size;
  sizey /= native_pixel_size;

  if (alignment == UsdUITokens->alignCenter)
  {
    /* Window centered around x,y location. */
    rect[0] -= sizex / 2;
    rect[1] -= sizey / 2;
  }
  else if (alignment == UsdUITokens->alignParent)
  {
    /* Centered within parent. X,Y as offsets from there. */
    rect[0] += (size[0] - sizex) / 2;
    rect[1] += (size[1] - sizey) / 2;
  }
  else
  {
    /* Positioned absolutely within parent bounds. */
  }

  rect[2] = rect[0] + sizex;
  rect[3] = rect[1] + sizey;

  /* changes rect to fit within desktop */
  wm_window_check_size(&rect);

  /* ----- */

  /* Reuse temporary windows when they share the same title. */
  wmWindow *win = POINTER_ZERO;
  if (temp)
  {
    UNIVERSE_FOR_ALL(win_iter, wm->windows)
    {
      if (WM_window_is_temp_screen(VALUE(win_iter)))
      {
        char *wintitle = ANCHOR::GetTitle((ANCHOR_SystemWindowHandle)VALUE(win_iter)->anchorwin);
        if (STREQ(title, wintitle))
        {
          win = VALUE(win_iter);
        }
        free(wintitle);
      }
    }
  }

  /* ----- */

  /**
   * Create Window. */
  if (win == POINTER_ZERO)
  {
    win = wm_window_new(C, wm, win_prev, dialog);
    FormFactory(win->pos, GfVec2f(GET_X(rect), GET_Y(rect)));
  }

  /* ----- */

  cScreen *screen = WM_window_get_active_screen(win);

  FormFactory(win->size, GfVec2f(GET_Z(rect) - GET_X(rect), GET_W(rect) - GET_Y(rect)));

  if (WM_window_get_active_workspace(win) == POINTER_ZERO)
  {
    WorkSpace *workspace = WM_window_get_active_workspace(win_prev);
    KKE_workspace_active_set(win->workspace_hook, workspace);
  }

  if (screen == POINTER_ZERO)
  {
    /* add new screen layout */
    WorkSpace *workspace = WM_window_get_active_workspace(win);
    WorkSpaceLayout *layout = ED_workspace_layout_add(C, workspace, win, "temp");

    screen = KKE_workspace_layout_screen_get(layout);
    WM_window_set_active_layout(win, workspace, layout);
  }

  screen->temp = temp;

  CTX_wm_window_set(C, win);
  const bool new_window = (win->anchorwin == POINTER_ZERO);
  if (new_window)
  {
    wm_window_anchorwindow_ensure(wm, win, dialog);
  }
  WM_check(C);

  /* ensure it shows the right spacetype editor */
  if (space_type != UsdUITokens->spaceEmpty)
  {
    ScrArea *area = screen->areas.at(0);
    CTX_wm_area_set(C, area);
    ED_area_newspace(C, area, space_type, false);
  }

  ED_screen_change(C, screen);

  if (!new_window)
  {
    /**
     * Set size in ANCHOR window and then update size and position from
     * ANCHOR, in case they where changed by ANCHOR to fit the monitor. */
    GfVec2f new_size = FormFactory(win->size);
    wm_window_set_size(win, GET_X(new_size), GET_Y(new_size));
    wm_window_update_size_position(win);
  }

  return win;
}


static void wm_close_file_dialog(cContext *C, wmGenericCallback *post_action)
{
  /**
   * TODO. */

  post_action->free_user_data(post_action->user_data);

  delete post_action;
}


void wm_exit_schedule_delayed(cContext *C)
{
  wmWindow *win = CTX_wm_window(C);

  /**
   * TODO. */

  exit(KRAKEN_SUCCESS);
}


static void wm_save_file_on_quit_dialog_callback(cContext *C, void *UNUSED(user_data))
{
  wm_exit_schedule_delayed(C);
}

static void wm_confirm_quit(cContext *C)
{
  /** Save User's Work. */
  TF_DEBUG(KRAKEN_DEBUG_OPERATORS).Msg("WARNING: Saving without a confirmation Dialog. Overwriting existing changes.");

  Stage stage = CTX_data_stage(C);
  if (stage->GetRootLayer()->Save())
  {
    wmGenericCallback *action = new wmGenericCallback;
    action->exec = (wmGenericCallbackFn)wm_save_file_on_quit_dialog_callback;
    wm_close_file_dialog(C, action);
  }
}


static void wm_window_raise(wmWindow *win)
{
  /* Restore window if minimized */
  if (ANCHOR::GetWindowState((ANCHOR_SystemWindowHandle)win->anchorwin) == ANCHOR_WindowStateMinimized)
  {
    ANCHOR::SetWindowState((ANCHOR_SystemWindowHandle)win->anchorwin, ANCHOR_WindowStateNormal);
  }
  ANCHOR::SetWindowOrder((ANCHOR_SystemWindowHandle)win->anchorwin, ANCHOR_kWindowOrderTop);
}


static void wm_window_desktop_pos_get(wmWindow *win,
                                      const GfVec2f screen_pos,
                                      GfVec2i *r_desk_pos)
{
  float win_pixelsz = FormFactory(win->pixelsz);
  GfVec2f win_pos = FormFactory(win->pos);

  /* To desktop space. */
  VEC2_SET(r_desk_pos,
           (GET_X(screen_pos) + (int)(win_pixelsz * GET_X(win_pos))),
           (GET_Y(screen_pos) + (int)(win_pixelsz * GET_Y(win_pos))));
}

static void wm_window_screen_pos_get(wmWindow *win,
                                     const GfVec2i desktop_pos,
                                     GfVec2i *r_scr_pos)
{
  float win_pixelsz = FormFactory(win->pixelsz);
  GfVec2f win_pos = FormFactory(win->pos);

  /* To window space. */
  VEC2_SET(r_scr_pos,
           (GET_X(desktop_pos) - (int)(win_pixelsz * GET_X(win_pos))),
           (GET_Y(desktop_pos) - (int)(win_pixelsz * GET_Y(win_pos))));
}


bool WM_window_find_under_cursor(wmWindowManager *wm,
                                 wmWindow *win_ignore,
                                 wmWindow *win,
                                 const GfVec2i mval,
                                 wmWindow **r_win,
                                 GfVec2i *r_mval)
{
  GfVec2i desk_pos;
  wm_window_desktop_pos_get(win, mval, &desk_pos);

  /* TODO: This should follow the order of the activated windows.
   * The current solution is imperfect but usable in most cases. */
  UNIVERSE_FOR_ALL(win_iter, wm->windows)
  {
    if (VALUE(win_iter) == win_ignore)
    {
      continue;
    }

    TfToken win_state = FormFactory(win->state);

    if (win_state == UsdUITokens->minimized)
    {
      continue;
    }

    GfVec2i scr_pos;
    wm_window_screen_pos_get(VALUE(win_iter), desk_pos, &scr_pos);

    GfVec2f win_pos = FormFactory(win->pos);

    if (GET_X(scr_pos) >= 0 && GET_Y(win_pos) >= 0 &&
        GET_X(scr_pos) <= WM_window_pixels_x(VALUE(win_iter)) &&
        GET_Y(scr_pos) <= WM_window_pixels_y(VALUE(win_iter)))
    {
      *r_win = VALUE(win_iter);

      VEC2_SET(r_mval, GET_X(scr_pos), GET_Y(scr_pos));
      return true;
    }
  }

  return false;
}


int WM_window_pixels_x(const wmWindow *win)
{
  float f = ANCHOR::GetNativePixelSize((ANCHOR_SystemWindowHandle)win->anchorwin);

  GfVec2f win_size = FormFactory(win->size);

  return (int)(f * GET_X(win_size));
}

int WM_window_pixels_y(const wmWindow *win)
{
  float f = ANCHOR::GetNativePixelSize((ANCHOR_SystemWindowHandle)win->anchorwin);

  GfVec2f win_size = FormFactory(win->size);

  return (int)(f * GET_Y(win_size));
}


void wm_quit_with_optional_confirmation_prompt(cContext *C, wmWindow *win)
{
  wmWindow *win_ctx = CTX_wm_window(C);

  Stage stage = CTX_data_stage(C);
  UserDef *uprefs = CTX_data_prefs(C);

  /* The popup will be displayed in the context window which may not be set
   * here (this function gets called outside of normal event handling loop). */
  CTX_wm_window_set(C, win);

  bool show_save = FormFactory(uprefs->showsave);

  if (show_save)
  {
    if (stage->GetRootLayer()->IsDirty())
    {
      wm_window_raise(win);
      wm_confirm_quit(C);
    }
    else
    {
      wm_exit_schedule_delayed(C);
    }
  }
  else
  {
    wm_exit_schedule_delayed(C);
  }

  CTX_wm_window_set(C, win_ctx);
}


/* this is event from anchor, or exit-kraken op */
void wm_window_close(cContext *C, wmWindowManager *wm, wmWindow *win)
{
  Stage stage = CTX_data_stage(C);

  SdfPath other_hash;

  /* First check if there is another main window remaining. */
  UNIVERSE_FOR_ALL(win_other, wm->windows)
  {
    if (VALUE(win_other) != win && VALUE(win_other)->parent == NULL)
    {
      other_hash = HASH(win_other);
      break;
    }
  }

  if (win->parent == NULL)
  {
    wm_quit_with_optional_confirmation_prompt(C, win);
    return;
  }

  /* Close child windows */
  UNIVERSE_FOR_ALL(iter_win, wm->windows)
  {
    if (VALUE(iter_win)->parent == win)
    {
      wm_window_close(C, wm, VALUE(iter_win));
    }
  }

  cScreen *screen = WM_window_get_active_screen(win);
  WorkSpace *workspace = WM_window_get_active_workspace(win);
  WorkSpaceLayout *layout = KKE_workspace_active_layout_get(win->workspace_hook);

  /** Remove Window From HashMap */
  wm->windows.erase(win->path);

  CTX_wm_window_set(C, win); /* needed by handlers */
  // WM_event_remove_handlers(C, &win->handlers);
  // WM_event_remove_handlers(C, &win->modalhandlers);

  /** Remove All Screens. */
  if (screen)
  {
    // ED_screen_exit(C, win, screen);
  }

  /** Null out C. */
  if (CTX_wm_window(C) == win)
  {
    CTX_wm_window_set(C, NULL);
  }

  delete win;
}


static SdfPath make_winpath(int id)
{
  return SdfPath(STRINGALL(KRAKEN_PATH_DEFAULTS::KRAKEN_WINDOW) + STRINGALL(id));
}


static int find_free_winid(wmWindowManager *wm)
{
  int id = 1;

  UNIVERSE_FOR_ALL(win, wm->windows)
  {
    if (id <= VALUE(win)->winid)
    {
      id = VALUE(win)->winid + 1;
    }
  }
  return id;
}


wmWindow *wm_window_new(cContext *C, wmWindowManager *wm, wmWindow *parent, bool dialog)
{
  int id = find_free_winid(wm);
  wmWindow *win = new wmWindow(C, make_winpath(id));
  UNIVERSE_INSERT_WINDOW(wm, win->path, win);
  win->winid = id;

  const Main *cmain = CTX_data_main(C);

  /* Dialogs may have a child window as parent. Otherwise, a child must not be a parent too. */
  win->parent = (!dialog && parent && parent->parent) ? parent->parent : parent;
  win->workspace_hook = KKE_workspace_instance_hook_create(cmain, win->winid);

  return win;
}


wmWindow *wm_window_copy(cContext *C,
                         wmWindowManager *wm,
                         wmWindow *win_src,
                         const bool duplicate_layout,
                         const bool child)
{
  const bool is_dialog = ANCHOR::IsDialogWindow((ANCHOR_SystemWindowHandle)win_src->anchorwin);
  wmWindow *win_parent = (child) ? win_src : win_src->parent;
  wmWindow *win_dst = wm_window_new(C, wm, win_parent, is_dialog);
  WorkSpace *workspace = WM_window_get_active_workspace(win_src);
  WorkSpaceLayout *layout_old = WM_window_get_active_layout(win_src);

  GfVec2f win_srcpos = FormFactory(win_src->pos);
  GfVec2f win_srcsize = FormFactory(win_src->size);

  win_dst->pos.Set(GfVec2f(GET_X(win_srcpos) + 10, GET_Y(win_srcpos)));
  win_dst->size.Set(GfVec2f(GET_X(win_srcsize), GET_Y(win_srcsize)));

  win_dst->scene = win_src->scene;
  KKE_workspace_active_set(win_dst->workspace_hook, workspace);

  /** 
   * TODO: Duplicate layouts */
  WorkSpaceLayout *layout_new = layout_old;

  return win_dst;
}


wmWindow *wm_window_copy_test(cContext *C,
                              wmWindow *win_src,
                              const bool duplicate_layout,
                              const bool child)
{
  Main *cmain = CTX_data_main(C);
  wmWindowManager *wm = CTX_wm_manager(C);

  wmWindow *win_dst = wm_window_copy(C, wm, win_src, duplicate_layout, child);

  WM_check(C);

  if (win_dst->anchorwin)
  {
    WM_event_add_notifier_ex(wm, CTX_wm_window(C), NC_WINDOW | NA_ADDED, NULL);
    return win_dst;
  }
  wm_window_close(C, wm, win_dst);
  return NULL;
}


static int wm_window_close_exec(cContext *C, wmOperator *UNUSED(op))
{
  wmWindowManager *wm = CTX_wm_manager(C);
  wmWindow *win = CTX_wm_window(C);
  wm_window_close(C, wm, win);
  return OPERATOR_FINISHED;
}


static int wm_window_new_exec(cContext *C, wmOperator *UNUSED(op))
{
  wmWindow *win_src = CTX_wm_window(C);
  ScrArea *area = KKE_screen_find_big_area(CTX_wm_screen(C), SPACE_TYPE_ANY, 0);

  SdfAssetPath icon = FormFactory(win_src->icon);
  GfVec2f size = FormFactory(win_src->size);

  TfToken spacetype = FormFactory(area->spacetype);

  bool ok = (WM_window_open(C,
                            IFACE_("Kraken"),
                            CHARALL(icon.GetAssetPath()),
                            0,
                            0,
                            GET_X(size) * 0.95f,
                            GET_Y(size) * 0.9f,
                            spacetype,
                            UsdUITokens->alignParent,
                            false,
                            false) != NULL);

  return ok ? OPERATOR_FINISHED : OPERATOR_CANCELLED;
}


void WM_anchor_init(cContext *C)
{
  /* Event handle of anchor stack. */
  ANCHOR_EventConsumerHandle consumer;

  if (C != NULL)
  {
    consumer = ANCHOR_CreateEventConsumer(anchor_event_proc, C);
  }

  if (!anchor_system)
  {
    anchor_system = ANCHOR_CreateSystem();
  }

  if (C != NULL)
  {
    ANCHOR::AddEventConsumer(anchor_system, consumer);
  }
}


void WM_window_process_events(cContext *C)
{
  bool has_event = ANCHOR::ProcessEvents(anchor_system, false);

  if (has_event)
  {
    ANCHOR::DispatchEvents(anchor_system);
  }

  if ((has_event == false))
  {
    PIL_sleep_ms(5);
  }
}


void WM_window_swap_buffers(wmWindow *win)
{
  ANCHOR::SwapChain((ANCHOR_SystemWindowHandle)win->anchorwin);
}


/**
 *  -----  The Window Operators. ----- */


static int wm_window_new_main_exec(cContext *C, wmOperator *UNUSED(op))
{
  wmWindow *win_src = CTX_wm_window(C);

  bool ok = (wm_window_copy_test(C, win_src, true, false) != NULL);

  return ok ? OPERATOR_FINISHED : OPERATOR_CANCELLED;
}


static int wm_window_fullscreen_toggle_exec(cContext *C, wmOperator *UNUSED(op))
{
  wmWindow *window = CTX_wm_window(C);

  eAnchorWindowState state = ANCHOR::GetWindowState((ANCHOR_SystemWindowHandle)window->anchorwin);
  if (state != ANCHOR_WindowStateFullScreen)
  {
    ANCHOR::SetWindowState((ANCHOR_SystemWindowHandle)window->anchorwin, ANCHOR_WindowStateFullScreen);
  }
  else
  {
    ANCHOR::SetWindowState((ANCHOR_SystemWindowHandle)window->anchorwin, ANCHOR_WindowStateNormal);
  }

  return OPERATOR_FINISHED;
}


static bool wm_operator_winactive(cContext *C)
{
  if (CTX_wm_window(C) == NULL)
  {
    return 0;
  }
  return 1;
}


static bool wm_operator_winactive_normal(cContext *C)
{
  wmWindow *win = CTX_wm_window(C);
  cScreen *screen;

  if (win == NULL)
  {
    return 0;
  }
  if (!((screen = WM_window_get_active_screen(win))))
  {
    return 0;
  }

  return 1;
}

static int wm_exit_kraken_exec(cContext *C, wmOperator *UNUSED(op))
{
  wm_exit_schedule_delayed(C);
  return OPERATOR_FINISHED;
}


static int wm_exit_kraken_invoke(cContext *C, wmOperator *UNUSED(op), wmEvent *UNUSED(event))
{
  UserDef *uprefs = CTX_data_prefs(C);

  bool prompt_save = FormFactory(uprefs->showsave);

  if (prompt_save)
  {
    wm_quit_with_optional_confirmation_prompt(C, CTX_wm_window(C));
  }
  else
  {
    wm_exit_schedule_delayed(C);
  }
  return OPERATOR_FINISHED;
}


static void WM_OT_window_close(wmOperatorType *ot)
{
  ot->name = "Close Window";
  ot->idname = IDNAME(WM_OT_window_close);
  ot->description = "Close the current window";

  ot->exec = wm_window_close_exec;
  ot->poll = wm_operator_winactive;
}


static void WM_OT_window_new(wmOperatorType *ot)
{
  ot->name = "New Window";
  ot->idname = IDNAME(WM_OT_window_new);
  ot->description = "Create a new window";

  ot->exec = wm_window_new_exec;
  ot->poll = wm_operator_winactive_normal;
}


static void WM_OT_window_new_main(wmOperatorType *ot)
{
  ot->name = "New Main Window";
  ot->idname = IDNAME(WM_OT_window_new_main);
  ot->description = "Create a new main window with its own workspace and scene selection";

  ot->exec = wm_window_new_main_exec;
  ot->poll = wm_operator_winactive_normal;
}


static void WM_OT_window_fullscreen_toggle(wmOperatorType *ot)
{
  ot->name = "Toggle Window Fullscreen";
  ot->idname = IDNAME(WM_OT_window_fullscreen_toggle);
  ot->description = "Toggle the current window fullscreen";

  ot->exec = wm_window_fullscreen_toggle_exec;
  ot->poll = wm_operator_winactive;
}


static void WM_OT_quit_kraken(wmOperatorType *ot)
{
  ot->name = "Quit Kraken";
  ot->idname = IDNAME(WM_OT_quit_kraken);
  ot->description = "Quit Kraken";

  ot->invoke = wm_exit_kraken_invoke;
  ot->exec = wm_exit_kraken_exec;
}


void WM_window_operators_register()
{
  /* ------ */

  WM_operatortype_append(WM_OT_window_close);
  WM_operatortype_append(WM_OT_window_new);
  WM_operatortype_append(WM_OT_window_new_main);
  WM_operatortype_append(WM_OT_window_fullscreen_toggle);
  WM_operatortype_append(WM_OT_quit_kraken);

  /* ------ */
}

WABI_NAMESPACE_END