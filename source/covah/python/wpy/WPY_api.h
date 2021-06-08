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
 * COVAH Python.
 * It Bites.
 */

#ifndef COVAH_PYTHON_API_H
#define COVAH_PYTHON_API_H

#include "CLI_utildefines.h"

#if defined(WABI_STATIC)
#  define COVAH_PYTHON_API
#  define COVAH_PYTHON_API_TEMPLATE_CLASS(...)
#  define COVAH_PYTHON_API_TEMPLATE_STRUCT(...)
#  define COVAH_PYTHON_LOCAL
#else
#  if defined(COVAH_PYTHON_EXPORTS)
#    define COVAH_PYTHON_API ARCH_EXPORT
#    define COVAH_PYTHON_API_TEMPLATE_CLASS(...) ARCH_EXPORT_TEMPLATE(class, __VA_ARGS__)
#    define COVAH_PYTHON_API_TEMPLATE_STRUCT(...) ARCH_EXPORT_TEMPLATE(struct, __VA_ARGS__)
#  else
#    define COVAH_PYTHON_API ARCH_IMPORT
#    define COVAH_PYTHON_API_TEMPLATE_CLASS(...) ARCH_IMPORT_TEMPLATE(class, __VA_ARGS__)
#    define COVAH_PYTHON_API_TEMPLATE_STRUCT(...) ARCH_IMPORT_TEMPLATE(struct, __VA_ARGS__)
#  endif
#  define COVAH_PYTHON_LOCAL ARCH_HIDDEN
#endif

#endif /* COVAH_PYTHON_API_H */