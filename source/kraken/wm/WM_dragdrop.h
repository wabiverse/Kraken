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
 * Window Manager.
 * Making GUI Fly.
 */

#include "KLI_compiler_attrs.h"
#include "KLI_sys_types.h"

#include "USD_wm_types.h"
#include "USD_space_types.h"

#include "WM_keymap.h"

#ifdef __cplusplus
extern "C" {
#endif

struct AssetHandle;
struct ID;
struct kContext;
struct wmDrag;

wmDrag *WM_event_start_drag(struct kContext *C,
                            int icon,
                            int type,
                            void *poin,
                            double value,
                            unsigned int flags);
void WM_event_start_prepared_drag(struct kContext *C, struct wmDrag *drag);

wmDrag *WM_drag_data_create(struct kContext *C, int icon, int type, void *poin, double value, uint flags);
wmDragAsset *WM_drag_create_asset_data(const struct AssetHandle *asset,
                                       struct AssetMetaData *metadata,
                                       const char *path,
                                       int import_type);

void WM_drag_data_free(int dragtype, void *poin);
void WM_drag_free(struct wmDrag *drag);
void WM_drag_free_list(struct ListBase *lb);

void WM_drag_add_local_ID(struct wmDrag *drag, struct ID *id, struct ID *from_parent);

#ifdef __cplusplus
}
#endif /* __cplusplus */