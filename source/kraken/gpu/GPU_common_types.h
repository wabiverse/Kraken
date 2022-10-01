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
 * @ingroup GPU.
 * Pixel Magic.
 *
 * This interface allow GPU to manage VAOs for multiple context and threads.
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef enum eGPULoadOp
{
  GPU_LOADACTION_CLEAR = 0,
  GPU_LOADACTION_LOAD,
  GPU_LOADACTION_DONT_CARE
} eGPULoadOp;

typedef enum eGPUStoreOp
{
  GPU_STOREACTION_STORE = 0,
  GPU_STOREACTION_DONT_CARE
} eGPUStoreOp;

typedef enum eGPUFrontFace
{
  GPU_CLOCKWISE,
  GPU_COUNTERCLOCKWISE,
} eGPUFrontFace;

#ifdef __cplusplus
}
#endif
