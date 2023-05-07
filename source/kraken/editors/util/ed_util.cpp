/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright 2008 Blender Foundation. All rights reserved. */

/** \file
 * \ingroup edutil
 */

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "KKE_context.h"
#include "KKE_main.h"
#include "KKE_global.h"
// #include "KKE_lib_id.h"
// #include "KKE_lib_remap.h"

// #include "ED_armature.h"
// #include "ED_asset.h"
// #include "ED_image.h"
// #include "ED_mesh.h"
// #include "ED_object.h"
// #include "ED_paint.h"
#include "ED_screen.h"
#include "ED_space_api.h"
#include "ED_util.h"

#include "UI_interface.h"
#include "UI_resources.h"

#include "LUXO_access.h"

void ED_editors_init(kContext *C)
{
  Main *kmain = CTX_data_main(C);
  kScene *scene = CTX_data_scene(C);
  wmWindowManager *wm = CTX_wm_manager(C);

  ReportList *reports = CTX_wm_reports(C);
  int reports_flag_prev = reports->flag & ~RPT_STORE;

  SWAP(int, reports->flag, reports_flag_prev);

  /* Don't do undo pushes when calling an operator. */
  wm->op_undo_depth++;

  /* toggle on modes for objects that were saved with these enabled. for
   * e.g. linked objects we have to ensure that they are actually the
   * active object in this scene. */
  // Object *obact = CTX_data_active_object(C);

  /* image editor paint mode */
  // if (scene) {
  //   ED_space_image_paint_update(kmain, wm, scene);
  // }

  /* Enforce a full redraw for the first time areas/regions get drawn. Further region init/refresh
   * just triggers non-rebuild redraws (#RGN_DRAW_NO_REBUILD). Usually a full redraw would be
   * triggered by a `NC_WM | ND_FILEREAD` notifier, but if a startup script calls an operator that
   * redraws the window, notifiers are not handled before the operator runs. See T98461. */
  // for (auto &win, wm->windows) {
  //   const kScreen *screen = WM_window_get_active_screen(win);

  //   ED_screen_areas_iter (win, screen, area) {
  //     ED_area_tag_redraw(area);
  //   }
  // }

  // ED_assetlist_storage_tag_main_data_dirty();

  SWAP(int, reports->flag, reports_flag_prev);
  wm->op_undo_depth--;
}


void ED_editors_exit(Main *kmain, bool do_undo_system)
{
  if (!kmain) {
    return;
  }

  /* Frees all edit-mode undo-steps. */
  if (do_undo_system && G_MAIN->wm.first) {
    wmWindowManager *wm = static_cast<wmWindowManager *>(G_MAIN->wm.first);
    /* normally we don't check for NULL undo stack,
     * do here since it may run in different context. */
    // if (wm->undo_stack) {
    //   KKE_undosys_stack_destroy(wm->undo_stack);
    //   wm->undo_stack = NULL;
    // }
  }

  /* On undo, tag for update so the depsgraph doesn't use stale edit-mode data,
   * this is possible when mixing edit-mode and memory-file undo.
   *
   * By convention, objects are not left in edit-mode - so this isn't often problem in practice,
   * since exiting edit-mode will tag the objects too.
   *
   * However there is no guarantee the active object _never_ changes while in edit-mode.
   * Python for example can do this, some callers to #ED_object_base_activate
   * don't handle modes either (doing so isn't always practical).
   *
   * To reproduce the problem where stale data is used, see: T84920. */
  // for (Object *ob = kmain->objects.first; ob; ob = ob->id.next) {
  //   if (ED_object_editmode_free_ex(kmain, ob)) {
  //     if (do_undo_system == false) {
  //       DEG_id_tag_update(&ob->id, ID_RECALC_TRANSFORM | ID_RECALC_GEOMETRY);
  //     }
  //   }
  // }

  /* global in meshtools... */
  //ED_mesh_mirror_spatial_table_end(NULL);
  //ED_mesh_mirror_topo_table_end(NULL);
}