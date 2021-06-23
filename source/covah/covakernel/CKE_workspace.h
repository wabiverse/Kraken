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
 * COVAH Kernel.
 * Purple Underground.
 */

#pragma once

#include "CKE_api.h"
#include "CKE_context.h"

#include "UNI_object.h"
#include "UNI_screen.h"
#include "UNI_workspace.h"

WABI_NAMESPACE_BEGIN

WorkSpace *CKE_workspace_active_get(WorkSpaceInstanceHook *hook);

void CKE_workspace_active_screen_set(WorkSpaceInstanceHook *hook,
                                     const SdfPath &winid,
                                     WorkSpace *workspace,
                                     cScreen *screen);

void CKE_workspace_active_layout_set(WorkSpaceInstanceHook *hook,
                                     const SdfPath &winid,
                                     WorkSpace *workspace,
                                     WorkSpaceLayout *layout);

cScreen *CKE_workspace_active_screen_get(const WorkSpaceInstanceHook *hook);

WorkSpaceLayout *CKE_workspace_layout_find(const WorkSpace *workspace, const cScreen *screen);

WABI_NAMESPACE_END