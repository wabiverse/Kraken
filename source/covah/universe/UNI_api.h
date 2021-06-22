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

/**
 * @file
 * Universe.
 * Set the Stage.
 */

#pragma once

#include "UNI_path_defaults.h"

#include "CLI_utildefines.h"

#if defined(WABI_STATIC)
#  define COVAH_UNIVERSE_API
#  define COVAH_UNIVERSE_API_TEMPLATE_CLASS(...)
#  define COVAH_UNIVERSE_API_TEMPLATE_STRUCT(...)
#  define COVAH_UNIVERSE_LOCAL
#else
#  if defined(COVAH_UNIVERSE_EXPORTS)
#    define COVAH_UNIVERSE_API ARCH_EXPORT
#    define COVAH_UNIVERSE_API_TEMPLATE_CLASS(...) ARCH_EXPORT_TEMPLATE(class, __VA_ARGS__)
#    define COVAH_UNIVERSE_API_TEMPLATE_STRUCT(...) ARCH_EXPORT_TEMPLATE(struct, __VA_ARGS__)
#  else
#    define COVAH_UNIVERSE_API ARCH_IMPORT
#    define COVAH_UNIVERSE_API_TEMPLATE_CLASS(...) ARCH_IMPORT_TEMPLATE(class, __VA_ARGS__)
#    define COVAH_UNIVERSE_API_TEMPLATE_STRUCT(...) ARCH_IMPORT_TEMPLATE(struct, __VA_ARGS__)
#  endif
#  define COVAH_UNIVERSE_LOCAL ARCH_HIDDEN
#endif

#define COVAH_DECLARE_STATIC_TOKEN(x) const TfToken x
#define COVAH_DEFINE_STATIC_TOKEN(y) y(STRINGIFY_APPEND("", y), TfToken::Immortal)
#define IDNAME(z) COVAH_OPERATOR_TOKENS->z

#define COVAH_UNIVERSE_CREATE_CHILD(i) Define(CTX_data_stage(i), prim->path.AppendPath(stagepath))
#define COVAH_UNIVERSE_CREATE(g) Define(CTX_data_stage(g), stagepath)
#define COVAH_PRIM_OPERATOR_CREATE(x, y) CTX_data_stage(x)->DefinePrim(SdfPath(COVAH_PATH_DEFAULTS::COVAH_OPERATORS).AppendPath(SdfPath(y)))

#define GET_X(x) x[0]
#define GET_Y(y) y[1]
#define GET_Z(z) z[2]
#define GET_W(w) w[3]

#define VEC2_SET(vx2, vx1x, vx2y) vx2->Set(vx1x, vx2y)
#define VEC3_SET(vx3, vx1x, vx2y, vx3z) vx3->Set(vx1x, vx2y, vx3z)
#define VEC4_SET(vx4, vx1x, vx2y, vx3z, vx4z) vx4->Set(vx1x, vx2y, vx3z, vx4z)

/** clang-format off */

#define AppendBool(__append) bool __append;
#define UniStageGetBool(__typed, __param, __value) AppendBool(__value) __typed->__param.Get(&__value)

#define AppendInt(__append) int __append;
#define UniStageGetInt(__typed, __param, __value) AppendInt(__value) __typed->__param.Get(&__value)

#define AppendFlt(__append) float __append;
#define UniStageGetFlt(__typed, __param, __value) AppendFlt(__value) __typed->__param.Get(&__value)

#define AppendToken(__append) TfToken __append;
#define UniStageGetToken(__typed, __param, __value) AppendToken(__value) __typed->__param.Get(&__value)

#define AppendAssetPath(__append) SdfAssetPath __append;
#define UniStageGetAsset(__typed, __param, __value) AppendAssetPath(__value) __typed->__param.Get(&__value)

#define AppendVecPath(__append) SdfPathVector __append;
#define UniStageGetTargets(__typed, __param, __value) AppendVecPath(__value) __typed->__param.GetTargets(&__value)

#define AppendVec2f(__append) GfVec2f __append;
#define UniStageGetVec2f(__typed, __param, __value) AppendVec2f(__value) __typed->__param.Get(&__value)

#define AppendVec2i(__append) GfVec2i __append;
#define UniStageGetVec2i(__typed, __param, __value) AppendVec2i(__value) __typed->__param.Get(&__value)

#define AppendVec3i(__append) GfVec3i __append;
#define UniStageGetVec3i(__typed, __param, __value) AppendVec3i(__value) __typed->__param.Get(&__value)

#define AppendVec3f(__append) GfVec3f __append;
#define UniStageGetVec3f(__typed, __param, __value) AppendVec3f(__value) __typed->__param.Get(&__value)

/** clang-format on */