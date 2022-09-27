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
 * Universe.
 * Set the Stage.
 */

#include "USD_vec_types.h"

/** Scroller flags for View2D (#View2D.scroll). */
enum {
  /* Left scroll-bar. */
  V2D_SCROLL_LEFT = (1 << 0),
  V2D_SCROLL_RIGHT = (1 << 1),
  V2D_SCROLL_VERTICAL = (V2D_SCROLL_LEFT | V2D_SCROLL_RIGHT),
  /* Horizontal scroll-bar. */
  V2D_SCROLL_TOP = (1 << 2),
  V2D_SCROLL_BOTTOM = (1 << 3),
  /* UNUSED                    = (1 << 4), */
  V2D_SCROLL_HORIZONTAL = (V2D_SCROLL_TOP | V2D_SCROLL_BOTTOM),
  /* display vertical scale handles */
  V2D_SCROLL_VERTICAL_HANDLES = (1 << 5),
  /* display horizontal scale handles */
  V2D_SCROLL_HORIZONTAL_HANDLES = (1 << 6),
  /* Induce hiding of scroll-bar - set by region drawing in response to size of region. */
  V2D_SCROLL_VERTICAL_HIDE = (1 << 7),
  V2D_SCROLL_HORIZONTAL_HIDE = (1 << 8),
  /* Scroll-bar extends beyond its available window -
   * set when calculating scroll-bar for drawing */
  V2D_SCROLL_VERTICAL_FULLR = (1 << 9),
  V2D_SCROLL_HORIZONTAL_FULLR = (1 << 10),
};

/** View 2D data - stored per region. */
typedef struct View2D {
  /** Tot - area that data can be drawn in; cur - region of tot that is visible in viewport. */
  rctf tot, cur;
  /** Vert - vertical scroll-bar region; hor - horizontal scroll-bar region. */
  rcti vert, hor;
  /** Mask - region (in screen-space) within which 'cur' can be viewed. */
  rcti mask;

  /** Min/max sizes of 'cur' rect (only when keepzoom not set). */
  float min[2], max[2];
  /** Allowable zoom factor range (only when (keepzoom & V2D_LIMITZOOM)) is set. */
  float minzoom, maxzoom;

  /** Scroll - scroll-bars to display (bit-flag). */
  short scroll;
  /** Scroll_ui - temp settings used for UI drawing of scrollers. */
  short scroll_ui;

  /** Keeptot - 'cur' rect cannot move outside the 'tot' rect? */
  short keeptot;
  /** Keepzoom - axes that zooming cannot occur on, and also clamp within zoom-limits. */
  short keepzoom;
  /** Keepofs - axes that translation is not allowed to occur on. */
  short keepofs;

  /** Settings. */
  short flag;
  /** Alignment of content in totrect. */
  short align;

  /** Storage of current winx/winy values, set in UI_view2d_size_update. */
  short winx, winy;
  /** Storage of previous winx/winy values encountered by UI_view2d_curRect_validate(),
   * for keepaspect. */
  short oldwinx, oldwiny;

  /** Pivot point for transforms (rotate and scale). */
  short around;

  /* Usually set externally (as in, not in view2d files). */
  /** Alpha of vertical and horizontal scroll-bars (range is [0, 255]). */
  char alpha_vert, alpha_hor;
  char _pad[6];

  /* animated smooth view */
  // struct SmoothView2DStore *sms;
  struct wmTimer *smooth_timer;
} View2D;
