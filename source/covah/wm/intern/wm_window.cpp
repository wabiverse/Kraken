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
#include "WM_inline_tools.h"
#include "WM_operators.h"
#include "WM_tokens.h"

#include "UNI_area.h"
#include "UNI_context.h"
#include "UNI_object.h"
#include "UNI_userpref.h"
#include "UNI_window.h"
#include "UNI_workspace.h"

#include "ANCHOR_api.h"
#include "ANCHOR_event_consumer.h"
#include "ANCHOR_system.h"

#include "CKE_context.h"
#include "CKE_main.h"

#include "CLI_icons.h"
#include "CLI_math_inline.h"
#include "CLI_string_utils.h"
#include "CLI_time.h"

#include <wabi/base/gf/vec2f.h>

WABI_NAMESPACE_BEGIN


/* handle to anchor system. */
static ANCHOR_SystemHandle anchor_system;

/**
 * This is called by anchor, and this is where
 * we handle events for windows or send them to
 * the event system. */
static int anchor_event_proc(ANCHOR_EventHandle evt, ANCHOR_UserPtr C_void_ptr)
{
  cContext C = TfCreateRefPtr((CovahContext *)C_void_ptr);
  wmWindowManager wm = CTX_wm_manager(C);
  eAnchorEventType type = ANCHOR::GetEventType(evt);

  if (type == ANCHOR_EventTypeQuitRequest)
  {
    ANCHOR_SystemWindowHandle anchorwin = ANCHOR::GetEventWindow(evt);
    wmWindow win;
    if (anchorwin && ANCHOR::ValidWindow(anchor_system, anchorwin))
    {
      win = TfCreateRefPtr((CovahWindow *)ANCHOR::GetWindowUserData(anchorwin));
    }
  }
  else
  {
    ANCHOR_SystemWindowHandle anchorwin = ANCHOR::GetEventWindow(evt);
    ANCHOR_EventDataPtr data = ANCHOR::GetEventData(evt);
  }

  return COVAH_SUCCESS;
}


static void wm_window_set_dpi(const wmWindow win)
{
  float uscale;
  win->scale.Get(&uscale);

  float ulinewidth;
  win->linewidth.Get(&ulinewidth);

  float auto_dpi = ANCHOR::GetDPIHint((ANCHOR_SystemWindowHandle)win->anchorwin);

  auto_dpi = max_ff(auto_dpi, 96.0f);
  auto_dpi *= ANCHOR::GetNativePixelSize((ANCHOR_SystemWindowHandle)win->anchorwin);
  int dpi = auto_dpi * uscale * (72.0 / 96.0f);

  int pixelsize = max_ii(1, (int)(dpi / 64));
  pixelsize = max_ii(1, pixelsize + ulinewidth);

  float dpiadj = dpi / pixelsize;
  float dpifac = (pixelsize * (float)(dpiadj)) / 72.0f;
  float wunit = (pixelsize * (dpiadj / pixelsize) * 20 + 36) / 72;

  /* ----- */

  /**
   * Set prefs on
   * Pixar Stage. */

  win->pixelsz.Set(pixelsize);
  win->dpi.Set(dpiadj);
  win->dpifac.Set(dpifac);
  win->widgetunit.Set(wunit += 2 * ((int)pixelsize - (int)dpifac));

  /* ----- */

  /* update font drawing */
  ANCHOR::GetIO().FontGlobalScale = pixelsize * dpiadj;
}


static void wm_window_anchorwindow_add(wmWindowManager wm, wmWindow win, bool is_dialog)
{

  /* ----- */

  /**
   * This comes direct
   * from Pixar Stage. */

  TfToken title;
  win->title.Get(&title);

  SdfAssetPath icon;
  win->icon.Get(&icon);

  GfVec2f pos;
  win->pos.Get(&pos);

  GfVec2f size;
  win->size.Get(&size);

  /* ----- */

  ANCHOR_SystemWindowHandle anchorwin = ANCHOR::CreateWindow(anchor_system,
                                                             NULL,
                                                             title.GetText(),
                                                             icon.GetAssetPath().c_str(),
                                                             pos[0],
                                                             pos[1],
                                                             size[0],
                                                             size[1],
                                                             ANCHOR_WindowStateNormal,
                                                             is_dialog,
                                                             ANCHOR_DrawingContextTypeVulkan,
                                                             0);
  if (anchorwin)
  {
    win->anchorwin = anchorwin;
  }
}


