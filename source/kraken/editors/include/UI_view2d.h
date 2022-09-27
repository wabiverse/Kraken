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

float UI_view2d_scale_get_x(const struct View2D *v2d);

char UI_view2d_mouse_in_scrollers_ex(const struct kraken::ARegion *region,
                                     const struct View2D *v2d,
                                     const int xy[2],
                                     int *r_scroll) ATTR_NONNULL(1, 2, 3, 4);
char UI_view2d_mouse_in_scrollers(const struct kraken::ARegion *region,
                                  const struct View2D *v2d,
                                  const int xy[2]) ATTR_NONNULL(1, 2, 3);

char UI_view2d_rect_in_scrollers(const kraken::ARegion *region, const View2D *v2d, const rcti *rect);
char UI_view2d_rect_in_scrollers_ex(const kraken::ARegion *region,
                                    const View2D *v2d,
                                    const rcti *rect,
                                    int *r_scroll);