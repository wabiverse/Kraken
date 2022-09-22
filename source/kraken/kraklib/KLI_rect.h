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

#include "KLI_api.h"
#include "KLI_assert.h"
#include "KLI_string_utils.h"
#include "KLI_utildefines.h"

#include <wabi/base/arch/attributes.h>
#include <wabi/base/gf/vec4i.h>
#include <wabi/base/gf/vec4f.h>

KRAKEN_NAMESPACE_BEGIN

bool KLI_rctf_isect_pt(const wabi::GfVec4f &rect, const float x, const float y);
void KLI_rcti_rctf_copy_round(wabi::GfVec4i *dst, const wabi::GfVec4f &src);

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

KRAKEN_NAMESPACE_END