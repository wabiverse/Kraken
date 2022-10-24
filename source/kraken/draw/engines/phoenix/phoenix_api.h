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

#ifndef __PHOENIX_RENDER_ENGINE_API_H__
#define __PHOENIX_RENDER_ENGINE_API_H__

/**
 * @file Draw.
 * Spontaneous Expression.
 *
 * The Phoenix Render Engine.
 * The OpenSubdiv-based real-time render engine of the 21st century.
 */

#ifdef __cplusplus

#include <wabi/base/arch/export.h>

#if defined(WABI_STATIC)
#  define PHOENIX_API
#  define PHOENIX_API_TEMPLATE_CLASS(...)
#  define PHOENIX_API_TEMPLATE_STRUCT(...)
#  define PHOENIX_LOCAL
#else
#  if defined(PHOENIX_EXPORTS)
#    define PHOENIX_API ARCH_EXPORT
#    define PHOENIX_API_TEMPLATE_CLASS(...) ARCH_EXPORT_TEMPLATE(class, __VA_ARGS__)
#    define PHOENIX_API_TEMPLATE_STRUCT(...) ARCH_EXPORT_TEMPLATE(struct, __VA_ARGS__)
#  else
#    define PHOENIX_API ARCH_IMPORT
#    define PHOENIX_API_TEMPLATE_CLASS(...) ARCH_IMPORT_TEMPLATE(class, __VA_ARGS__)
#    define PHOENIX_API_TEMPLATE_STRUCT(...) ARCH_IMPORT_TEMPLATE(struct, __VA_ARGS__)
#  endif
#  define PHOENIX_LOCAL ARCH_HIDDEN
#endif

#endif /* __cplusplus */

#endif /* __PHOENIX_RENDER_ENGINE_API_H__ */
