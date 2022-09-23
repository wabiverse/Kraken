/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright 2008 Blender Foundation. All rights reserved. */

/** \file
 * \ingroup edutil
 */

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "KLI_string.h"

#include "KKE_main.h"
// #include "KKE_lib_id.h"
// #include "KKE_lib_remap.h"

// #include "ED_armature.h"
// #include "ED_asset.h"
// #include "ED_image.h"
// #include "ED_mesh.h"
// #include "ED_object.h"
// #include "ED_paint.h"
#include "ED_screen.h"
// #include "ED_space_api.h"
#include "ED_util.h"

#include "UI_interface.h"
#include "UI_resources.h"

#include "LUXO_access.h"

void ED_editors_init(kContext *C)
{
  Main *kmain = CTX_data_main(C);
  Scene *scene = CTX_data_scene(C);
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
  //   ED_space_image_paint_update(bmain, wm, scene);
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