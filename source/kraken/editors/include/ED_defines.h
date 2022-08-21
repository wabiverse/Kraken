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

/* std */
#include <cmath>
#include <functional>
#include <iostream>
#include <memory>
#include <set>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

/* base */
#include <wabi/base/gf/vec2i.h>
#include <wabi/base/gf/vec4f.h>
#include <wabi/base/tf/token.h>

/* imaging */
#include <wabi/imaging/hio/types.h>

/* usd */
#include <wabi/usd/usd/prim.h>
#include <wabi/usd/usd/timeCode.h>
#include <wabi/usd/usdGeom/basisCurves.h>

/* kraklib */
#include "KLI_icons.h"
#include "KLI_utildefines.h"

#define RGN_MARGINS 0, 0, 0, 0
#define RGN_SPACING 0
