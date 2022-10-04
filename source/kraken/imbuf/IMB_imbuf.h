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

#pragma once

/**
 * @file
 * @ingroup IMBUF
 * Image Manipulation.
 */

#include "../kraklib/KLI_sys_types.h"
#include "../gpu/GPU_texture.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IM_MAX_SPACE 64

struct ImBuf;
struct rctf;
struct rcti;

struct ColorManagedDisplay;

void IMB_init(void);
void IMB_exit(void);

#ifdef __cplusplus
}
#endif