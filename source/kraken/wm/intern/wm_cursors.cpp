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

#include "KKE_context.h"
#include "KKE_global.h"

#include "WM_cursors_api.h"
#include "WM_cursors.h"
#include "WM_window.hh"

#include "USD_area.h"
#include "USD_factory.h"
#include "USD_region.h"
#include "USD_screen.h"
#include "USD_window.h"

#include "ANCHOR_api.h"



/* Kraken custom cursor. */
struct KCursor
{
  char *bitmap;
  char *mask;
  char hotx;
  char hoty;
  bool can_invert_color;
};

static KCursor *KrakenCursor[WM_CURSOR_NUM] = {0};

void WM_cursor_position_from_anchor(wmWindow *win, int *x, int *y)
{
  float fac = ANCHOR::GetNativePixelSize((AnchorSystemWindowHandle)win->anchorwin);

  ANCHOR::ScreenToClient((AnchorSystemWindowHandle)win->anchorwin, *x, *y, x, y);
  *x *= fac;

  GfVec2f win_size = FormFactory(win->size);

  *y = (GET_Y(win_size) - 1) - *y;
  *y *= fac;
}

void WM_cursor_position_to_anchor(wmWindow *win, int *x, int *y)
{
  float fac = ANCHOR::GetNativePixelSize((AnchorSystemWindowHandle)win->anchorwin);

  GfVec2f win_size = FormFactory(win->size);

  *x /= fac;
  *y /= fac;
  *y = GET_Y(win_size) - *y - 1;

  ANCHOR::ClientToScreen((AnchorSystemWindowHandle)win->anchorwin, *x, *y, x, y);
}

static void window_set_custom_cursor_ex(wmWindow *win, KCursor *cursor)
{
  ANCHOR::SetCustomCursorShape((AnchorSystemWindowHandle)win->anchorwin,
                               (uint8_t *)cursor->bitmap,
                               (uint8_t *)cursor->mask,
                               16,
                               16,
                               cursor->hotx,
                               cursor->hoty,
                               cursor->can_invert_color);
}

