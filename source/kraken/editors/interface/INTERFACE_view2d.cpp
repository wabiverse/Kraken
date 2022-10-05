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

char UI_view2d_mouse_in_scrollers_ex(const ARegion *region,
                                     const View2D *v2d,
                                     const int xy[2],
                                     int *r_scroll)
{
  const int scroll = view2d_scroll_mapped(v2d->scroll);
  *r_scroll = scroll;

  GfVec4i coords = FormFactory(region->coords);

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

char UI_view2d_rect_in_scrollers_ex(const ARegion *region,
                                    const View2D *v2d,
                                    const rcti *rect,
                                    int *r_scroll)
{
  const int scroll = view2d_scroll_mapped(v2d->scroll);
  *r_scroll = scroll;

  if (scroll) {
    /* Move to region-coordinates. */
    rcti rect_region = *rect;
    GfVec4i coords = FormFactory(region->coords);
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

char UI_view2d_mouse_in_scrollers(const ARegion *region, const View2D *v2d, const int xy[2])
{
  int scroll_dummy = 0;
  return UI_view2d_mouse_in_scrollers_ex(region, v2d, xy, &scroll_dummy);
}

char UI_view2d_rect_in_scrollers(const ARegion *region, const View2D *v2d, const rcti *rect)
{
  int scroll_dummy = 0;
  return UI_view2d_rect_in_scrollers_ex(region, v2d, rect, &scroll_dummy);
}

void UI_view2d_scroller_size_get(const View2D *v2d, bool mapped, float *r_x, float *r_y)
{
  const int scroll = (mapped) ? view2d_scroll_mapped(v2d->scroll) : v2d->scroll;

  if (r_x) {
    if (scroll & V2D_SCROLL_VERTICAL) {
      *r_x = (scroll & V2D_SCROLL_VERTICAL_HANDLES) ? V2D_SCROLL_HANDLE_WIDTH : V2D_SCROLL_WIDTH;
      *r_x = ((*r_x - V2D_SCROLL_MIN_WIDTH) * (v2d->alpha_vert / 255.0f)) + V2D_SCROLL_MIN_WIDTH;
    } else {
      *r_x = 0;
    }
  }
  if (r_y) {
    if (scroll & V2D_SCROLL_HORIZONTAL) {
      *r_y = (scroll & V2D_SCROLL_HORIZONTAL_HANDLES) ? V2D_SCROLL_HANDLE_HEIGHT :
                                                        V2D_SCROLL_HEIGHT;
      *r_y = ((*r_y - V2D_SCROLL_MIN_WIDTH) * (v2d->alpha_hor / 255.0f)) + V2D_SCROLL_MIN_WIDTH;
    } else {
      *r_y = 0;
    }
  }
}

void UI_view2d_mask_from_win(const View2D *v2d, rcti *r_mask)
{
  r_mask->xmin = 0;
  r_mask->ymin = 0;
  r_mask->xmax = v2d->winx - 1; /* -1 yes! masks are pixels */
  r_mask->ymax = v2d->winy - 1;
}

/**
 * Called each time #View2D.cur changes, to dynamically update masks.
 *
 * @param mask_scroll: Optionally clamp scroll-bars by this region.
 */
static void view2d_masks(View2D *v2d, const rcti *mask_scroll)
{
  int scroll;

  /* mask - view frame */
  UI_view2d_mask_from_win(v2d, &v2d->mask);
  if (mask_scroll == nullptr) {
    mask_scroll = &v2d->mask;
  }

  /* check size if hiding flag is set: */
  if (v2d->scroll & V2D_SCROLL_HORIZONTAL_HIDE) {
    if (!(v2d->scroll & V2D_SCROLL_HORIZONTAL_HANDLES)) {
      if (KLI_rctf_size_x(&v2d->tot) > KLI_rctf_size_x(&v2d->cur)) {
        v2d->scroll &= ~V2D_SCROLL_HORIZONTAL_FULLR;
      } else {
        v2d->scroll |= V2D_SCROLL_HORIZONTAL_FULLR;
      }
    }
  }
  if (v2d->scroll & V2D_SCROLL_VERTICAL_HIDE) {
    if (!(v2d->scroll & V2D_SCROLL_VERTICAL_HANDLES)) {
      if (KLI_rctf_size_y(&v2d->tot) + 0.01f > KLI_rctf_size_y(&v2d->cur)) {
        v2d->scroll &= ~V2D_SCROLL_VERTICAL_FULLR;
      } else {
        v2d->scroll |= V2D_SCROLL_VERTICAL_FULLR;
      }
    }
  }

  /* Do not use mapped scroll here because we want to update scroller rects
   * even if they are not displayed. For initialization purposes. See T75003. */
  scroll = v2d->scroll;

  /* Scrollers are based off region-size:
   * - they can only be on one to two edges of the region they define
   * - if they overlap, they must not occupy the corners (which are reserved for other widgets)
   */
  if (scroll) {
    float scroll_width, scroll_height;

    UI_view2d_scroller_size_get(v2d, false, &scroll_width, &scroll_height);

    /* vertical scroller */
    if (scroll & V2D_SCROLL_LEFT) {
      /* on left-hand edge of region */
      v2d->vert = *mask_scroll;
      v2d->vert.xmax = scroll_width;
    } else if (scroll & V2D_SCROLL_RIGHT) {
      /* on right-hand edge of region */
      v2d->vert = *mask_scroll;
      v2d->vert.xmax++; /* one pixel extra... was leaving a minor gap... */
      v2d->vert.xmin = v2d->vert.xmax - scroll_width;
    }

    /* Currently, all regions that have vertical scale handles,
     * also have the scrubbing area at the top.
     * So the scroll-bar has to move down a bit. */
    if (scroll & V2D_SCROLL_VERTICAL_HANDLES) {
      v2d->vert.ymax -= UI_TIME_SCRUB_MARGIN_Y;
    }

    /* horizontal scroller */
    if (scroll & V2D_SCROLL_BOTTOM) {
      /* on bottom edge of region */
      v2d->hor = *mask_scroll;
      v2d->hor.ymax = scroll_height;
    } else if (scroll & V2D_SCROLL_TOP) {
      /* on upper edge of region */
      v2d->hor = *mask_scroll;
      v2d->hor.ymin = v2d->hor.ymax - scroll_height;
    }

    /* adjust vertical scroller if there's a horizontal scroller, to leave corner free */
    if (scroll & V2D_SCROLL_VERTICAL) {
      if (scroll & V2D_SCROLL_BOTTOM) {
        /* on bottom edge of region */
        v2d->vert.ymin = v2d->hor.ymax;
      } else if (scroll & V2D_SCROLL_TOP) {
        /* on upper edge of region */
        v2d->vert.ymax = v2d->hor.ymin;
      }
    }
  }
}

/**
 * Ensure View2D rects remain in a viable configuration
 * 'cur' is not allowed to be: larger than max, smaller than min, or outside of 'tot'
 */
static void ui_view2d_curRect_validate_resize(View2D *v2d, bool resize)
{
  float totwidth, totheight, curwidth, curheight, width, height;
  float winx, winy;
  rctf *cur, *tot;

  /* use mask as size of region that View2D resides in, as it takes into account
   * scroll-bars already - keep in sync with zoomx/zoomy in #view_zoomstep_apply_ex! */
  winx = float(KLI_rcti_size_x(&v2d->mask) + 1);
  winy = float(KLI_rcti_size_y(&v2d->mask) + 1);

  /* get pointers to rcts for less typing */
  cur = &v2d->cur;
  tot = &v2d->tot;

  /* we must satisfy the following constraints (in decreasing order of importance):
   * - alignment restrictions are respected
   * - cur must not fall outside of tot
   * - axis locks (zoom and offset) must be maintained
   * - zoom must not be excessive (check either sizes or zoom values)
   * - aspect ratio should be respected (NOTE: this is quite closely related to zoom too)
   */

  /* Step 1: if keepzoom, adjust the sizes of the rects only
   * - firstly, we calculate the sizes of the rects
   * - curwidth and curheight are saved as reference... modify width and height values here
   */
  totwidth = KLI_rctf_size_x(tot);
  totheight = KLI_rctf_size_y(tot);
  /* keep in sync with zoomx/zoomy in view_zoomstep_apply_ex! */
  curwidth = width = KLI_rctf_size_x(cur);
  curheight = height = KLI_rctf_size_y(cur);

  /* if zoom is locked, size on the appropriate axis is reset to mask size */
  if (v2d->keepzoom & V2D_LOCKZOOM_X) {
    width = winx;
  }
  if (v2d->keepzoom & V2D_LOCKZOOM_Y) {
    height = winy;
  }

  /* values used to divide, so make it safe
   * NOTE: width and height must use FLT_MIN instead of 1, otherwise it is impossible to
   *       get enough resolution in Graph Editor for editing some curves
   */
  if (width < FLT_MIN) {
    width = 1;
  }
  if (height < FLT_MIN) {
    height = 1;
  }
  if (winx < 1) {
    winx = 1;
  }
  if (winy < 1) {
    winy = 1;
  }

  /* V2D_LIMITZOOM indicates that zoom level should be preserved when the window size changes */
  if (resize && (v2d->keepzoom & V2D_KEEPZOOM)) {
    float zoom, oldzoom;

    if ((v2d->keepzoom & V2D_LOCKZOOM_X) == 0) {
      zoom = winx / width;
      oldzoom = v2d->oldwinx / curwidth;

      if (oldzoom != zoom) {
        width *= zoom / oldzoom;
      }
    }

    if ((v2d->keepzoom & V2D_LOCKZOOM_Y) == 0) {
      zoom = winy / height;
      oldzoom = v2d->oldwiny / curheight;

      if (oldzoom != zoom) {
        height *= zoom / oldzoom;
      }
    }
  }
  /* keepzoom (V2D_LIMITZOOM set), indicates that zoom level on each axis must not exceed limits
   * NOTE: in general, it is not expected that the lock-zoom will be used in conjunction with this
   */
  else if (v2d->keepzoom & V2D_LIMITZOOM) {

    /* check if excessive zoom on x-axis */
    if ((v2d->keepzoom & V2D_LOCKZOOM_X) == 0) {
      const float zoom = winx / width;
      if (zoom < v2d->minzoom) {
        width = winx / v2d->minzoom;
      } else if (zoom > v2d->maxzoom) {
        width = winx / v2d->maxzoom;
      }
    }

    /* check if excessive zoom on y-axis */
    if ((v2d->keepzoom & V2D_LOCKZOOM_Y) == 0) {
      const float zoom = winy / height;
      if (zoom < v2d->minzoom) {
        height = winy / v2d->minzoom;
      } else if (zoom > v2d->maxzoom) {
        height = winy / v2d->maxzoom;
      }
    }
  } else {
    /* make sure sizes don't exceed that of the min/max sizes
     * (even though we're not doing zoom clamping) */
    CLAMP(width, v2d->min[0], v2d->max[0]);
    CLAMP(height, v2d->min[1], v2d->max[1]);
  }

  /* check if we should restore aspect ratio (if view size changed) */
  if (v2d->keepzoom & V2D_KEEPASPECT) {
    bool do_x = false, do_y = false, do_cur;
    float curRatio, winRatio;

    /* when a window edge changes, the aspect ratio can't be used to
     * find which is the best new 'cur' rect. that's why it stores 'old'
     */
    if (winx != v2d->oldwinx) {
      do_x = true;
    }
    if (winy != v2d->oldwiny) {
      do_y = true;
    }

    curRatio = height / width;
    winRatio = winy / winx;

    /* Both sizes change (area/region maximized). */
    if (do_x == do_y) {
      if (do_x && do_y) {
        /* here is 1,1 case, so all others must be 0,0 */
        if (fabsf(winx - v2d->oldwinx) > fabsf(winy - v2d->oldwiny)) {
          do_y = false;
        } else {
          do_x = false;
        }
      } else if (winRatio > curRatio) {
        do_x = false;
      } else {
        do_x = true;
      }
    }
    do_cur = do_x;
    /* do_win = do_y; */ /* UNUSED */

    if (do_cur) {
      if ((v2d->keeptot == V2D_KEEPTOT_STRICT) && (winx != v2d->oldwinx)) {
        /* Special exception for Outliner (and later channel-lists):
         * - The view may be moved left to avoid contents
         *   being pushed out of view when view shrinks.
         * - The keeptot code will make sure cur->xmin will not be less than tot->xmin
         *   (which cannot be allowed).
         * - width is not adjusted for changed ratios here.
         */
        if (winx < v2d->oldwinx) {
          const float temp = v2d->oldwinx - winx;

          cur->xmin -= temp;
          cur->xmax -= temp;

          /* width does not get modified, as keepaspect here is just set to make
           * sure visible area adjusts to changing view shape!
           */
        }
      } else {
        /* portrait window: correct for x */
        width = height / winRatio;
      }
    } else {
      if ((v2d->keeptot == V2D_KEEPTOT_STRICT) && (winy != v2d->oldwiny)) {
        /* special exception for Outliner (and later channel-lists):
         * - Currently, no actions need to be taken here...
         */

        if (winy < v2d->oldwiny) {
          const float temp = v2d->oldwiny - winy;

          if (v2d->align & V2D_ALIGN_NO_NEG_Y) {
            cur->ymin -= temp;
            cur->ymax -= temp;
          } else { /* Assume V2D_ALIGN_NO_POS_Y or combination */
            cur->ymin += temp;
            cur->ymax += temp;
          }
        }
      } else {
        /* landscape window: correct for y */
        height = width * winRatio;
      }
    }

    /* store region size for next time */
    v2d->oldwinx = short(winx);
    v2d->oldwiny = short(winy);
  }

  /* Step 2: apply new sizes to cur rect,
   * but need to take into account alignment settings here... */
  if ((width != curwidth) || (height != curheight)) {
    float temp, dh;

    /* Resize from center-point, unless otherwise specified. */
    if (width != curwidth) {
      if (v2d->keepofs & V2D_LOCKOFS_X) {
        cur->xmax += width - KLI_rctf_size_x(cur);
      } else if (v2d->keepofs & V2D_KEEPOFS_X) {
        if (v2d->align & V2D_ALIGN_NO_POS_X) {
          cur->xmin -= width - KLI_rctf_size_x(cur);
        } else {
          cur->xmax += width - KLI_rctf_size_x(cur);
        }
      } else {
        temp = KLI_rctf_cent_x(cur);
        dh = width * 0.5f;

        cur->xmin = temp - dh;
        cur->xmax = temp + dh;
      }
    }
    if (height != curheight) {
      if (v2d->keepofs & V2D_LOCKOFS_Y) {
        cur->ymax += height - KLI_rctf_size_y(cur);
      } else if (v2d->keepofs & V2D_KEEPOFS_Y) {
        if (v2d->align & V2D_ALIGN_NO_POS_Y) {
          cur->ymin -= height - KLI_rctf_size_y(cur);
        } else {
          cur->ymax += height - KLI_rctf_size_y(cur);
        }
      } else {
        temp = KLI_rctf_cent_y(cur);
        dh = height * 0.5f;

        cur->ymin = temp - dh;
        cur->ymax = temp + dh;
      }
    }
  }

  /* Step 3: adjust so that it doesn't fall outside of bounds of 'tot' */
  if (v2d->keeptot) {
    float temp, diff;

    /* recalculate extents of cur */
    curwidth = KLI_rctf_size_x(cur);
    curheight = KLI_rctf_size_y(cur);

    /* width */
    if ((curwidth > totwidth) &&
        !(v2d->keepzoom & (V2D_KEEPZOOM | V2D_LOCKZOOM_X | V2D_LIMITZOOM))) {
      /* if zoom doesn't have to be maintained, just clamp edges */
      if (cur->xmin < tot->xmin) {
        cur->xmin = tot->xmin;
      }
      if (cur->xmax > tot->xmax) {
        cur->xmax = tot->xmax;
      }
    } else if (v2d->keeptot == V2D_KEEPTOT_STRICT) {
      /* This is an exception for the outliner (and later channel-lists, headers)
       * - must clamp within tot rect (absolutely no excuses)
       * --> therefore, cur->xmin must not be less than tot->xmin
       */
      if (cur->xmin < tot->xmin) {
        /* move cur across so that it sits at minimum of tot */
        temp = tot->xmin - cur->xmin;

        cur->xmin += temp;
        cur->xmax += temp;
      } else if (cur->xmax > tot->xmax) {
        /* - only offset by difference of cur-xmax and tot-xmax if that would not move
         *   cur-xmin to lie past tot-xmin
         * - otherwise, simply shift to tot-xmin???
         */
        temp = cur->xmax - tot->xmax;

        if ((cur->xmin - temp) < tot->xmin) {
          /* only offset by difference from cur-min and tot-min */
          temp = cur->xmin - tot->xmin;

          cur->xmin -= temp;
          cur->xmax -= temp;
        } else {
          cur->xmin -= temp;
          cur->xmax -= temp;
        }
      }
    } else {
      /* This here occurs when:
       * - width too big, but maintaining zoom (i.e. widths cannot be changed)
       * - width is OK, but need to check if outside of boundaries
       *
       * So, resolution is to just shift view by the gap between the extremities.
       * We favor moving the 'minimum' across, as that's origin for most things.
       * (XXX: in the past, max was favored... if there are bugs, swap!)
       */
      if ((cur->xmin < tot->xmin) && (cur->xmax > tot->xmax)) {
        /* outside boundaries on both sides,
         * so take middle-point of tot, and place in balanced way */
        temp = KLI_rctf_cent_x(tot);
        diff = curwidth * 0.5f;

        cur->xmin = temp - diff;
        cur->xmax = temp + diff;
      } else if (cur->xmin < tot->xmin) {
        /* move cur across so that it sits at minimum of tot */
        temp = tot->xmin - cur->xmin;

        cur->xmin += temp;
        cur->xmax += temp;
      } else if (cur->xmax > tot->xmax) {
        /* - only offset by difference of cur-xmax and tot-xmax if that would not move
         *   cur-xmin to lie past tot-xmin
         * - otherwise, simply shift to tot-xmin???
         */
        temp = cur->xmax - tot->xmax;

        if ((cur->xmin - temp) < tot->xmin) {
          /* only offset by difference from cur-min and tot-min */
          temp = cur->xmin - tot->xmin;

          cur->xmin -= temp;
          cur->xmax -= temp;
        } else {
          cur->xmin -= temp;
          cur->xmax -= temp;
        }
      }
    }

    /* height */
    if ((curheight > totheight) &&
        !(v2d->keepzoom & (V2D_KEEPZOOM | V2D_LOCKZOOM_Y | V2D_LIMITZOOM))) {
      /* if zoom doesn't have to be maintained, just clamp edges */
      if (cur->ymin < tot->ymin) {
        cur->ymin = tot->ymin;
      }
      if (cur->ymax > tot->ymax) {
        cur->ymax = tot->ymax;
      }
    } else {
      /* This here occurs when:
       * - height too big, but maintaining zoom (i.e. heights cannot be changed)
       * - height is OK, but need to check if outside of boundaries
       *
       * So, resolution is to just shift view by the gap between the extremities.
       * We favor moving the 'minimum' across, as that's origin for most things.
       */
      if ((cur->ymin < tot->ymin) && (cur->ymax > tot->ymax)) {
        /* outside boundaries on both sides,
         * so take middle-point of tot, and place in balanced way */
        temp = KLI_rctf_cent_y(tot);
        diff = curheight * 0.5f;

        cur->ymin = temp - diff;
        cur->ymax = temp + diff;
      } else if (cur->ymin < tot->ymin) {
        /* there's still space remaining, so shift up */
        temp = tot->ymin - cur->ymin;

        cur->ymin += temp;
        cur->ymax += temp;
      } else if (cur->ymax > tot->ymax) {
        /* there's still space remaining, so shift down */
        temp = cur->ymax - tot->ymax;

        cur->ymin -= temp;
        cur->ymax -= temp;
      }
    }
  }

  /* Step 4: Make sure alignment restrictions are respected */
  if (v2d->align) {
    /* If alignment flags are set (but keeptot is not), they must still be respected, as although
     * they don't specify any particular bounds to stay within, they do define ranges which are
     * invalid.
     *
     * Here, we only check to make sure that on each axis, the 'cur' rect doesn't stray into these
     * invalid zones, otherwise we offset.
     */

    /* handle width - posx and negx flags are mutually exclusive, so watch out */
    if ((v2d->align & V2D_ALIGN_NO_POS_X) && !(v2d->align & V2D_ALIGN_NO_NEG_X)) {
      /* width is in negative-x half */
      if (v2d->cur.xmax > 0) {
        v2d->cur.xmin -= v2d->cur.xmax;
        v2d->cur.xmax = 0.0f;
      }
    } else if ((v2d->align & V2D_ALIGN_NO_NEG_X) && !(v2d->align & V2D_ALIGN_NO_POS_X)) {
      /* width is in positive-x half */
      if (v2d->cur.xmin < 0) {
        v2d->cur.xmax -= v2d->cur.xmin;
        v2d->cur.xmin = 0.0f;
      }
    }

    /* handle height - posx and negx flags are mutually exclusive, so watch out */
    if ((v2d->align & V2D_ALIGN_NO_POS_Y) && !(v2d->align & V2D_ALIGN_NO_NEG_Y)) {
      /* height is in negative-y half */
      if (v2d->cur.ymax > 0) {
        v2d->cur.ymin -= v2d->cur.ymax;
        v2d->cur.ymax = 0.0f;
      }
    } else if ((v2d->align & V2D_ALIGN_NO_NEG_Y) && !(v2d->align & V2D_ALIGN_NO_POS_Y)) {
      /* height is in positive-y half */
      if (v2d->cur.ymin < 0) {
        v2d->cur.ymax -= v2d->cur.ymin;
        v2d->cur.ymin = 0.0f;
      }
    }
  }

  /* set masks */
  view2d_masks(v2d, nullptr);
}

void UI_view2d_curRect_validate(View2D *v2d)
{
  ui_view2d_curRect_validate_resize(v2d, false);
}

void UI_view2d_curRect_changed(const kContext *C, View2D *v2d)
{
  UI_view2d_curRect_validate(v2d);

  ARegion *region = CTX_wm_region(C);

  if (region->type->on_view2d_changed != nullptr) {
    region->type->on_view2d_changed(C, region);
  }
}

void UI_view2d_region_to_view_rctf(const View2D *v2d, const rctf *rect_src, rctf *rect_dst)
{
  const float cur_size[2] = {KLI_rctf_size_x(&v2d->cur), KLI_rctf_size_y(&v2d->cur)};
  const float mask_size[2] = {float(KLI_rcti_size_x(&v2d->mask)),
                              float(KLI_rcti_size_y(&v2d->mask))};

  rect_dst->xmin = (v2d->cur.xmin +
                    (cur_size[0] * (rect_src->xmin - v2d->mask.xmin) / mask_size[0]));
  rect_dst->xmax = (v2d->cur.xmin +
                    (cur_size[0] * (rect_src->xmax - v2d->mask.xmin) / mask_size[0]));
  rect_dst->ymin = (v2d->cur.ymin +
                    (cur_size[1] * (rect_src->ymin - v2d->mask.ymin) / mask_size[1]));
  rect_dst->ymax = (v2d->cur.ymin +
                    (cur_size[1] * (rect_src->ymax - v2d->mask.ymin) / mask_size[1]));
}
