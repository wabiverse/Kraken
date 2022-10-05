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
 * Editors.
 * Tools for Artists.
 */

#pragma once

#include "KLI_compiler_attrs.h"
#include "KLI_rect.h"

/* ------------------------------------------ */
/* Macros:                                    */

/* Test if mouse in a scroll-bar (assume that scroller availability has been tested). */
#define IN_2D_VERT_SCROLL(v2d, co) (KLI_rcti_isect_pt_v(&v2d->vert, co))
#define IN_2D_HORIZ_SCROLL(v2d, co) (KLI_rcti_isect_pt_v(&v2d->hor, co))

#define IN_2D_VERT_SCROLL_RECT(v2d, rct) (KLI_rcti_isect(&v2d->vert, rct, NULL))
#define IN_2D_HORIZ_SCROLL_RECT(v2d, rct) (KLI_rcti_isect(&v2d->hor, rct, NULL))

/* ------ Defines for Scrollers ----- */

/** Scroll bar area. */

/* Maximum has to include outline which varies with line width. */
#define V2D_SCROLL_HEIGHT ((0.45f * U.widget_unit) + (2.0f * U.pixelsize))
#define V2D_SCROLL_WIDTH ((0.45f * U.widget_unit) + (2.0f * U.pixelsize))

/* Alpha of scroll-bar when at minimum size. */
#define V2D_SCROLL_MIN_ALPHA (0.4f)

/* Minimum size needs to include outline which varies with line width. */
#define V2D_SCROLL_MIN_WIDTH ((5.0f * U.dpi_fac) + (2.0f * U.pixelsize))

/* When to start showing the full-width scroller. */
#define V2D_SCROLL_HIDE_WIDTH (AREAMINX * U.dpi_fac)
#define V2D_SCROLL_HIDE_HEIGHT (HEADERY * U.dpi_fac)

/** Scroll bars with 'handles' used for scale (zoom). */
#define V2D_SCROLL_HANDLE_HEIGHT (0.6f * U.widget_unit)
#define V2D_SCROLL_HANDLE_WIDTH (0.6f * U.widget_unit)

/** Scroll bar with 'handles' hot-spot radius for cursor proximity. */
#define V2D_SCROLL_HANDLE_SIZE_HOTSPOT (0.6f * U.widget_unit)

/** Don't allow scroll thumb to show below this size (so it's never too small to click on). */
#define V2D_SCROLL_THUMB_SIZE_MIN (30.0 * UI_DPI_FAC)

#define UI_MARKER_MARGIN_Y (42 * UI_DPI_FAC)
#define UI_TIME_SCRUB_MARGIN_Y (23 * UI_DPI_FAC)

/* ------ Define for UI_view2d_sync ----- */

/* means copy it from another v2d */
#define V2D_LOCK_SET 0
/* means copy it to the other v2ds */
#define V2D_LOCK_COPY 1

float UI_view2d_scale_get_x(const struct View2D *v2d);

char UI_view2d_mouse_in_scrollers_ex(const struct ARegion *region,
                                     const struct View2D *v2d,
                                     const int xy[2],
                                     int *r_scroll) ATTR_NONNULL(1, 2, 3, 4);
char UI_view2d_mouse_in_scrollers(const struct ARegion *region,
                                  const struct View2D *v2d,
                                  const int xy[2]) ATTR_NONNULL(1, 2, 3);

char UI_view2d_rect_in_scrollers(const ARegion *region, const View2D *v2d, const rcti *rect);
char UI_view2d_rect_in_scrollers_ex(const ARegion *region,
                                    const View2D *v2d,
                                    const rcti *rect,
                                    int *r_scroll);

/**
 * Perform all required updates after `v2d->cur` as been modified.
 * This includes like validation view validation (#UI_view2d_curRect_validate).
 *
 * Current intent is to use it from user code, such as view navigation and zoom operations.
 */
void UI_view2d_curRect_changed(const struct kContext *C, struct View2D *v2d);
void UI_view2d_curRect_validate(struct View2D *v2d);

void UI_view2d_mask_from_win(const struct View2D *v2d, rcti *r_mask);

void UI_view2d_region_to_view_rctf(const struct View2D *v2d,
                                   const struct rctf *rect_src,
                                   struct rctf *rect_dst) ATTR_NONNULL();