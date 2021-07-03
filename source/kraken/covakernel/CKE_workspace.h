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
 * KRAKEN Kernel.
 * Purple Underground.
 */

#pragma once

#include "CKE_api.h"
#include "CKE_context.h"

#include "UNI_object.h"
#include "UNI_screen.h"
#include "UNI_workspace.h"

WABI_NAMESPACE_BEGIN

/* clang-format off */
#define GETTER_ATTRS ATTR_NONNULL() ATTR_WARN_UNUSED_RESULT
#define SETTER_ATTRS ATTR_NONNULL(1)
/* clang-format on */

WorkSpace *CKE_workspace_add(cContext *C, const char *name);

void CKE_workspace_active_screen_set(WorkSpaceInstanceHook *hook,
                                     const int winid,
                                     WorkSpace *workspace,
                                     cScreen *screen);

void CKE_workspace_active_layout_set(WorkSpaceInstanceHook *hook,
                                     const int winid,
                                     WorkSpace *workspace,
                                     WorkSpaceLayout *layout);

cScreen *CKE_workspace_active_screen_get(const WorkSpaceInstanceHook *hook);
cScreen *CKE_workspace_layout_screen_get(const WorkSpaceLayout *layout) GETTER_ATTRS;

WorkSpaceLayout *CKE_workspace_layout_add(Main *cmain,
                                          WorkSpace *workspace,
                                          cScreen *screen,
                                          const char *name) ATTR_NONNULL();
WorkSpaceLayout *CKE_workspace_active_layout_get(const WorkSpaceInstanceHook *hook);
WorkSpaceLayout *CKE_workspace_layout_find(const WorkSpace *workspace, const cScreen *screen);

WorkSpaceLayout *CKE_workspace_layout_find_global(const Main *cmain,
                                                  const cScreen *screen,
                                                  WorkSpace **r_workspace);

void CKE_workspace_active_set(WorkSpaceInstanceHook *hook, WorkSpace *workspace);

WorkSpace *CKE_workspace_active_get(WorkSpaceInstanceHook *hook);
WorkSpaceInstanceHook *CKE_workspace_instance_hook_create(const Main *cmain, const int winid);


WABI_NAMESPACE_END