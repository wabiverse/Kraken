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

#include "kraken/kraken.h"

#include "USD_api.h"

#include "KLI_api.h"
#include "KLI_assert.h"
#include "KLI_rect.h"

KRAKEN_NAMESPACE_BEGIN

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


void KLI_rcti_rctf_copy_round(wabi::GfVec4i *dst, const wabi::GfVec4f &src)
{
  *dst = GfVec4i(floorf(src[0] + 0.5f),
                 floorf(src[1] + 0.5f),
                 floorf(src[2] + 0.5f),
                 floorf(src[3] + 0.5f));
}

KRAKEN_NAMESPACE_END