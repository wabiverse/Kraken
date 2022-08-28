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

#pragma once

#include "KKE_api.h"
#include "KKE_context.h"

#include "USD_object.h"
#include "USD_screen.h"
#include "USD_workspace.h"

KRAKEN_NAMESPACE_BEGIN

/* clang-format off */
#define GETTER_ATTRS ATTR_NONNULL() ATTR_WARN_UNUSED_RESULT
#define SETTER_ATTRS ATTR_NONNULL(1)
/* clang-format on */

WorkSpace *KKE_workspace_add(kContext *C, const char *name);

void KKE_workspace_active_screen_set(WorkSpaceInstanceHook *hook,
                                     const int winid,
                                     WorkSpace *workspace,
                                     kScreen *screen);

void KKE_workspace_active_layout_set(WorkSpaceInstanceHook *hook,
                                     const int winid,
                                     WorkSpace *workspace,
                                     WorkSpaceLayout *layout);

kScreen *KKE_workspace_active_screen_get(const WorkSpaceInstanceHook *hook);
kScreen *KKE_workspace_layout_screen_get(const WorkSpaceLayout *layout) GETTER_ATTRS;

WorkSpaceLayout *KKE_workspace_layout_add(kContext *C,
                                          Main *kmain,
                                          WorkSpace *workspace,
                                          kScreen *screen,
                                          const char *name) ATTR_NONNULL();
WorkSpaceLayout *KKE_workspace_active_layout_get(const WorkSpaceInstanceHook *hook);
WorkSpaceLayout *KKE_workspace_layout_find(const WorkSpace *workspace, const kScreen *screen);

WorkSpaceLayout *KKE_workspace_layout_find_global(const Main *kmain,
                                                  const kScreen *screen,
                                                  WorkSpace **r_workspace);

void KKE_workspace_active_set(WorkSpaceInstanceHook *hook, WorkSpace *workspace);

WorkSpace *KKE_workspace_active_get(WorkSpaceInstanceHook *hook);
WorkSpaceInstanceHook *KKE_workspace_instance_hook_create(const Main *kmain, const int winid);


KRAKEN_NAMESPACE_END