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
 * KRAKEN Library.
 * Gadget Vault.
 */

#include "USD_api.h"

#include "KLI_api.h"
#include "KLI_assert.h"
#include "KLI_rect.h"

#include <wabi/base/gf/vec2i.h>
#include <wabi/base/gf/vec4i.h>
#include <wabi/base/gf/vec4f.h>

void KLI_rctf_init_minmax(rctf *rect)
{
  rect->xmin = rect->ymin = FLT_MAX;
  rect->xmax = rect->ymax = -FLT_MAX;
}

bool KLI_rctf_isect_pt(const wabi::GfVec4f &rect, const float x, const float y)
{
  if (x < GET_X(rect)) {
    return false;
  }
  if (x > GET_Y(rect)) {
    return false;
  }
  if (y < GET_Z(rect)) {
    return false;
  }
  if (y > GET_W(rect)) {
    return false;
  }
  return true;
}

bool KLI_rcti_isect_pt(const rcti *rect, const int x, const int y)
{
  if (x < rect->xmin) {
    return false;
  }
  if (x > rect->xmax) {
    return false;
  }
  if (y < rect->ymin) {
    return false;
  }
  if (y > rect->ymax) {
    return false;
  }
  return true;
}

bool KLI_rcti_clamp(rcti *rect, const rcti *rect_bounds, int r_xy[2])
{
  bool changed = false;

  r_xy[0] = 0;
  r_xy[1] = 0;

  if (rect->xmax > rect_bounds->xmax) {
    int ofs = rect_bounds->xmax - rect->xmax;
    rect->xmin += ofs;
    rect->xmax += ofs;
    r_xy[0] += ofs;
    changed = true;
  }

  if (rect->xmin < rect_bounds->xmin) {
    int ofs = rect_bounds->xmin - rect->xmin;
    rect->xmin += ofs;
    rect->xmax += ofs;
    r_xy[0] += ofs;
    changed = true;
  }

  if (rect->ymin < rect_bounds->ymin) {
    int ofs = rect_bounds->ymin - rect->ymin;
    rect->ymin += ofs;
    rect->ymax += ofs;
    r_xy[1] += ofs;
    changed = true;
  }

  if (rect->ymax > rect_bounds->ymax) {
    int ofs = rect_bounds->ymax - rect->ymax;
    rect->ymin += ofs;
    rect->ymax += ofs;
    r_xy[1] += ofs;
    changed = true;
  }

  return changed;
}

bool KLI_rcti_isect_pt_v(const rcti *rect, const int xy[2])
{
  if (xy[0] < rect->xmin) {
    return false;
  }
  if (xy[0] > rect->xmax) {
    return false;
  }
  if (xy[1] < rect->ymin) {
    return false;
  }
  if (xy[1] > rect->ymax) {
    return false;
  }
  return true;
}


void KLI_rcti_rctf_copy(rcti *dst, const rctf *src)
{
  dst->xmin = floorf(src->xmin + 0.5f);
  dst->xmax = dst->xmin +
              floorf(KLI_rctf_size_x(GfVec4f(src->xmin, src->xmax, src->ymin, src->ymax)) + 0.5f);
  dst->ymin = floorf(src->ymin + 0.5f);
  dst->ymax = dst->ymin +
              floorf(KLI_rctf_size_y(GfVec4f(src->xmin, src->xmax, src->ymin, src->ymax)) + 0.5f);
}

void KLI_rcti_rctf_copy_round(wabi::GfVec4i *dst, const wabi::GfVec4f &src)
{
  *dst = GfVec4i(floorf(src[0] + 0.5f),
                 floorf(src[1] + 0.5f),
                 floorf(src[2] + 0.5f),
                 floorf(src[3] + 0.5f));
}

void KLI_rcti_rctf_copy_floor(rcti *dst, const rctf *src)
{
  dst->xmin = floorf(src->xmin);
  dst->xmax = floorf(src->xmax);
  dst->ymin = floorf(src->ymin);
  dst->ymax = floorf(src->ymax);
}

void KLI_rctf_rcti_copy(rctf *dst, const rcti *src)
{
  dst->xmin = src->xmin;
  dst->xmax = src->xmax;
  dst->ymin = src->ymin;
  dst->ymax = src->ymax;
}

void KLI_rcti_translate(rcti *rect, int x, int y)
{
  rect->xmin += x;
  rect->ymin += y;
  rect->xmax += x;
  rect->ymax += y;
}

void KLI_rctf_translate(rctf *rect, float x, float y)
{
  rect->xmin += x;
  rect->ymin += y;
  rect->xmax += x;
  rect->ymax += y;
}


bool KLI_rctf_isect(const rctf *src1, const rctf *src2, rctf *dest)
{
  float xmin, xmax;
  float ymin, ymax;

  xmin = (src1->xmin) > (src2->xmin) ? (src1->xmin) : (src2->xmin);
  xmax = (src1->xmax) < (src2->xmax) ? (src1->xmax) : (src2->xmax);
  ymin = (src1->ymin) > (src2->ymin) ? (src1->ymin) : (src2->ymin);
  ymax = (src1->ymax) < (src2->ymax) ? (src1->ymax) : (src2->ymax);

  if (xmax >= xmin && ymax >= ymin) {
    if (dest) {
      dest->xmin = xmin;
      dest->xmax = xmax;
      dest->ymin = ymin;
      dest->ymax = ymax;
    }
    return true;
  }

  if (dest) {
    dest->xmin = 0;
    dest->xmax = 0;
    dest->ymin = 0;
    dest->ymax = 0;
  }
  return false;
}

