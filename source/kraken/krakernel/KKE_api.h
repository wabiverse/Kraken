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
 * KRAKEN Kernel.
 * Purple Underground.
 */

#ifndef KRAKEN_KERNEL_API_H
#define KRAKEN_KERNEL_API_H

#include "KLI_utildefines.h"

#include <wabi/usd/sdf/path.h>

#if defined(WABI_STATIC)
#  define KRAKEN_KERNEL_API
#  define KRAKEN_KERNEL_API_TEMPLATE_CLASS(...)
#  define KRAKEN_KERNEL_API_TEMPLATE_STRUCT(...)
#  define KRAKEN_KERNEL_LOCAL
#else
#  if defined(KRAKEN_KERNEL_EXPORTS)
#    define KRAKEN_KERNEL_API ARCH_EXPORT
#    define KRAKEN_KERNEL_API_TEMPLATE_CLASS(...) ARCH_EXPORT_TEMPLATE(class, __VA_ARGS__)
#    define KRAKEN_KERNEL_API_TEMPLATE_STRUCT(...) ARCH_EXPORT_TEMPLATE(struct, __VA_ARGS__)
#  else
#    define KRAKEN_KERNEL_API ARCH_IMPORT
#    define KRAKEN_KERNEL_API_TEMPLATE_CLASS(...) ARCH_IMPORT_TEMPLATE(class, __VA_ARGS__)
#    define KRAKEN_KERNEL_API_TEMPLATE_STRUCT(...) ARCH_IMPORT_TEMPLATE(struct, __VA_ARGS__)
#  endif
#  define KRAKEN_KERNEL_LOCAL ARCH_HIDDEN
#endif

#ifdef __BIG_ENDIAN__
/* big endian */
#  define MAKE_ID2(c, d) ((c) << 8 | (d))
#else
/* little endian */
#  define MAKE_ID2(c, d) ((d) << 8 | (c))
#endif

#define KKE_UNDO_STR_MAX 64

#endif /* KRAKEN_KERNEL_API_H */
