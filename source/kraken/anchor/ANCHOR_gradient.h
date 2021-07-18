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
 * Copyright 2021, Wabi.
 */

#pragma once

/**
 * @file
 * ⚓︎ Anchor.
 * Bare Metal.
 */

#include "ANCHOR_api.h"

#include <cstddef>

namespace AnchorGradient
{
   struct Delegate
   {
      virtual size_t GetPointCount() = 0;
      virtual wabi::GfVec4f* GetPoints() = 0;
      virtual int EditPoint(int pointIndex, wabi::GfVec4f value) = 0;
      virtual wabi::GfVec4f GetPoint(float t) = 0;
      virtual void AddPoint(wabi::GfVec4f value) = 0;
   };

   bool Edit(Delegate& delegate, const wabi::GfVec2f& size, int& selection);
}