static void wm_window_anchorwindow_ensure(wmWindowManager wm, wmWindow win, bool is_dialog)
{
  if (win->anchorwin == NULL)
  {

    /* ----- */

    /**
     * This comes direct
     * from Pixar Stage. */

    GfVec2f pos;
    win->pos.Get(&pos);

    GfVec2f size;
    win->size.Get(&size);

    TfToken title;
    win->title.Get(&title);

    SdfAssetPath icon;
    win->icon.Get(&icon);

    TfToken cursor;
    win->cursor.Get(&cursor);

    /* ----- */

    if ((size[0] == 0))
    {
      win->pos.Set(GfVec2f(0, 0));
      win->size.Set(GfVec2f(1920, 1080));

      if (cursor.IsEmpty())
      {
        win->cursor.Set(UsdUITokens->default_);
      }

      if (title.IsEmpty())
      {
        win->title.Set("Covah");
      }

      if (icon.GetAssetPath().empty())
      {
        win->icon.Set(CLI_icon(ICON_COVAH));
      }
    }

    wm_window_anchorwindow_add(wm, win, is_dialog);
    wm_window_set_dpi(win);
  }
}


static void wm_get_screensize(int *r_width, int *r_height)
{
  unsigned int uiwidth;
  unsigned int uiheight;

  ANCHOR::GetMainDisplayDimensions(anchor_system, &uiwidth, &uiheight);
  *r_width = uiwidth;
  *r_height = uiheight;
}


