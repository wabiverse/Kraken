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

#include "MEM_guardedalloc.h"

#include "USD_wm_types.h"
#include "USD_area.h"
#include "USD_context.h"
#include "USD_factory.h"
#include "USD_object.h"
#include "USD_operator.h"
#include "USD_pixar_utils.h"
#include "USD_region.h"
#include "USD_screen.h"
#include "USD_space_types.h"
#include "USD_userpref.h"
#include "USD_window.h"
#include "USD_workspace.h"

#include "KKE_main.h"
#include "KKE_global.h"
#include "KKE_screen.h"
#include "KKE_workspace.h"

#include "KLI_assert.h"
#include "KLI_listbase.h"
#include "KLI_string.h"
#include "KLI_string_utils.h"
#include "KLI_threads.h"


static WorkSpaceLayout *workspace_layout_find_exec(const WorkSpace *workspace,
                                                   const kScreen *screen)
{
  return static_cast<WorkSpaceLayout *>(
    KLI_findptr(&workspace->layouts, screen, offsetof(WorkSpaceLayout, screen)));
}


static void workspace_relation_add(ListBase *relation_list,
                                   void *parent,
                                   const int parentid,
                                   void *data)
{
  WorkSpaceDataRelation *relation = MEM_new<WorkSpaceDataRelation>(__func__);
  relation->parent = static_cast<WorkSpaceInstanceHook *>(parent);
  relation->parentid = parentid;
  relation->value = static_cast<WorkSpaceLayout *>(data);
  /* add to head, if we switch back to it soon we find it faster. */
  KLI_addhead(relation_list, relation);
}


static void *workspace_relation_get_data_matching_parent(const ListBase *relation_list,
                                                         const void *parent)
{
  WorkSpaceDataRelation *relation = static_cast<WorkSpaceDataRelation *>(
    KLI_findptr(relation_list, parent, offsetof(WorkSpaceDataRelation, parent)));
  if (relation != nullptr) {
    return relation->value;
  }

  return nullptr;
}


static void workspace_relation_ensure_updated(ListBase *relation_list,
                                              void *parent,
                                              const int parentid,
                                              void *data)
{
  WorkSpaceDataRelation *relation = static_cast<WorkSpaceDataRelation *>(
    KLI_listbase_bytes_find(relation_list,
                            &parentid,
                            sizeof(parentid),
                            offsetof(WorkSpaceDataRelation, parentid)));
  if (relation != nullptr) {
    relation->parent = static_cast<WorkSpaceInstanceHook *>(parent);
    relation->value = static_cast<WorkSpaceLayout *>(data);
    /* reinsert at the head of the list, so that more commonly used relations are found faster. */
    KLI_remlink(relation_list, relation);
    KLI_addhead(relation_list, relation);
  } else {
    /* no matching relation found, add new one */
    workspace_relation_add(relation_list, parent, parentid, data);
  }
}


/**
 * Checks if @a screen is already used within any workspace. A screen should never be assigned to
 * multiple WorkSpaceLayouts, but that should be ensured outside of the KKE_workspace module
 * and without such checks.
 * Hence, this should only be used as assert check before assigning a screen to a workspace.
 */
