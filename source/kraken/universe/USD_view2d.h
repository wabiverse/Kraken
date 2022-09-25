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