void WM_cursor_set(wmWindow *win, int curs)
{
  if (win == NULL || G.background) {
    return; /* Can't set custom cursor before Window init */
  }

  if (curs == WM_CURSOR_DEFAULT && win->modalcursor) {
    curs = win->modalcursor;
  }

  if (curs == WM_CURSOR_NONE) {
    ANCHOR::SetCursorVisibility((AnchorSystemWindowHandle)win->anchorwin, 0);
    return;
  }

  ANCHOR::SetCursorVisibility((AnchorSystemWindowHandle)win->anchorwin, 1);

  static TfToken current;
  eAnchorStandardCursor anchor_cursor;

  /* Check to see if already set. */
  if (win->cursor.Get(&current)) {
    switch (curs) {
    WM_CURSOR_DEFAULT:
      anchor_cursor = ANCHOR_StandardCursorDefault;
      if (current == UsdUITokens->default_) {
        win->cursor.Set(UsdUITokens->default_);
        return;
      }
      break;
    WM_CURSOR_TEXT_EDIT:
      anchor_cursor = ANCHOR_StandardCursorText;
      if (current == UsdUITokens->textEdit) {
        win->cursor.Set(UsdUITokens->textEdit);
        return;
      }
      break;
    WM_CURSOR_WAIT:
      anchor_cursor = ANCHOR_StandardCursorWait;
      if (current == UsdUITokens->wait) {
        win->cursor.Set(UsdUITokens->wait);
        return;
      }
      break;
    WM_CURSOR_STOP:
      anchor_cursor = ANCHOR_StandardCursorStop;
      if (current == UsdUITokens->stop) {
        win->cursor.Set(UsdUITokens->stop);
        return;
      }
      break;
    WM_CURSOR_CROSS:
    WM_CURSOR_EDIT:
      anchor_cursor = ANCHOR_StandardCursorCrosshair;
      if (current == UsdUITokens->edit) {
        win->cursor.Set(UsdUITokens->edit);
        return;
      }
      break;
    WM_CURSOR_COPY:
      anchor_cursor = ANCHOR_StandardCursorCopy;
      if (current == UsdUITokens->copy) {
        win->cursor.Set(UsdUITokens->copy);
        return;
      }
      break;
    WM_CURSOR_HAND:
      anchor_cursor = ANCHOR_StandardCursorMove;
      if (current == UsdUITokens->hand) {
        win->cursor.Set(UsdUITokens->hand);
        return;
      }
      break;
    WM_CURSOR_PAINT:
      anchor_cursor = ANCHOR_StandardCursorCrosshairA;
      if (current == UsdUITokens->paint) {
        win->cursor.Set(UsdUITokens->paint);
        return;
      }
      break;
    WM_CURSOR_DOT:
      anchor_cursor = ANCHOR_StandardCursorCrosshairB;
      if (current == UsdUITokens->dot) {
        win->cursor.Set(UsdUITokens->dot);
        return;
      }
      break;
    // todo: author this with the others
    // in the usd schema.
    // WM_CURSOR_CROSSC:
    //   anchor_cursor = ANCHOR_StandardCursorCrosshairC;
    //   if (current == UsdUITokens->crossc) {
    //     return;
    //   }
    //   break;
    WM_CURSOR_KNIFE:
      anchor_cursor = ANCHOR_StandardCursorKnife;
      if (current == UsdUITokens->knife) {
        win->cursor.Set(UsdUITokens->knife);
        return;
      }
      break;
    WM_CURSOR_PAINT_BRUSH:
      anchor_cursor = ANCHOR_StandardCursorPencil;
      if (current == UsdUITokens->paintBrush) {
        win->cursor.Set(UsdUITokens->paintBrush);
        return;
      }
      break;
    WM_CURSOR_ERASER:
      anchor_cursor = ANCHOR_StandardCursorEraser;
      if (current == UsdUITokens->eraser) {
        win->cursor.Set(UsdUITokens->eraser);
        return;
      }
      break;
    WM_CURSOR_EYEDROPPER:
      anchor_cursor = ANCHOR_StandardCursorEyedropper;
      if (current == UsdUITokens->eyedropper) {
        win->cursor.Set(UsdUITokens->eyedropper);
        return;
      }
      break;
    WM_CURSOR_X_MOVE:
      anchor_cursor = ANCHOR_StandardCursorLeftRight;
      if (current == UsdUITokens->xMove) {
        win->cursor.Set(UsdUITokens->xMove);
        return;
      }
      break;
    WM_CURSOR_Y_MOVE:
      anchor_cursor = ANCHOR_StandardCursorUpDown;
      if (current == UsdUITokens->yMove) {
        win->cursor.Set(UsdUITokens->yMove);
        return;
      }
      break;
    WM_CURSOR_H_SPLIT:
      anchor_cursor = ANCHOR_StandardCursorHorizontalSplit;
      if (current == UsdUITokens->hSplit) {
        win->cursor.Set(UsdUITokens->hSplit);
        return;
      }
      break;
    WM_CURSOR_V_SPLIT:
      anchor_cursor = ANCHOR_StandardCursorVerticalSplit;
      if (current == UsdUITokens->verticalSplit) {
        win->cursor.Set(UsdUITokens->verticalSplit);
        return;
      }
      break;
      // todo: author these with the others in the usd schema. ------------------
      // WM_CURSOR_NW_ARROW:
      //   if (current == UsdUITokens->nwArrow) {
      //     return;
      //   }
      //   break;
      // todo: author this with the others
      // in the usd schema.
      // WM_CURSOR_NS_ARROW:
      //   if (current == UsdUITokens->nsArrow) {
      //     return;
      //   }
      //   break;
      // todo: author this with the others
      // in the usd schema.
      // WM_CURSOR_EW_ARROW:
      //   if (current == UsdUITokens->ewArrow) {
      //     return;
      //   }
      //   break;
      // WM_CURSOR_N_ARROW:
      //   if (current == UsdUITokens->nArrow) {
      //     return;
      //   }
      //   break;
      // WM_CURSOR_S_ARROW:
      //   if (current == UsdUITokens->sArrow) {
      //     return;
      //   }
      //   break;
      // WM_CURSOR_E_ARROW:
      //   if (current == UsdUITokens->eArrow) {
      //     return;
      //   }
      //   break;
      // WM_CURSOR_W_ARROW:
      //   if (current == UsdUITokens->wArrow) {
      //     return;
      //   }
      //   break;
      // WM_CURSOR_NSEW_SCROLL:
      //   if (current == UsdUITokens->nsewScroll) {
      //     return;
      //   }
      //   break;
      // WM_CURSOR_NS_SCROLL:
      //   if (current == UsdUITokens->nsScroll) {
      //     return;
      //   }
      //   break;
      // WM_CURSOR_EW_SCROLL:
      //   if (current == UsdUITokens->ewScroll) {
      //     return;
      //   }
      //   break;
      // WM_CURSOR_ZOOM_IN:
      //   if (current == UsdUITokens->zoomIn) {
      //     return;
      //   }
      //   break;
      // WM_CURSOR_ZOOM_OUT:
      //   if (current == UsdUITokens->zoomOut) {
      //     return;
      //   }
      //   break;
      // WM_CURSOR_NONE:
      //   if (current == UsdUITokens->cursorNone) {
      //     return;
      //   }
      //   break;
      // WM_CURSOR_MUTE:
      //   if (current == UsdUITokens->mute) {
      //     return;
      //   }
      //   break;
      // WM_CURSOR_PICK_AREA:
      //   if (current == UsdUITokens->pickArea) {
      //     return;
      //   }
      //   break
      // ----------------------------------------------------------------
      default:
        break;
    }
  }

  if (curs < 0 || curs >= WM_CURSOR_NUM) {
    printf("Invalid cursor used.\n");
    KLI_assert(0);
    return;
  }

  if (anchor_cursor != ANCHOR_StandardCursorCustom &&
      ANCHOR::HasCursorShape((AnchorSystemWindowHandle)win->anchorwin, anchor_cursor)) {
    /* Use native ANCHOR cursor when available. */
    ANCHOR::SetCursorShape((AnchorSystemWindowHandle)win->anchorwin, anchor_cursor);
  } else {
    KCursor *kcursor = KrakenCursor[curs];
    if (kcursor) {
      /* Use custom bitmap cursor. */
      window_set_custom_cursor_ex(win, kcursor);
    } else {
      /* Fallback to default cursor if no bitmap found. */
      ANCHOR::SetCursorShape((AnchorSystemWindowHandle)win->anchorwin,
                             ANCHOR_StandardCursorDefault);
    }
  }
}

