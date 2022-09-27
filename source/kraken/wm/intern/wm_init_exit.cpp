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
 * Window Manager.
 * Making GUI Fly.
 */

#include "KrakenOS/Kraken.OS.hpp"

#include "WM_init_exit.h" /* Own include. */
#include "WM_cursors.h"
#include "WM_event_system.h"
#include "WM_msgbus.h"
#include "WM_operators.h"
#include "WM_tokens.h"
#include "WM_window.h"
#include "WM_files.h"

#include "ANCHOR_api.h"
#include "ANCHOR_system_paths.h"

#include "USD_context.h"
#include "USD_userpref.h"
#include "USD_factory.h"

#include "KKE_appdir.h"
#include "KKE_context.h"
#include "KKE_icons.h"
#include "KKE_main.h"

#include "KPY_extern_python.h"
#include "KPY_extern_run.h"

#include "ED_debug_codes.h"
#include "ED_screen.h"

#include "UI_interface.h"
#include "UI_resources.h"

#include <wabi/base/tf/stringUtils.h>

KRAKEN_NAMESPACE_BEGIN


void WM_init(kContext *C, int argc, const char **argv)
{
  WM_anchor_init(C);
  WM_init_cursor_data();

  ANCHOR_CreateSystemPaths();

  WM_operators_init(C);
  WM_operators_register(C);

  WM_init_manager(C);
  WM_files_init(C);

  // ED_spacetypes_init();

  KKE_icons_init(KIFICONID_LAST);

  KPY_python_start(C, argc, argv);
  KPY_python_reset(C);

  ANCHOR::ToggleConsole(3);

  Main *kmain = CTX_data_main(C);

  wm_add_default(kmain, C);

  CTX_wm_window_set(C, CTX_wm_manager(C)->windows.begin()->second);

  CTX_wm_window_set(C, NULL);
}


