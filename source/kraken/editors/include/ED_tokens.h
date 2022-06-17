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
 * Editors.
 * Tools for Artists.
 */

#pragma once

#include <wabi/base/tf/staticTokens.h>
#include <wabi/wabi.h>

#include "ED_view3d.h"

WABI_NAMESPACE_BEGIN

/* clang-format off */
#define VIEW3D_RENDERER_TOKENS                          \
  ((RENDERER_STORM,      "HdStormRendererPlugin"))       \
  ((RENDERER_ARNOLD,    "HdArnoldRendererPlugin"))      \
  ((RENDERER_RENDERMAN, "HdPrmanLoaderRendererPlugin")) \
  ((RENDERER_EMBREE,    "HdEmbreeRendererPlugin"))      \
  ((RENDERER_MITSUBA,   "HdMitsubaRendererPlugin"))     \
  ((RENDERER_CYCLES,    "HdCyclesRendererPlugin"))      \
  ((RENDERER_PRORENDER, "HdRprPlugin"))                 \
  ((RENDERER_REDSHIFT,  "HdRedshiftRendererPlugin"))    \
  ((RENDERER_VRAY,      "HdVrayRendererPlugin"))        \
  ((RENDERER_3DELIGHT,  "Hd3DelightRendererPlugin"))
/* clang-format on */

TF_DECLARE_PUBLIC_TOKENS(View3DRendererTokens, VIEW3D_EDITOR_API, VIEW3D_RENDERER_TOKENS);

WABI_NAMESPACE_END
