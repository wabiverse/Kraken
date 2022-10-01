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
 * @file Draw.
 * Spontaneous Expression.
 *
 * List of defines that are shared with the GPUShaderCreateInfos. We do this to avoid
 * dragging larger headers into the createInfo pipeline which would cause problems.
 */

#define DRW_VIEW_UBO_SLOT 0

#define DRW_RESOURCE_ID_SLOT 11
#define DRW_OBJ_MAT_SLOT 10
#define DRW_OBJ_INFOS_SLOT 9
#define DRW_OBJ_ATTR_SLOT 8

#define DRW_DEBUG_PRINT_SLOT 15
#define DRW_DEBUG_DRAW_SLOT 14

#define DRW_COMMAND_GROUP_SIZE 64
#define DRW_FINALIZE_GROUP_SIZE 64
/* Must be multiple of 32. Set to 32 for shader simplicity. */
#define DRW_VISIBILITY_GROUP_SIZE 32
