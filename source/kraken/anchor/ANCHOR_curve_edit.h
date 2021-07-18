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

#include <stdint.h>
#include "ANCHOR_api.h"

struct AnchorBBox;

namespace AnchorCurveEdit
{
   enum CurveType
   {
      CurveNone,
      CurveDiscrete,
      CurveLinear,
      CurveSmooth,
      CurveBezier,
   };

   struct EditPoint
   {
      int curveIndex;
      int pointIndex;
      bool operator <(const EditPoint& other) const
      {
         if (curveIndex < other.curveIndex)
            return true;
         if (curveIndex > other.curveIndex)
            return false;

         if (pointIndex < other.pointIndex)
            return true;
         return false;
      }
   };

   struct Delegate
   {
      bool focused = false;
      virtual size_t GetCurveCount() = 0;
      virtual bool IsVisible(size_t /*curveIndex*/) { return true; }
      virtual CurveType GetCurveType(size_t /*curveIndex*/) const { return CurveLinear; }
      virtual wabi::GfVec2f& GetMin() = 0;
      virtual wabi::GfVec2f& GetMax() = 0;
      virtual size_t GetPointCount(size_t curveIndex) = 0;
      virtual uint32_t GetCurveColor(size_t curveIndex) = 0;
      virtual wabi::GfVec2f* GetPoints(size_t curveIndex) = 0;
      virtual int EditPoint(size_t curveIndex, int pointIndex, wabi::GfVec2f value) = 0;
      virtual void AddPoint(size_t curveIndex, wabi::GfVec2f value) = 0;
      virtual unsigned int GetBackgroundColor() { return 0xFF202020; }
      // handle undo/redo thru this functions
      virtual void BeginEdit(int /*index*/) {}
      virtual void EndEdit() {}
   };

   int Edit(Delegate& delegate, const wabi::GfVec2f& size, unsigned int id, const AnchorBBox* clippingRect = NULL, std::vector<EditPoint>* selectedPoints = NULL);
}
