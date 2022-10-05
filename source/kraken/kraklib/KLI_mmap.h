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
 * Derived from original work by Copyright 2022, Blender Foundation.
 * From the Blender Library. (source/blender/blenlib).
 *
 * With any additions or modifications specific to Kraken.
 *
 * Modifications Copyright 2022, Wabi Animation Studios, Ltd. Co.
 */

#pragma once

/**
 * @file
 * KRAKEN Library.
 * Gadget Vault.
 */

#include "KLI_compiler_attrs.h"
#include "KLI_utildefines.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Memory-mapped file IO that implements all the OS-specific details and error handling. */

struct KLI_mmap_file;

typedef struct KLI_mmap_file KLI_mmap_file;

/* Prepares an opened file for memory-mapped IO.
 * May return NULL if the operation fails.
 * Note that this seeks to the end of the file to determine its length. */
KLI_mmap_file *KLI_mmap_open(int fd) ATTR_MALLOC ATTR_WARN_UNUSED_RESULT;

/* Reads length bytes from file at the given offset into dest.
 * Returns whether the operation was successful (may fail when reading beyond the file
 * end or when IO errors occur). */
bool KLI_mmap_read(KLI_mmap_file *file, void *dest, size_t offset, size_t length)
    ATTR_WARN_UNUSED_RESULT ATTR_NONNULL(1);

void *KLI_mmap_get_pointer(KLI_mmap_file *file) ATTR_WARN_UNUSED_RESULT;

void KLI_mmap_free(KLI_mmap_file *file) ATTR_NONNULL(1);

#ifdef __cplusplus
}
#endif