#ifndef NDEBUG
static bool workspaces_is_screen_used
#else
static bool UNUSED_FUNCTION(workspaces_is_screen_used)
#endif
  (const Main *kmain, kScreen *screen)
{
  for (WorkSpace *workspace = static_cast<WorkSpace *>(kmain->workspaces.first); workspace;
       workspace = static_cast<WorkSpace *>(workspace->id.next)) {
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
  layout->name = TfToken(new_name);
  KLI_uniquename(&workspace->layouts,
                 layout,
                 "Layout",
                 '.',
                 offsetof(WorkSpaceLayout, name),
                 sizeof(layout->name));

  FormFactory(workspace->name, layout->name);
}

WorkSpace *KKE_workspace_add(kContext *C, const char *name)
{
  SdfPath path(STRINGALL(KRAKEN_PATH_DEFAULTS::KRAKEN_WORKSPACES));
  WorkSpace *new_workspace = MEM_new<WorkSpace>(__func__, C, path.AppendPath(SdfPath(name)));

  Main *kmain = CTX_data_main(C);

  KLI_spin_lock((SpinLock *)kmain->lock);
  KLI_addtail(&kmain->workspaces, new_workspace);
  kmain->is_memfile_undo_written = false;
  KLI_spin_unlock((SpinLock *)kmain->lock);

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

  LISTBASE_FOREACH(WorkSpace *, workspace, &kmain->workspaces)
  {
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
  WorkSpaceLayout *layout = MEM_new<WorkSpaceLayout>(__func__);

  KLI_assert(!workspaces_is_screen_used(kmain, screen));
#ifndef DEBUG
  UNUSED_VARS(kmain);
#endif
  layout->screen = screen;
  layout->screen->winid = find_free_screenid(C);
  layout->screen->path = make_screenpath(name, layout->screen->winid);
  workspace_layout_name_set(workspace, layout, name);
  KLI_addtail(&workspace->layouts, layout);

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

  return nullptr;
}


kScreen *KKE_workspace_active_screen_get(const WorkSpaceInstanceHook *hook)
{
  return hook->act_layout->screen;
}


WorkSpaceLayout *KKE_workspace_active_layout_for_workspace_get(const WorkSpaceInstanceHook *hook,
                                                               const WorkSpace *workspace)
{
  /* If the workspace is active, the active layout can be returned, no need for a lookup. */
  if (hook->active == workspace) {
    return hook->act_layout;
  }

  /* Inactive workspace */
  return static_cast<WorkSpaceLayout *>(workspace_relation_get_data_matching_parent(&workspace->hook_layout_relations, hook));
}


void KKE_workspace_active_layout_set(WorkSpaceInstanceHook *hook,
                                     const int winid,
                                     WorkSpace *workspace,
                                     WorkSpaceLayout *layout)
{
  hook->act_layout = layout;
  workspace_relation_ensure_updated(&workspace->hook_layout_relations, hook, winid, layout);
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
  WorkSpaceInstanceHook *hook = MEM_new<WorkSpaceInstanceHook>(__func__);

  /* set an active screen-layout for each possible window/workspace combination */
  for (WorkSpace *workspace = static_cast<WorkSpace *>(kmain->workspaces.first); workspace;
       workspace = static_cast<WorkSpace *>(workspace->id.next)) {
    KKE_workspace_active_layout_set(hook,
                                    winid,
                                    workspace,
                                    static_cast<WorkSpaceLayout *>(workspace->layouts.first));
  }

  return hook;
}

void KKE_workspace_layout_remove(Main *kmain, WorkSpace *workspace, WorkSpaceLayout *layout)
{
  /* Screen should usually be set, but we call this from file reading to get rid of invalid
   * layouts. */
  if (layout->screen) {
    LISTBASE_FOREACH_MUTABLE(kScreen *, screen, &kmain->screens)
    {
      if (layout->screen == screen) {
        KLI_spin_lock((SpinLock *)kmain->lock);
        KLI_remlink(&kmain->screens, layout->screen);
        KLI_spin_unlock((SpinLock *)kmain->lock);
        break;
      }
    }
  }
  KLI_freelinkN(&workspace->layouts, layout);
}

static void workspace_relation_remove(ListBase *relation_list, WorkSpaceDataRelation *relation)
{
  KLI_remlink(relation_list, relation);
  MEM_delete(relation);
}

void KKE_workspace_instance_hook_free(const Main *kmain, WorkSpaceInstanceHook *hook)
{
  /* workspaces should never be freed before wm (during which we call this function).
   * However, when running in background mode, loading a blend file may allocate windows (that need
   * to be freed) without creating workspaces. This happens in BlendfileLoadingBaseTest. */
  KLI_assert(!KLI_listbase_is_empty(&kmain->workspaces) || G.background);

  /* Free relations for this hook */
  for (WorkSpace *workspace = static_cast<WorkSpace *>(kmain->workspaces.first); workspace;
       workspace = static_cast<WorkSpace *>(workspace->id.next)) {
    for (WorkSpaceDataRelation *
           relation = static_cast<WorkSpaceDataRelation *>(workspace->hook_layout_relations.first),
          *relation_next;
         relation;
         relation = relation_next) {
      relation_next = relation->next;
      if (relation->parent == hook) {
        workspace_relation_remove(&workspace->hook_layout_relations, relation);
      }
    }
  }

  MEM_delete(hook);
}


WorkSpace *KKE_workspace_active_get(WorkSpaceInstanceHook *hook)
{
  return hook->active;
}

void KKE_workspace_active_set(WorkSpaceInstanceHook *hook, WorkSpace *workspace)
{
  /* DO NOT check for `hook->active == workspace` here. Caller code is supposed to do it if
   * that optimization is possible and needed.
   * This code can be called from places where we might have this equality, but still want to
   * ensure/update the active layout below.
   * Known case where this is buggy and will crash later due to nullptr active layout: reading
   * a blend file, when the new read workspace ID happens to have the exact same memory address
   * as when it was saved in the blend file (extremely unlikely, but possible). */

  hook->active = workspace;
  if (workspace) {
    WorkSpaceLayout *layout = static_cast<WorkSpaceLayout *>(
      workspace_relation_get_data_matching_parent(&workspace->hook_layout_relations, hook));
    if (layout) {
      hook->act_layout = layout;
    }
  }
}

kScreen *KKE_workspace_layout_screen_get(const WorkSpaceLayout *layout)
{
  return layout->screen;
}

bool KKE_workspace_owner_id_check(const WorkSpace *workspace, const TfToken &owner_id)
{
  if ((owner_id == TfToken()) || ((workspace->flags & WORKSPACE_USE_FILTER_BY_ORIGIN) == 0)) {
    return true;
  }

  for (auto &wspaceid : workspace->owner_ids) {
    if (wspaceid == owner_id) {
      return true;
    }
  }

  return false;
}
