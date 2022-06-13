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

#include "UNI_area.h"
#include "UNI_context.h"
#include "UNI_factory.h"
#include "UNI_object.h"
#include "UNI_operator.h"
#include "UNI_pixar_utils.h"
#include "UNI_region.h"
#include "UNI_screen.h"
#include "UNI_space_types.h"
#include "UNI_userpref.h"
#include "UNI_window.h"
#include "UNI_wm_types.h"
#include "UNI_workspace.h"

#include "KKE_screen.h"
#include "KKE_workspace.h"

#include "KLI_assert.h"
#include "KLI_string_utils.h"

WABI_NAMESPACE_BEGIN


static WorkSpaceLayout *workspace_layout_find_exec(const WorkSpace *workspace,
                                                   const kScreen *screen)
{
  UNIVERSE_FOR_ALL (layout, workspace->layouts) {
    if (layout->screen == screen) {
      return layout;
    }
  }

  return nullptr;
}


static void workspace_relation_add(WorkSpaceDataRelationVector relation_list,
                                   WorkSpaceInstanceHook *parent,
                                   const int parentid,
                                   WorkSpaceLayout *layout)
{
  WorkSpaceDataRelation *relation = new WorkSpaceDataRelation();
  relation->parent = parent;
  relation->parentid = parentid;
  relation->value = layout;
  /* add to head, if we switch back to it soon we find it faster. */
  relation_list.insert(relation_list.begin(), relation);
}


static WorkSpaceLayout *workspace_relation_get_data_matching_parent(
  const WorkSpaceDataRelationVector relation_list,
  const WorkSpaceInstanceHook *parent)
{
  UNIVERSE_FOR_ALL (relation, relation_list) {
    if (relation->parent == parent) {
      return relation->value;
    }
  }

  return nullptr;
}


static void workspace_relation_ensure_updated(WorkSpaceDataRelationVector relation_list,
                                              WorkSpaceInstanceHook *parent,
                                              const int parentid,
                                              WorkSpaceLayout *layout)
{
  if (relation_list.begin() != relation_list.end()) {

    auto relation = std::find_if(relation_list.begin(),
                                 relation_list.end(),
                                 [&](WorkSpaceDataRelation *r) -> bool {
                                   if (r->parentid == parentid) {
                                     r->parent = parent;
                                     r->value = layout;
                                     return true;
                                   }

                                   else {
                                     return false;
                                   }
                                 });

    /* reinsert at the head of the list, so that more commonly used relations are found faster. */
    if (relation != relation_list.end()) {
      std::rotate(relation_list.begin(), relation, relation + 1);
      return;
    }
  }
  /* no matching relation found, add new one */
  workspace_relation_add(relation_list, parent, parentid, layout);
}


static bool workspaces_is_screen_used(const Main *kmain, kScreen *screen)
{
  UNIVERSE_FOR_ALL (workspace, kmain->workspaces) {
    if (workspace_layout_find_exec(workspace, screen)) {
      return true;
    }
  }

  return false;
}

static void workspace_layout_name_set(WorkSpace *workspace,
                                      WorkSpaceLayout *layout,
                                      const char *new_name)
{
  TfToken new_token(new_name);
  layout->name.Swap(new_token);

  std::vector<void *> void_vec;
  UNIVERSE_FOR_ALL (alayout, workspace->layouts) {
    void_vec.push_back(alayout);
  }

  KLI_uniquename(void_vec,
                 layout,
                 "Layout",
                 '.',
                 offsetof(WorkSpaceLayout, name),
                 sizeof(CHARALL(layout->name)));

  FormFactory(workspace->name, layout->name);
}

WorkSpace *KKE_workspace_add(kContext *C, const char *name)
{
  SdfPath path(STRINGALL(KRAKEN_PATH_DEFAULTS::KRAKEN_WORKSPACES));
  WorkSpace *new_workspace = new WorkSpace(C, path.AppendPath(SdfPath(name)));
  FormFactory(new_workspace->name, TfToken(name));
  return new_workspace;
}

