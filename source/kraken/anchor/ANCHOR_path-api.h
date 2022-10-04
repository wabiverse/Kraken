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
 * ⚓︎ Anchor.
 * Bare Metal.
 *
 * Anchor system paths API.
 * Compliant with C.
 */

#pragma once

#include "ANCHOR_types.h"

#ifdef __cplusplus
extern "C" {
#endif

ANCHOR_DECLARE_HANDLE(AnchorSystemPathsHandle);

/**
 * Creates the one and only instance of the system path access.
 * @return An indication of success. */
eAnchorStatus ANCHOR_CreateSystemPaths(void);

/**
 * Disposes the one and only system.
 * @return An indication of success. */
eAnchorStatus ANCHOR_DisposeSystemPaths(void);

/**
 * Determine the base dir in which shared resources are located. It will first try to use
 * "unpack and run" path, then look for properly installed path, including versioning.
 * @return Unsigned char string pointing to system dir (eg /usr/share/kraken/). */
const char *ANCHOR_getSystemDir(int version, const char *versionstr);

/**
 * Determine the base dir in which user configuration is stored, including versioning.
 * @return Unsigned char string pointing to user dir (eg ~). */
const char *ANCHOR_getUserDir(int version, const char *versionstr);

/**
 * Determine a special ("well known") and easy to reach user directory.
 * @return Unsigned char string pointing to user dir (eg `~/Documents/`). */
const char *ANCHOR_getUserSpecialDir(eAnchorUserSpecialDirTypes type);

/**
 * Determine the dir in which the binary file is found.
 * @return Unsigned char string pointing to binary dir (eg ~/usr/local/bin/). */
const char *ANCHOR_getBinaryDir(void);

/**
 * Add the file to the operating system most recently used files */
void ANCHOR_addToSystemRecentFiles(const char *filename);

#ifdef __cplusplus
}
#endif
