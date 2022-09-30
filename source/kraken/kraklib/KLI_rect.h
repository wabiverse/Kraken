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

#pragma once

#include "USD_vec_types.h"

#include "KLI_assert.h"
#include "KLI_string.h"
#include "KLI_utildefines.h"

#ifdef __cplusplus
#  include <wabi/base/arch/attributes.h>
#  include <wabi/base/gf/vec4i.h>
#  include <wabi/base/gf/vec4f.h>

void KLI_rcti_rctf_copy_round(wabi::GfVec4i *dst, const wabi::GfVec4f &src);
bool KLI_rctf_isect_pt(const wabi::GfVec4f &rect, const float x, const float y);

KLI_INLINE float KLI_rctf_cent_x(const wabi::GfVec4f &rct)
{
  return (rct[0] + rct[1]) / 2.0;
}
KLI_INLINE float KLI_rctf_cent_y(const wabi::GfVec4f &rct)
{
  return (rct[2] + rct[3]) / 2.0;
}

/* ----- integers. ----- */

KLI_INLINE int KLI_rcti_size_x(const wabi::GfVec4i &rct)
{
  return (&rct[1] - &rct[0]);
}

KLI_INLINE int KLI_rcti_size_y(const wabi::GfVec4i &rct)
{
  return (&rct[3] - &rct[2]);
}

/* ----- floats. ----- */

KLI_INLINE float KLI_rctf_size_x(const wabi::GfVec4f &rct)
{
  return (&rct[1] - &rct[0]);
}

KLI_INLINE float KLI_rctf_size_y(const wabi::GfVec4f &rct)
{
  return (&rct[3] - &rct[2]);
}
#endif /* __cplusplus */

void KLI_rctf_init_minmax(rctf *rect);

void KLI_rctf_rcti_copy(struct rctf *dst, const struct rcti *src);
void KLI_rcti_rctf_copy(struct rcti *dst, const struct rctf *src);
bool KLI_rcti_clamp(struct rcti *rect, const struct rcti *rect_bounds, int r_xy[2]);

bool KLI_rctf_isect(const struct rctf *src1, const struct rctf *src2, struct rctf *dest);
bool KLI_rcti_isect(const struct rcti *src1, const struct rcti *src2, struct rcti *dest);
bool KLI_rcti_isect_pt(const struct rcti *rect, int x, int y);
bool KLI_rcti_isect_pt_v(const struct rcti *rect, const int xy[2]);
bool KLI_rctf_isect_pt_v(const rctf *rect, const float xy[2]);
bool KLI_rctf_isect_segment(const struct rctf *rect, const float s1[2], const float s2[2]);
bool KLI_rctf_isect_rect_y(const rctf *src1, const rctf *src2, float range_y[2]);
bool KLI_rctf_isect_rect_x(const rctf *src1, const rctf *src2, float range_x[2]);

void KLI_rcti_rctf_copy_floor(struct rcti *dst, const struct rctf *src);
void KLI_rcti_translate(struct rcti *rect, int x, int y);
void KLI_rctf_translate(struct rctf *rect, float x, float y);

void KLI_rctf_union(struct rctf *rct_a, const struct rctf *rct_b);