void WM_cursor_modal_restore(wmWindow *win)
{
  win->modalcursor = 0;
  if (win->lastcursor) {
    WM_cursor_set(win, win->lastcursor);
  }
  win->lastcursor = 0;
}

void WM_cursor_grab_enable(wmWindow *win, int wrap, bool hide, int bounds[4])
{
  /* Only grab cursor when not running debug.
   * It helps not to get a stuck WM when hitting a break-point. */
  eAnchorGrabCursorMode mode = ANCHOR_GrabNormal;
  int mode_axis = ANCHOR_GrabAxisX | ANCHOR_GrabAxisY;

  if (bounds) {
    WM_cursor_position_to_anchor(win, &bounds[0], &bounds[1]);
    WM_cursor_position_to_anchor(win, &bounds[2], &bounds[3]);
  }

  if (hide) {
    mode = ANCHOR_GrabHide;
  } else if (wrap) {
    mode = ANCHOR_GrabWrap;

    if (wrap == WM_CURSOR_WRAP_X) {
      mode_axis = ANCHOR_GrabAxisX;
    } else if (wrap == WM_CURSOR_WRAP_Y) {
      mode_axis = ANCHOR_GrabAxisY;
    }
  }
}

void WM_cursor_modal_set(wmWindow *win, int val)
{
  if (win->lastcursor == 0) {

    static TfToken lastcursor;

    /* convert usd authored cursor to int. */
    if (win->cursor.Get(&lastcursor)) {
      if (lastcursor == UsdUITokens->default_) {
        win->lastcursor = WM_CURSOR_DEFAULT;
      } else if (lastcursor == UsdUITokens->textEdit) {
        win->lastcursor = WM_CURSOR_TEXT_EDIT;
      } else if (lastcursor == UsdUITokens->wait) {
        win->lastcursor = WM_CURSOR_WAIT;
      } else if (lastcursor == UsdUITokens->stop) {
        win->lastcursor = WM_CURSOR_STOP;
      } else if (lastcursor == UsdUITokens->edit) {
        win->lastcursor = WM_CURSOR_EDIT;
      } else if (lastcursor == UsdUITokens->copy) {
        win->lastcursor = WM_CURSOR_COPY;
      } else if (lastcursor == UsdUITokens->hand) {
        win->lastcursor = WM_CURSOR_HAND;
      } else if (lastcursor == UsdUITokens->paint) {
        win->lastcursor = WM_CURSOR_PAINT;
      } else if (lastcursor == UsdUITokens->dot) {
        win->lastcursor = WM_CURSOR_DOT;
      } else if (lastcursor == UsdUITokens->knife) {
        win->lastcursor = WM_CURSOR_KNIFE;
      } else if (lastcursor == UsdUITokens->paintBrush) {
        win->lastcursor = WM_CURSOR_PAINT_BRUSH;
      } else if (lastcursor == UsdUITokens->eraser) {
        win->lastcursor = WM_CURSOR_ERASER;
      } else if (lastcursor == UsdUITokens->eyedropper) {
        win->lastcursor = WM_CURSOR_EYEDROPPER;
      } else if (lastcursor == UsdUITokens->xMove) {
        win->lastcursor = WM_CURSOR_X_MOVE;
      } else if (lastcursor == UsdUITokens->yMove) {
        win->lastcursor = WM_CURSOR_Y_MOVE;
      } else if (lastcursor == UsdUITokens->hSplit) {
        win->lastcursor = WM_CURSOR_H_SPLIT;
      } else if (lastcursor == UsdUITokens->verticalSplit) {
        win->lastcursor = WM_CURSOR_V_SPLIT;
      }
    }
  }

  win->modalcursor = val;
  WM_cursor_set(win, val);
}

void WM_cursor_grab_disable(wmWindow *win, const int mouse_ungrab_xy[2])
{
  if ((G.debug & G_DEBUG) == 0) {
    if (win && win->anchorwin) {
      if (mouse_ungrab_xy) {
        int mouse_xy[2] = {mouse_ungrab_xy[0], mouse_ungrab_xy[1]};
        WM_cursor_position_to_anchor_screen_coords(win, &mouse_xy[0], &mouse_xy[1]);
        ANCHOR::SetCursorGrab((AnchorSystemWindowHandle)win->anchorwin, ANCHOR_GrabDisable, ANCHOR_GrabAxisNone, NULL, mouse_xy);
      }
      else {
        ANCHOR::SetCursorGrab((AnchorSystemWindowHandle)win->anchorwin, ANCHOR_GrabDisable, ANCHOR_GrabAxisNone, NULL, NULL);
      }

      win->grabcursor = ANCHOR_GrabDisable;
    }
  }
}

