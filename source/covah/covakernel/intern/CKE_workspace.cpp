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

#include "CKE_workspace.h"

#include "UNI_factory.h"

WABI_NAMESPACE_BEGIN


static WorkSpaceLayout *workspace_layout_find_exec(const WorkSpace *workspace,
                                                   const cScreen *screen)
{
  UNIVERSE_FOR_ALL(layout, workspace->layouts)
  {
    if (layout->screen == screen)
    {
      return layout;
    }
  }
}


static void workspace_relation_add(WorkSpaceDataRelationVector relation_list,
                                   WorkSpaceInstanceHook *parent,
                                   const SdfPath &parentid,
                                   WorkSpaceLayout *layout)
{
  WorkSpaceDataRelation *relation = new WorkSpaceDataRelation();
  relation->parent = parent;
  relation->parentid = parentid;
  relation->value = layout;
  /* add to head, if we switch back to it soon we find it faster. */
  relation_list.insert(relation_list.begin(), relation);
}


static void workspace_relation_ensure_updated(WorkSpaceDataRelationVector relation_list,
                                              WorkSpaceInstanceHook *parent,
                                              const SdfPath &parentid,
                                              WorkSpaceLayout *layout)
{
  auto relation = std::find_if(relation_list.begin(), relation_list.end(), [&](WorkSpaceDataRelation &r) -> bool {
    if (r.parentid == parentid)
    {
      r.parent = parent;
      r.value = layout;
      return true;
    }

    else
    {
      return false;
    }
  });

  /* reinsert at the head of the list, so that more commonly used relations are found faster. */
  if (relation != relation_list.end())
  {
    std::rotate(relation_list.begin(), relation, relation + 1);
  }
  else
  {
    /* no matching relation found, add new one */
    workspace_relation_add(relation_list, parent, parentid, layout);
  }
}


WorkSpaceLayout *CKE_workspace_layout_find(const WorkSpace *workspace, const cScreen *screen)
{
  WorkSpaceLayout *layout = workspace_layout_find_exec(workspace, screen);
  if (layout)
  {
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


cScreen *CKE_workspace_active_screen_get(const WorkSpaceInstanceHook *hook)
{
  return hook->act_layout->screen;
}


void CKE_workspace_active_layout_set(WorkSpaceInstanceHook *hook,
                                     const SdfPath &winid,
                                     WorkSpace *workspace,
                                     WorkSpaceLayout *layout)
{
  hook->act_layout = layout;
  workspace_relation_ensure_updated(workspace->hook_layout_relations, hook, winid, layout);
}


void CKE_workspace_active_screen_set(WorkSpaceInstanceHook *hook,
                                     const SdfPath &winid,
                                     WorkSpace *workspace,
                                     cScreen *screen)
{
  /* we need to find the WorkspaceLayout that wraps this screen */
  WorkSpaceLayout *layout = CKE_workspace_layout_find(hook->active, screen);
  CKE_workspace_active_layout_set(hook, winid, workspace, layout);
}


WorkSpace *CKE_workspace_active_get(WorkSpaceInstanceHook *hook)
{
  return hook->active;
}

WABI_NAMESPACE_END