WorkSpaceLayout *KKE_workspace_layout_find_global(const Main *kmain,
                                                  const kScreen *screen,
                                                  WorkSpace **r_workspace)
{
  WorkSpaceLayout *layout;

  if (r_workspace) {
    *r_workspace = nullptr;
  }

  UNIVERSE_FOR_ALL (workspace, kmain->workspaces) {
    if ((layout = workspace_layout_find_exec(workspace, screen))) {
      if (r_workspace) {
        *r_workspace = workspace;
      }

      return layout;
    }
  }

  return nullptr;
}


WorkSpaceLayout *KKE_workspace_layout_add(kContext *C,
                                          Main *kmain,
                                          WorkSpace *workspace,
                                          kScreen *screen,
                                          const char *name)
{
  WorkSpaceLayout *layout = new WorkSpaceLayout();

  KLI_assert(!workspaces_is_screen_used(kmain, screen));
  layout->screen = screen;
  layout->screen->winid = find_free_screenid(C);
  layout->screen->path = make_screenpath(name, layout->screen->winid);
  workspace_layout_name_set(workspace, layout, name);
  workspace->layouts.push_back(layout);

  return layout;
}


WorkSpaceLayout *KKE_workspace_active_layout_get(const WorkSpaceInstanceHook *hook)
{
  return hook->act_layout;
}


WorkSpaceLayout *KKE_workspace_layout_find(const WorkSpace *workspace, const kScreen *screen)
{
  WorkSpaceLayout *layout = workspace_layout_find_exec(workspace, screen);
  if (layout) {
    return layout;
  }

  TfToken name = FormFactory(workspace->name);

  TF_CODING_ERROR(
    "%s: Couldn't find layout in this workspace: '%s' screen: '%s'. "
    "This should not happen!\n",
    CHARALL(name),
    CHARALL(screen->path.GetName()));

  return NULL;
}


kScreen *KKE_workspace_active_screen_get(const WorkSpaceInstanceHook *hook)
{
  return hook->act_layout->screen;
}


void KKE_workspace_active_layout_set(WorkSpaceInstanceHook *hook,
                                     const int winid,
                                     WorkSpace *workspace,
                                     WorkSpaceLayout *layout)
{
  hook->act_layout = layout;
  workspace_relation_ensure_updated(workspace->hook_layout_relations, hook, winid, layout);
}


void KKE_workspace_active_screen_set(WorkSpaceInstanceHook *hook,
                                     const int winid,
                                     WorkSpace *workspace,
                                     kScreen *screen)
{
  /* we need to find the WorkspaceLayout that wraps this screen */
  WorkSpaceLayout *layout = KKE_workspace_layout_find(hook->active, screen);
  KKE_workspace_active_layout_set(hook, winid, workspace, layout);
}


WorkSpaceInstanceHook *KKE_workspace_instance_hook_create(const Main *kmain, const int winid)
{
  WorkSpaceInstanceHook *hook = new WorkSpaceInstanceHook();

  /* set an active screen-layout for each possible window/workspace combination */
  UNIVERSE_FOR_ALL (workspace, kmain->workspaces) {
    UNIVERSE_FOR_ALL (layout, workspace->layouts) {
      KKE_workspace_active_layout_set(hook, winid, workspace, layout);
    }
  }

  return hook;
}


WorkSpace *KKE_workspace_active_get(WorkSpaceInstanceHook *hook)
{
  return hook->active;
}

void KKE_workspace_active_set(WorkSpaceInstanceHook *hook, WorkSpace *workspace)
{
  hook->active = workspace;
  if (workspace) {
    WorkSpaceLayout *layout = workspace_relation_get_data_matching_parent(
      workspace->hook_layout_relations,
      hook);
    if (layout) {
      hook->act_layout = layout;
    }
  }
}

kScreen *KKE_workspace_layout_screen_get(const WorkSpaceLayout *layout)
{
  return layout->screen;
}

WABI_NAMESPACE_END