bool KLI_rcti_isect(const rcti *src1, const rcti *src2, rcti *dest)
{
  int xmin, xmax;
  int ymin, ymax;

  xmin = (src1->xmin) > (src2->xmin) ? (src1->xmin) : (src2->xmin);
  xmax = (src1->xmax) < (src2->xmax) ? (src1->xmax) : (src2->xmax);
  ymin = (src1->ymin) > (src2->ymin) ? (src1->ymin) : (src2->ymin);
  ymax = (src1->ymax) < (src2->ymax) ? (src1->ymax) : (src2->ymax);

  if (xmax >= xmin && ymax >= ymin) {
    if (dest) {
      dest->xmin = xmin;
      dest->xmax = xmax;
      dest->ymin = ymin;
      dest->ymax = ymax;
    }
    return true;
  }

  if (dest) {
    dest->xmin = 0;
    dest->xmax = 0;
    dest->ymin = 0;
    dest->ymax = 0;
  }
  return false;
}

void KLI_rctf_union(rctf *rct_a, const rctf *rct_b)
{
  if (rct_a->xmin > rct_b->xmin) {
    rct_a->xmin = rct_b->xmin;
  }
  if (rct_a->xmax < rct_b->xmax) {
    rct_a->xmax = rct_b->xmax;
  }
  if (rct_a->ymin > rct_b->ymin) {
    rct_a->ymin = rct_b->ymin;
  }
  if (rct_a->ymax < rct_b->ymax) {
    rct_a->ymax = rct_b->ymax;
  }
}

bool KLI_rctf_isect_pt_v(const rctf *rect, const float xy[2])
{
  if (xy[0] < rect->xmin) {
    return false;
  }
  if (xy[0] > rect->xmax) {
    return false;
  }
  if (xy[1] < rect->ymin) {
    return false;
  }
  if (xy[1] > rect->ymax) {
    return false;
  }
  return true;
}

static int isect_segments_fl(const float v1[2],
                             const float v2[2],
                             const float v3[2],
                             const float v4[2])
{
  const double div = (double)((v2[0] - v1[0]) * (v4[1] - v3[1]) -
                              (v2[1] - v1[1]) * (v4[0] - v3[0]));
  if (div == 0.0) {
    return 1; /* co-linear */
  }

  const double lambda = (double)((v1[1] - v3[1]) * (v4[0] - v3[0]) -
                                 (v1[0] - v3[0]) * (v4[1] - v3[1])) /
                        div;
  const double mu = (double)((v1[1] - v3[1]) * (v2[0] - v1[0]) -
                             (v1[0] - v3[0]) * (v2[1] - v1[1])) /
                    div;
  return (lambda >= 0.0 && lambda <= 1.0 && mu >= 0.0 && mu <= 1.0);
}

bool KLI_rctf_isect_rect_x(const rctf *src1, const rctf *src2, float range_x[2])
{
  const float xmin = (src1->xmin) > (src2->xmin) ? (src1->xmin) : (src2->xmin);
  const float xmax = (src1->xmax) < (src2->xmax) ? (src1->xmax) : (src2->xmax);

  if (xmax >= xmin) {
    if (range_x) {
      range_x[0] = xmin;
      range_x[1] = xmax;
    }
    return true;
  }

  if (range_x) {
    range_x[0] = 0;
    range_x[1] = 0;
  }
  return false;
}

bool KLI_rctf_isect_rect_y(const rctf *src1, const rctf *src2, float range_y[2])
{
  const float ymin = (src1->ymin) > (src2->ymin) ? (src1->ymin) : (src2->ymin);
  const float ymax = (src1->ymax) < (src2->ymax) ? (src1->ymax) : (src2->ymax);

  if (ymax >= ymin) {
    if (range_y) {
      range_y[0] = ymin;
      range_y[1] = ymax;
    }
    return true;
  }

  if (range_y) {
    range_y[0] = 0;
    range_y[1] = 0;
  }
  return false;
}

bool KLI_rctf_isect_segment(const rctf *rect, const float s1[2], const float s2[2])
{
  /* first do outside-bounds check for both points of the segment */
  if (s1[0] < rect->xmin && s2[0] < rect->xmin) {
    return false;
  }
  if (s1[0] > rect->xmax && s2[0] > rect->xmax) {
    return false;
  }
  if (s1[1] < rect->ymin && s2[1] < rect->ymin) {
    return false;
  }
  if (s1[1] > rect->ymax && s2[1] > rect->ymax) {
    return false;
  }

  /* if either points intersect then we definitely intersect */
  if (KLI_rctf_isect_pt_v(rect, s1) || KLI_rctf_isect_pt_v(rect, s2)) {
    return true;
  }

  /* both points are outside but may intersect the rect */
  float tvec1[2];
  float tvec2[2];
  /* diagonal: [/] */
  tvec1[0] = rect->xmin;
  tvec1[1] = rect->ymin;
  tvec2[0] = rect->xmax;
  tvec2[1] = rect->ymax;
  if (isect_segments_fl(s1, s2, tvec1, tvec2)) {
    return true;
  }

  /* diagonal: [\] */
  tvec1[0] = rect->xmin;
  tvec1[1] = rect->ymax;
  tvec2[0] = rect->xmax;
  tvec2[1] = rect->ymin;
  if (isect_segments_fl(s1, s2, tvec1, tvec2)) {
    return true;
  }

  /* no intersection */
  return false;
}