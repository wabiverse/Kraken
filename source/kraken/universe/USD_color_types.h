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
 * Universe.
 * Set the Stage.
 */

#pragma once

#include "USD_api.h"

KRAKEN_NAMESPACE_BEGIN

struct CurveMapPoint {
  float x, y;
  /** Shorty for result lookup. */
  short flag, shorty;
};

/** #CurveMapPoint.flag */
enum {
  CUMA_SELECT = (1 << 0),
  CUMA_HANDLE_VECTOR = (1 << 1),
  CUMA_HANDLE_AUTO_ANIM = (1 << 2),
};

struct CurveMap {
  short totpoint;

  /** Quick multiply value for reading table. */
  float range;
  /** The x-axis range for the table. */
  float mintable, maxtable;
  /** For extrapolated curves, the direction vector. */
  float ext_in[2], ext_out[2];
  /** Actual curve. */
  CurveMapPoint *curve;
  /** Display and evaluate table. */
  CurveMapPoint *table;

  /** For RGB curves, pre-multiplied table. */
  CurveMapPoint *premultable;
  /** For RGB curves, pre-multiplied extrapolation vector. */
  float premul_ext_in[2];
  float premul_ext_out[2];
};

struct CurveMapping {
  /** Cur; for buttons, to show active curve. */
  int flag, cur;
  int preset;
  int changed_timestamp;

  /** Current rect, clip rect (is default rect too). */
  wabi::GfVec4f curr, clipr;

  /** Max 4 builtin curves per mapping struct now. */
  CurveMap cm[4];
  /** Black/white point (black[0] abused for current frame). */
  float black[3], white[3];
  /** Black/white point multiply value, for speed. */
  float bwmul[3];

  /** Sample values, if flag set it draws line and intersection. */
  float sample[3];

  short tone;
};

KRAKEN_NAMESPACE_END