/* keeps size within monitor bounds */
static void wm_window_check_size(GfVec4i *rect)
{
  int width, height;
  wm_get_screensize(&width, &height);

  int xmin = rect->GetArray()[0];
  int ymin = rect->GetArray()[1];
  int xmax = rect->GetArray()[2];
  int ymax = rect->GetArray()[3];

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


/**
 * @param space_type: SPACE_VIEW3D, SPACE_INFO, ... (eSpace_Type)
 * @param dialog: whether this should be made as a dialog-style window
 * @param temp: whether this is considered a short-lived window
 * @param alignment: how this window is positioned relative to its parent
 * @return the window or NULL in case of failure. */
wmWindow WM_window_open(const cContext &C,
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
  Main cmain = CTX_data_main(C);
  wmWindowManager wm = CTX_wm_manager(C);
  wmWindow win_prev = CTX_wm_window(C);
  Scene scene = CTX_data_scene(C);
  GfVec4i rect;

  GfVec2f pos;
  win_prev->pos.Get(&pos);

  GfVec2f size;
  win_prev->pos.Get(&size);

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

  /**
   * Create Window. */
  wmWindow win = TfCreateRefPtr(new CovahWindow(C, win_prev, SdfPath("Child")));
  wm->windows.insert(std::make_pair(win->path, win));

  /**
   * Dialogs may have a child window as parent.
   * Otherwise, a child must not be a parent too. */
  win->parent = (!dialog && win_prev && win_prev->parent) ? win_prev->parent : win_prev;

  /* ----- */

  win->pos.Set(GfVec2f(rect[0], rect[1]));
  win->size.Set(GfVec2f(rect[2] - rect[0], rect[3] - rect[1]));

  if (!win->prims.workspace->GetPrim().IsValid())
  {
    win->prims.workspace = win_prev->prims.workspace;
  }

  if (!win->prims.screen->areas_rel.HasAuthoredTargets())
  {
    /* add new screen layout */
  }

  CTX_wm_window_set(C, win);
  const bool new_window = (win->anchorwin == NULL);
  if (new_window)
  {
    wm_window_anchorwindow_ensure(wm, win, dialog);
  }

  return win;
}


static void wm_close_file_dialog(const cContext &C, wmGenericCallback *post_action)
{
  /**
   * TODO. */

  post_action->free_user_data(post_action->user_data);

  delete post_action;
}


void wm_exit_schedule_delayed(const cContext &C)
{
  wmWindow win = CTX_wm_window(C);

  /**
   * TODO. */

  return;
}


static void wm_save_file_on_quit_dialog_callback(const cContext &C, void *UNUSED(user_data))
{
  wm_exit_schedule_delayed(C);
}


static void wm_confirm_quit(const cContext &C)
{
  wmGenericCallback *action = new wmGenericCallback;
  action->exec = (wmGenericCallbackFn)wm_save_file_on_quit_dialog_callback;
  wm_close_file_dialog(C, action);
}


static void wm_window_raise(const wmWindow &win)
{
  /* Restore window if minimized */
  if (ANCHOR::GetWindowState((ANCHOR_SystemWindowHandle)win->anchorwin) == ANCHOR_WindowStateMinimized)
  {
    ANCHOR::SetWindowState((ANCHOR_SystemWindowHandle)win->anchorwin, ANCHOR_WindowStateNormal);
  }
  ANCHOR::SetWindowOrder((ANCHOR_SystemWindowHandle)win->anchorwin, ANCHOR_kWindowOrderTop);
}


void wm_quit_with_optional_confirmation_prompt(const cContext &C, const wmWindow &win)
{
  wmWindow win_ctx = CTX_wm_window(C);

  Stage stage = CTX_data_stage(C);
  UserDef uprefs = CTX_data_uprefs(C);

  /* The popup will be displayed in the context window which may not be set
   * here (this function gets called outside of normal event handling loop). */
  CTX_wm_window_set(C, win);

  bool showsave;
  uprefs->showsave.Get(&showsave);

  if (showsave)
  {
    if (stage->GetSessionLayer()->IsDirty())
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


/* this is event from anchor, or exit-covah op */
void wm_window_close(const cContext &C, const wmWindowManager &wm, const wmWindow &win)
{
  Stage stage = CTX_data_stage(C);

  SdfPath other_hash;

  /* First check if there is another main window remaining. */
  TF_FOR_ALL (win_other, wm->windows)
  {
    if (win_other->second != win && win_other->second->parent == NULL)
    {
      other_hash = win_other->first;
      break;
    }
  }

  if (win->parent == NULL && other_hash.IsEmpty())
  {
    wm_quit_with_optional_confirmation_prompt(C, win);
    return;
  }

  /* Close child windows */
  TF_FOR_ALL (iter_win, wm->windows)
  {
    if (iter_win->second->parent == win)
    {
      wm_window_close(C, wm, iter_win->second);
    }
  }

  /** Remove Window From HashMap */
  wm->windows.erase(win->path);

  /** Remove All Areas from Screens. */
  if (win->prims.screen)
  {
    SdfPathVector areas;
    win->prims.screen->areas_rel.GetTargets(&areas);
    TF_FOR_ALL (area, areas)
    {
      UsdPrim areaprim = stage->GetPrimAtPath(area->GetPrimPath());
      areaprim.Unload();
    }

    SdfPathVector screens;
    win->prims.workspace->screen_rel.GetTargets(&screens);
    TF_FOR_ALL (screen, screens)
    {
      UsdPrim screenprim = stage->GetPrimAtPath(screen->GetPrimPath());
      screenprim.Unload();
    }
  }

  /** Null out C. */
  if (CTX_wm_window(C) == win)
  {
    CTX_wm_window_set(C, NULL);
  }

  win.~TfRefPtr();
  delete &win;
}


wmWindow wm_window_copy(cContext C,
                        wmWindowManager wm,
                        wmWindow win_src,
                        const bool duplicate_layout,
                        const bool child)
{
  const bool is_dialog = ANCHOR::IsDialogWindow((ANCHOR_SystemWindowHandle)win_src->anchorwin);
  wmWindow win_parent = (child) ? win_src : win_src->parent;

  /* ----- */

  /**
   * Create Window. */
  wmWindow win_dst = TfCreateRefPtr(new CovahWindow(C, win_parent, SdfPath("Child")));
  wm->windows.insert(std::make_pair(win_dst->path, win_dst));

  /**
   * Dialogs may have a child window as parent.
   * Otherwise, a child must not be a parent too. */
  win_dst->parent = (!is_dialog && win_parent && win_parent->parent) ? win_parent->parent : win_parent;

  /* ----- */

  Workspace workspace = win_src->prims.workspace;

  GfVec2f srcpos;
  win_src->pos.Get(&srcpos);

  GfVec2f srcsize;
  win_src->size.Get(&srcsize);

  win_dst->pos.Set(GfVec2f(srcpos[0] + 10, srcpos[1]));
  win_dst->size.Set(GfVec2f(srcsize[2], srcsize[3]));

  win_dst->scene = win_src->scene;
  STRNCPY(win_dst->view_layer_name, win_src->view_layer_name);
  win_dst->workspace_rel.AddTarget(workspace->path);

  return win_dst;
}


wmWindow wm_window_copy_test(const cContext &C,
                             const wmWindow &win_src,
                             const bool duplicate_layout,
                             const bool child)
{
  Main cmain = CTX_data_main(C);
  wmWindowManager wm = CTX_wm_manager(C);

  wmWindow win_dst = wm_window_copy(C, wm, win_src, duplicate_layout, child);

  if (win_dst->anchorwin)
  {
    // WM_event_add_notifier_ex(wm, CTX_wm_window(C), NC_WINDOW | NA_ADDED, NULL);
    return win_dst;
  }
  wm_window_close(C, wm, win_dst);
  return NULL;
}


static int wm_window_close_exec(const cContext &C, UsdAttribute &UNUSED(op))
{
  wmWindowManager wm = CTX_wm_manager(C);
  wmWindow win = CTX_wm_window(C);
  wm_window_close(C, wm, win);
  return OPERATOR_FINISHED;
}


static int wm_window_new_exec(const cContext &C, UsdAttribute &UNUSED(op))
{
  Stage stage = CTX_data_stage(C);
  wmWindow win_src = CTX_wm_window(C);

  GfVec2f size;
  win_src->size.Get(&size);

  TfToken align;
  win_src->alignment.Get(&align);

  SdfPathVector areas;
  win_src->prims.screen->areas_rel.GetTargets(&areas);

  SdfAssetPath icon;
  win_src->icon.Get(&icon);

  TfToken spacetype;
  TF_FOR_ALL (sdf_area, areas)
  {
    auto area = CovahArea::Get(stage, sdf_area->GetPrimPath());
    UsdAttribute type = area.GetSpacetypeAttr();

    TfToken possibletype;
    type.Get(&possibletype);

    spacetype = wm_verify_spacetype(possibletype);
  }

  bool ok = (WM_window_open(C,
                            IFACE_("Covah"),
                            icon.GetAssetPath().c_str(),
                            0,
                            0,
                            size[2] * 0.95f,
                            size[3] * 0.9f,
                            spacetype,
                            align,
                            false,
                            false) != NULL);

  return ok ? OPERATOR_FINISHED : OPERATOR_CANCELLED;
}


void WM_anchor_init(cContext C)
{
  /* Event handle of anchor stack. */
  ANCHOR_EventConsumerHandle consumer;

  if (C != NULL)
  {
    consumer = ANCHOR_CreateEventConsumer(anchor_event_proc, &C);
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


void WM_window_process_events(const cContext &C)
{
  bool has_event = ANCHOR::ProcessEvents(anchor_system, false);

  if (has_event)
  {
    ANCHOR::DispatchEvents(anchor_system);
  }

  if ((has_event == false))
  {
    printf("Quick sleep: No Events on Stack\n");
    PIL_sleep_ms(5);
  }
}


void WM_window_swap_buffers(wmWindow win)
{
  ANCHOR::SwapChain((ANCHOR_SystemWindowHandle)win->anchorwin);
}


/**
 *  -----  The Window Operators. ----- */


static int wm_window_new_main_exec(const cContext &C, UsdAttribute &UNUSED(op))
{
  wmWindow win_src = CTX_wm_window(C);

  bool ok = (wm_window_copy_test(C, win_src, true, false) != NULL);

  return ok ? OPERATOR_FINISHED : OPERATOR_CANCELLED;
}


static int wm_window_fullscreen_toggle_exec(const cContext &C, UsdAttribute &UNUSED(op))
{
  wmWindow window = CTX_wm_window(C);

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


static bool wm_operator_winactive(const cContext &C)
{
  if (CTX_wm_window(C) == NULL)
  {
    return 0;
  }
  return 1;
}


static bool wm_operator_winactive_normal(const cContext &C)
{
  wmWindow win = CTX_wm_window(C);

  if (win == NULL)
  {
    return 0;
  }

  if (!(win->prims.screen))
  {
    return 0;
  }

  // TfToken alignment;
  // win->prims.screen->align.Get(&alignment);
  // if (!(alignment == UsdUITokens->none)) {
  //   return 0;
  // }

  return 1;
}


static int wm_exit_covah_exec(const cContext &C, UsdAttribute &UNUSED(op))
{
  wm_exit_schedule_delayed(C);
  return OPERATOR_FINISHED;
}


static int wm_exit_covah_invoke(const cContext &C, UsdAttribute &UNUSED(op), const TfNotice &UNUSED(event))
{
  UserDef uprefs = CTX_data_uprefs(C);

  bool showsave;
  uprefs->showsave.Get(&showsave);

  if (showsave)
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
  ot->idname = COVAH_OPERATOR_IDNAME(WM_OT_window_close);
  ot->description = "Close the current window";

  ot->exec = wm_window_close_exec;
  ot->poll = wm_operator_winactive;
}


static void WM_OT_window_new(wmOperatorType *ot)
{
  ot->name = "New Window";
  ot->idname = COVAH_OPERATOR_IDNAME(WM_OT_window_new);
  ot->description = "Create a new window";

  ot->exec = wm_window_new_exec;
  ot->poll = wm_operator_winactive_normal;
}


static void WM_OT_window_new_main(wmOperatorType *ot)
{
  ot->name = "New Main Window";
  ot->idname = COVAH_OPERATOR_IDNAME(WM_OT_window_new_main);
  ot->description = "Create a new main window with its own workspace and scene selection";

  ot->exec = wm_window_new_main_exec;
  ot->poll = wm_operator_winactive_normal;
}


static void WM_OT_window_fullscreen_toggle(wmOperatorType *ot)
{
  ot->name = "Toggle Window Fullscreen";
  ot->idname = COVAH_OPERATOR_IDNAME(WM_OT_window_fullscreen_toggle);
  ot->description = "Toggle the current window fullscreen";

  ot->exec = wm_window_fullscreen_toggle_exec;
  ot->poll = wm_operator_winactive;
}


static void WM_OT_quit_covah(wmOperatorType *ot)
{
  ot->name = "Quit Covah";
  ot->idname = COVAH_OPERATOR_IDNAME(WM_OT_quit_covah);
  ot->description = "Quit Covah";

  ot->invoke = wm_exit_covah_invoke;
  ot->exec = wm_exit_covah_exec;
}


void WM_window_operators_register(const cContext &C)
{
  /* ------ */

  WM_operatortype_append((C), WM_OT_window_close);
  WM_operatortype_append((C), WM_OT_window_new);
  WM_operatortype_append((C), WM_OT_window_new_main);
  WM_operatortype_append((C), WM_OT_window_fullscreen_toggle);
  WM_operatortype_append((C), WM_OT_quit_covah);

  /* ------ */
}

WABI_NAMESPACE_END