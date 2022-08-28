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

#include "USD_path_defaults.h"

#include "KLI_utildefines.h"

#include <wabi/base/arch/defines.h>
#include <wabi/usd/usd/attribute.h>

#include <cstddef>
#include <iterator>

#if defined(WABI_STATIC)
#  define KRAKEN_USD_API
#  define KRAKEN_USD_API_TEMPLATE_CLASS(...)
#  define KRAKEN_USD_API_TEMPLATE_STRUCT(...)
#  define KRAKEN_USD_LOCAL
#else
#  if defined(KRAKEN_USD_EXPORTS)
#    define KRAKEN_USD_API ARCH_EXPORT
#    define KRAKEN_USD_API_TEMPLATE_CLASS(...) ARCH_EXPORT_TEMPLATE(class, __VA_ARGS__)
#    define KRAKEN_USD_API_TEMPLATE_STRUCT(...) ARCH_EXPORT_TEMPLATE(struct, __VA_ARGS__)
#  else
#    define KRAKEN_USD_API ARCH_IMPORT
#    define KRAKEN_USD_API_TEMPLATE_CLASS(...) ARCH_IMPORT_TEMPLATE(class, __VA_ARGS__)
#    define KRAKEN_USD_API_TEMPLATE_STRUCT(...) ARCH_IMPORT_TEMPLATE(struct, __VA_ARGS__)
#  endif
#  define KRAKEN_USD_LOCAL ARCH_HIDDEN
#endif

#define KRAKEN_DECLARE_STATIC_TOKEN(x) const TfToken x
#define KRAKEN_DEFINE_STATIC_TOKEN(y) y(STRINGIFY_APPEND("", y), TfToken::Immortal)
#define IDNAME(z) KRAKEN_OPERATOR_TOKENS->z

/* Switch Get() to Define() for production. */
#define KRAKEN_STAGE_CREATE_CHILD(i) Define(CTX_data_stage(i), prim->path.AppendPath(stagepath))
#define KRAKEN_STAGE_CREATE(g) Define(CTX_data_stage(g), stagepath)
#define KRAKEN_PRIM_OPERATOR_CREATE(x, y) \
  CTX_data_stage(x)->DefinePrim(          \
    SdfPath(KRAKEN_PATH_DEFAULTS::KRAKEN_OPERATORS).AppendPath(SdfPath(y)))

#define GET_X(x) x[0]
#define GET_Y(y) y[1]
#define GET_Z(z) z[2]
#define GET_W(w) w[3]

#define VEC2_SET(vx2, vx1x, vx2y) vx2->Set(vx1x, vx2y)
#define VEC3_SET(vx3, vx1x, vx2y, vx3z) vx3->Set(vx1x, vx2y, vx3z)
#define VEC4_SET(vx4, vx1x, vx2y, vx3z, vx4w) vx4->Set(vx1x, vx2y, vx3z, vx4w)
#define SET_VEC2(vx2, vx1x, vx2y) vx2.Set(vx1x, vx2y)
#define SET_VEC3(vx3, vx1x, vx2y, vx3z) vx3.Set(vx1x, vx2y, vx3z)
#define SET_VEC4(vx4, vx1x, vx2y, vx3z, vx4w) vx4.Set(vx1x, vx2y, vx3z, vx4w)

#define UNIVERSE_INSERT_WINDOW(m, h, v) m->windows.insert(std::make_pair(h, v))
#define HASH(x) x.first
#define VALUE(y) y.second
#define UNIVERSE_FOR_ALL(iter, c) for (const auto &iter : c)
#define UNIVERSE_MUTABLE_FOR_ALL(iter, c) for (auto &iter : c)

#define FILE_MAX 1024
#define USD_MAX_TIME 80

#define TIMECODE_DEFAULT UsdTimeCode::Default()

#define DEFAULT_VALUE(v) VtValue(v)
#define DEFAULT_TOKEN(t) VtValue(TfToken(t))
#define DEFAULT_ASSET(a) VtValue(SdfAssetPath(a))
#define DEFAULT_VEC2F(v1f, v2f) VtValue(GfVec2f(v1f, v2f))

#define KRAKEN_FILE_VERSION_HEADER                                    \
  (std::string("Kraken v" + wabi::TfStringify(KRAKEN_VERSION_MAJOR) + "." + \
               wabi::TfStringify(KRAKEN_VERSION_MINOR)))

enum eIconSizes
{
  ICON_SIZE_ICON = 0,
  ICON_SIZE_PREVIEW = 1,

  NUM_ICON_SIZES,
};