void WM_exit_ex(kContext *C, const bool do_python)
{
  wmWindowManager *wm = C ? CTX_wm_manager(C) : NULL;

  if (C && wm) {
    WM_jobs_kill_all(wm);

    UNIVERSE_FOR_ALL (win, wm->windows) {
      CTX_wm_window_set(C, VALUE(win));
      WM_event_remove_handlers(C, VALUE(win)->modalhandlers);
      ED_screen_exit(C, VALUE(win), WM_window_get_active_screen(VALUE(win)));
    }

    UserDef *uprefs = CTX_data_prefs(C);
    KrakenSTAGE stage = CTX_data_stage(C);

    bool show_save = FormFactory(uprefs->showsave);

    if ((show_save) && ((G.f & G_FLAG_USERPREF_NO_SAVE_ON_EXIT) == 0)) {
      if (stage->GetRootLayer()->IsDirty()) {
        stage->GetRootLayer()->Save();
      }
    }
  }

#if defined(WITH_PYTHON) && !defined(WITH_PYTHON_MODULE)
  /* Without this, we there isn't a good way to manage false-positive resource leaks.
   *
   * This allows add-ons to free resources when unregistered (which is good practice anyway).
   *
   * Don't run this code when built as a Python module as this runs when Python is in the
   * process of shutting down, where running a snippet like this will crash. Instead use
   * the `atexit` module, installed by #KPY_python_start */
  const char *imports[] = {"addon_utils", NULL};
  KPY_run_string_eval(C, imports, "addon_utils.disable_all()");
#endif

  // KLI_timer_free();

  // WM_paneltype_clear();

  // KKE_addon_pref_type_free();
  // KKE_keyconfig_pref_type_free();
  // KKE_materials_exit();

  // wm_operatortype_free();
  // wm_surfaces_free();
  // wm_dropbox_free();
  // WM_menutype_free();
  // WM_uilisttype_free();

  /* all non-screen and non-space stuff editors did, like editmode */
  // if (C) {
  //   Main *kmain = CTX_data_main(C);
  //   ED_editors_exit(kmain, true);
  // }

  // ED_undosys_type_free();

  // free_openrecent();

  // KKE_mball_cubeTable_free();

  /* render code might still access databases */
  // RE_FreeAllRender();
  // RE_engines_exit();

  // ED_preview_free_dbase(); /* frees a Main dbase, before KKE_kraken_free! */

  // if (wm) {
  //   /* Before KKE_kraken_free!. */
  //   wm_free_reports(wm);
  // }

  // SEQ_clipboard_free(); /* sequencer.c */
  // KKE_tracking_clipboard_free();
  // KKE_mask_clipboard_free();
  // KKE_vfont_clipboard_free();
  // KKE_node_clipboard_free();

#ifdef WITH_COMPOSITOR
  // COM_deinitialize();
#endif

  // KKE_subdiv_exit();

  // if (gpu_is_init) {
  //   KKE_image_free_unused_gpu_textures();
  // }

  /* KKE_kraken.cpp, does entire library and spacetypes */
  KKE_kraken_free();
  // ANIM_fcurves_copybuf_free();
  // ANIM_drivers_copybuf_free();
  // ANIM_driver_vars_copybuf_free();
  // ANIM_fmodifiers_copybuf_free();
  // ED_gpencil_anim_copybuf_free();
  // ED_gpencil_strokes_copybuf_free();

  /* free gizmo-maps after freeing blender,
   * so no deleted data get accessed during cleaning up of areas. */
  // wm_gizmomaptypes_free();
  // wm_gizmogrouptype_free();
  // wm_gizmotype_free();

  // KLF_exit();

  // if (gpu_is_init) {
  // DRW_gpu_context_enable_ex(false);
  // GPU_pass_cache_free();
  // GPU_exit();
  // DRW_gpu_context_disable_ex(false);
  // DRW_gpu_context_destroy();
  // }

  // ANIM_keyingset_infos_exit();

  //  free_txt_data();

#ifdef WITH_PYTHON
  /* option not to close python so we can use 'atexit' */
  if (do_python && ((C == NULL) || CTX_py_init_get(C))) {
    /* NOTE: (old note)
     * before KKE_kraken_free so Python's garbage-collection happens while library still exists.
     * Needed at least for a rare crash that can happen in python-drivers.
     *
     * Update for Blender 2.5, move after #KKE_kraken_free because Blender now holds references
     * to #PyObject's so #Py_DECREF'ing them after Python ends causes bad problems every time
     * the python-driver bug can be fixed if it happens again we can deal with it then. */
    KPY_python_end();
  }
#else
  TF_UNUSED(do_python);
#endif

  // ED_file_exit();

  // UI_exit();
  // KKE_blender_userdef_data_free(&U, false);

  // USD_exit();

  // GPU_backend_exit();

  WM_anchor_exit();

  CTX_free(C);

  ANCHOR_DisposeSystemPaths();

  // KLI_task_scheduler_exit();

  // KKE_sound_exit();

  KKE_appdir_exit();

  KKE_kraken_atexit();

  // wm_autosave_delete();

  KKE_tempdir_session_purge();
}

#ifdef WIN32
/* Read console events until there is a key event.  Also returns on any error. */
static void wait_for_console_key(void)
{
  HANDLE hConsoleInput = GetStdHandle(STD_INPUT_HANDLE);

  if (((hConsoleInput != NULL) || (hConsoleInput != INVALID_HANDLE_VALUE)) &&
      FlushConsoleInputBuffer(hConsoleInput)) {
    for (;;) {
      INPUT_RECORD buffer;
      DWORD ignored;

      if (!ReadConsoleInput(hConsoleInput, &buffer, 1, &ignored)) {
        break;
      }

      if (buffer.EventType == KEY_EVENT) {
        break;
      }
    }
  }
}
#endif

void WM_exit(kContext *C)
{
  WM_exit_ex(C, true);

  printf("\nKraken quit\n");

#ifdef WIN32
  if (G.debug & G_DEBUG) {
    printf("Press any key to exit . . .\n\n");
    wait_for_console_key();
  }
#endif

  exit(G.is_break == true);
}

KRAKEN_NAMESPACE_END