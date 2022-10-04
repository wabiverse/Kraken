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

#include "ED_defines.h"

#if defined(WABI_STATIC)
#  define CODE_EDITOR_API
#  define CODE_EDITOR_API_TEMPLATE_CLASS(...)
#  define CODE_EDITOR_API_TEMPLATE_STRUCT(...)
#  define CODE_EDITOR_LOCAL
#else
#  if defined(CODE_EDITOR_EXPORTS)
#    define CODE_EDITOR_API ARCH_EXPORT
#    define CODE_EDITOR_API_TEMPLATE_CLASS(...) ARCH_EXPORT_TEMPLATE(class, __VA_ARGS__)
#    define CODE_EDITOR_API_TEMPLATE_STRUCT(...) ARCH_EXPORT_TEMPLATE(struct, __VA_ARGS__)
#  else
#    define CODE_EDITOR_API ARCH_IMPORT
#    define CODE_EDITOR_API_TEMPLATE_CLASS(...) ARCH_IMPORT_TEMPLATE(class, __VA_ARGS__)
#    define CODE_EDITOR_API_TEMPLATE_STRUCT(...) ARCH_IMPORT_TEMPLATE(struct, __VA_ARGS__)
#  endif
#  define CODE_EDITOR_LOCAL ARCH_HIDDEN
#endif

CODE_EDITOR_API
void ED_code_run(bool *show = NULL);
