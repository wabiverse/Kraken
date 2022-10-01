/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright 2008 Blender Foundation. All rights reserved. */

/** \file
 * \ingroup edinterface
 */

#include <cfloat>
#include <climits>
#include <cmath>
#include <cstring>
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "MEM_guardedalloc.h"

#include "USD_factory.h"
#include "USD_scene.h"
#include "USD_userpref.h"
#include "USD_view2d.h"

#include "KLI_array.h"
#include "KLI_easing.h"
#include "KLI_link_utils.h"
#include "KLI_listbase.h"
#include "KLI_math.h"
// #include "KLI_memarena.h"
#include "KLI_rect.h"
#include "KLI_string.h"
// #include "KLI_timecode.h"
#include "KLI_utildefines.h"

#include "KKE_context.h"
#include "KKE_main.h"
#include "KKE_screen.h"

#include "GPU_immediate.h"
#include "GPU_matrix.h"
#include "GPU_state.h"

#include "WM_api.h"

// #include "KRF_api.h"

#include "ED_screen.h"

#include "UI_interface.h"
#include "UI_view2d.h"

#include "interface_intern.h"


float UI_view2d_scale_get_x(const View2D *v2d)
{
  GfVec4i mask;
  mask[0] = v2d->mask.xmin;
  mask[1] = v2d->mask.xmax;
  mask[2] = v2d->mask.ymin;
  mask[3] = v2d->mask.ymax;

  GfVec4f cur;
  cur[0] = v2d->cur.xmin;
  cur[1] = v2d->cur.xmax;
  cur[2] = v2d->cur.ymin;
  cur[3] = v2d->cur.ymax;

  return KLI_rcti_size_x(mask) / KLI_rctf_size_x(cur);
}

/**
 * Helper to allow scroll-bars to dynamically hide:
 * - Returns a copy of the scroll-bar settings with the flags to display
 *   horizontal/vertical scroll-bars removed.
 * - Input scroll value is the v2d->scroll var.
 * - Hide flags are set per region at draw-time.
 */
static int view2d_scroll_mapped(int scroll)
{
  if (scroll & V2D_SCROLL_HORIZONTAL_FULLR) {
    scroll &= ~V2D_SCROLL_HORIZONTAL;
  }
  if (scroll & V2D_SCROLL_VERTICAL_FULLR) {
    scroll &= ~V2D_SCROLL_VERTICAL;
  }
  return scroll;
}

char UI_view2d_mouse_in_scrollers_ex(const kraken::ARegion *region,
                                     const View2D *v2d,
                                     const int xy[2],
                                     int *r_scroll)
{
  const int scroll = view2d_scroll_mapped(v2d->scroll);
  *r_scroll = scroll;

  GfVec4i coords = kraken::FormFactory(region->coords);

  if (scroll) {
    /* Move to region-coordinates. */
    const int co[2] = {
      xy[0] - coords[0],
      xy[1] - coords[2],
    };
    if (scroll & V2D_SCROLL_HORIZONTAL) {
      if (IN_2D_HORIZ_SCROLL(v2d, co)) {
        return 'h';
      }
    }
    if (scroll & V2D_SCROLL_VERTICAL) {
      if (IN_2D_VERT_SCROLL(v2d, co)) {
        return 'v';
      }
    }
  }

  return 0;
}

char UI_view2d_rect_in_scrollers_ex(const kraken::ARegion *region,
                                    const View2D *v2d,
                                    const rcti *rect,
                                    int *r_scroll)
{
  const int scroll = view2d_scroll_mapped(v2d->scroll);
  *r_scroll = scroll;

  if (scroll) {
    /* Move to region-coordinates. */
    rcti rect_region = *rect;
    GfVec4i coords = kraken::FormFactory(region->coords);
    KLI_rcti_translate(&rect_region, -coords[0], coords[2]);
    if (scroll & V2D_SCROLL_HORIZONTAL) {
      if (IN_2D_HORIZ_SCROLL_RECT(v2d, &rect_region)) {
        return 'h';
      }
    }
    if (scroll & V2D_SCROLL_VERTICAL) {
      if (IN_2D_VERT_SCROLL_RECT(v2d, &rect_region)) {
        return 'v';
      }
    }
  }

  return 0;
}

char UI_view2d_mouse_in_scrollers(const kraken::ARegion *region, const View2D *v2d, const int xy[2])
{
  int scroll_dummy = 0;
  return UI_view2d_mouse_in_scrollers_ex(region, v2d, xy, &scroll_dummy);
}

char UI_view2d_rect_in_scrollers(const kraken::ARegion *region, const View2D *v2d, const rcti *rect)
{
  int scroll_dummy = 0;
  return UI_view2d_rect_in_scrollers_ex(region, v2d, rect, &scroll_dummy);
}