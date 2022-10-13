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

#include "MEM_guardedalloc.h"

#include "CLG_log.h"

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
#include "USD_wm_types.h"

#include "KLI_assert.h"
#include "KLI_listbase.h"
#include "KLI_path_utils.h"
#include "KLI_task.h"
#include "KLI_threads.h"
#include "KLI_utildefines.h"

#include "KKE_appdir.hh"
#include "KKE_appdir.h"
#include "KKE_context.h"
#include "KKE_icons.h"
#include "KKE_main.h"
#include "KKE_report.h"
#include "KKE_global.h"

#include "KPY_extern_python.h"
#include "KPY_extern_run.h"

#include "ED_debug_codes.h"
#include "ED_screen.h"

#include "GPU_capabilities.h"
#include "GPU_context.h"
#include "GPU_init_exit.h"
#include "GPU_material.h"

#include "KRF_api.h"

#include "UI_interface.h"
#include "UI_resources.h"

#include <wabi/base/tf/stringUtils.h>

CLG_LOGREF_DECLARE_GLOBAL(WM_LOG_OPERATORS, "wm.operator");
CLG_LOGREF_DECLARE_GLOBAL(WM_LOG_HANDLERS, "wm.handler");
CLG_LOGREF_DECLARE_GLOBAL(WM_LOG_EVENTS, "wm.event");
CLG_LOGREF_DECLARE_GLOBAL(WM_LOG_KEYMAPS, "wm.keymap");
CLG_LOGREF_DECLARE_GLOBAL(WM_LOG_TOOLS, "wm.tool");
CLG_LOGREF_DECLARE_GLOBAL(WM_LOG_MSGBUS_PUB, "wm.msgbus.pub");
CLG_LOGREF_DECLARE_GLOBAL(WM_LOG_MSGBUS_SUB, "wm.msgbus.sub");

static void wm_init_reports(kContext *C)
{
  ReportList *reports = CTX_wm_reports(C);

  KLI_assert(!reports || KLI_listbase_is_empty(&reports->list));

  KKE_reports_init(reports, RPT_STORE);
}

static void wm_free_reports(wmWindowManager *wm)
{
  KKE_reports_clear(&wm->reports);
}

static bool wm_start_with_console = false;

void WM_init_state_start_with_console_set(bool value)
{
  wm_start_with_console = value;
}

/**
 * Since we cannot know in advance if we will require the draw manager
 * context when starting kraken in background mode (specially true with
 * scripts) we defer the anchor initialization the most as possible
 * so that it does not break anything that can run in headless mode (as in
 * without display server attached).
 */
static bool opengl_is_init = false;

void WM_init_opengl(void)
{
  /* Must be called only once. */
  KLI_assert(opengl_is_init == false);

  if (G.background) {
    /* Anchor is still not initialized elsewhere in background mode. */
    // wm_anchor_init_background();
  }

  if (!GPU_backend_supported()) {
    return;
  }

  /* Needs to be first to have an OpenGL context bound. */
  // DRW_opengl_context_create();

  GPU_init();

  GPU_pass_cache_init();

  opengl_is_init = true;
}

void WM_init(kContext *C, int argc, const char **argv)
{
  if (!G.background) {
    WM_anchor_init(C);
    WM_init_cursor_data();
  }

  ANCHOR_CreateSystemPaths();

  WM_operators_init(C);
  WM_operators_register(C);

  WM_init_manager(C);
  WM_files_init(C);

  // ED_spacetypes_init();

  KRF_init();

  KKE_icons_init(KIFICONID_LAST);

  wm_init_reports(C);

  WM_msgbus_types_init();

  KLI_assert((G.fileflags & G_FILE_NO_UI) == 0);

  /* NOTE: leave `G.main->filepath` set to an empty string since this
   * matches behavior after loading a new file. */
  KLI_assert(G.main->stage_id == '\0');

  if (!G.background) {
    GPU_render_begin();

    WM_init_opengl();

    GPU_context_begin_frame(GPU_context_active_get());
    UI_init();
    GPU_context_end_frame(GPU_context_active_get());
    GPU_render_end();
  }

#ifdef WITH_PYTHON
  KPY_python_start(C, argc, argv);
  KPY_python_reset(C);
#else
  UNUSED_VARS(argc, argv);
#endif /* WITH_PYTHON */

  if (!G.background) {
    if (wm_start_with_console) {
      ANCHOR::ToggleConsole(ANCHOR_ConsoleWindowStateShow);
    } else {
      ANCHOR::ToggleConsole(ANCHOR_ConsoleWindowStateHideForNonConsoleLaunch);
    }
  }

  KLI_strncpy(G.lib, KKE_main_usdfile_path(G_MAIN), sizeof(G.lib));

  wm_add_default(G.main, C);

  CTX_wm_window_set(C, VALUE_PTR(CTX_wm_manager(C)->windows.begin()));

  CTX_wm_window_set(C, NULL);
}


void WM_exit_ex(kContext *C, const bool do_python)
{
  KrakenSTAGE stage = CTX_data_stage(C);
  wmWindowManager *wm = C ? CTX_wm_manager(C) : nullptr;
  kUserDef *uprefs = CTX_data_prefs(C);
  bool show_save = FormFactory(uprefs->showsave);
  bool has_edited = stage->GetRootLayer()->IsDirty();

  if (C && wm) {
    if (!G.background) {
      Main *kmain = CTX_data_main(C);
      char filepath[FILE_MAX];
      const int fileflags = G.fileflags & ~G_FILE_COMPRESS;

      KLI_join_dirfile(filepath, sizeof(filepath), KKE_tempdir_base(), KRAKEN_QUIT_FILE);

      if (has_edited) {
        if (stage->GetRootLayer()->Export(filepath,
                                          TfStringPrintf("Kraken v%d.%d Session Recovery File",
                                                         KRAKEN_VERSION_MAJOR,
                                                         KRAKEN_VERSION_MINOR))) {
          printf("Saved session recovery to '%s'\n", filepath);
        }
      }
    }

    WM_jobs_kill_all(wm);

    for (auto &win : wm->windows) {
      CTX_wm_window_set(C, VALUE(win));
      WM_event_remove_handlers(C, &VALUE(win)->handlers);
      WM_event_remove_handlers(C, &VALUE(win)->modalhandlers);
      ED_screen_exit(C, VALUE(win), WM_window_get_active_screen(VALUE(win)));
    }

    if (!G.background) {
      if ((show_save) && ((G.f & G_FLAG_USERPREF_NO_SAVE_ON_EXIT) == 0)) {
        if (U.runtime.is_dirty) {
          /** @TODO: update USD userpref with global U.xxx */
        }
      }
    }
  }

#if defined(WITH_PYTHON) && !defined(WITH_PYTHON_MODULE)
    /* Without this, we there isn't a good way to manage false-positive resource leaks
     * where a #PyObject references memory allocated with guarded-alloc, T71362.
     *
     * This allows add-ons to free resources when unregistered (which is good practice anyway).
     *
     * Don't run this code when built as a Python module as this runs when Python is in the
     * process of shutting down, where running a snippet like this will crash, see T82675.
     * Instead use the `atexit` module, installed by #BPY_python_start */
    KPY_run_string_eval(C, (const char *[]){"addon_utils", NULL}, "addon_utils.disable_all()");
#endif

    // KLI_timer_free();

    // WM_paneltype_clear();

    // KKE_addon_pref_type_free();
    // KKE_keyconfig_pref_type_free();
    // KKE_materials_exit();

    WM_operators_free();
    // wm_surfaces_free();
    // wm_dropbox_free();
    // WM_menutype_free();

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

    if (wm) {
      /* Before KKE_kraken_free!. */
      wm_free_reports(wm);
    }

    // SEQ_clipboard_free(); /* sequencer.c */
    // KKE_tracking_clipboard_free();
    // KKE_mask_clipboard_free();
    // KKE_vfont_clipboard_free();
    // KKE_node_clipboard_free();

#ifdef WITH_COMPOSITOR
    // COM_deinitialize();
#endif

    // KKE_subdiv_exit();

    // if (opengl_is_init) {
    // KKE_image_free_unused_gpu_textures();
    // }

    /* KKE_kraken.cpp, does entire library and spacetypes */
    KKE_kraken_free();

    /* Free the GPU subdivision data after the database to ensure that subdivision structs used by
     * the modifiers were garbage collected. */
    if (opengl_is_init) {
      // DRW_subdiv_free();
    }

    // ANIM_fcurves_copybuf_free();
    // ANIM_drivers_copybuf_free();
    // ANIM_driver_vars_copybuf_free();
    // ANIM_fmodifiers_copybuf_free();
    // ED_gpencil_anim_copybuf_free();
    // ED_gpencil_strokes_copybuf_free();

    /**
     * free gizmo-maps after freeing blender,
     * so no deleted data get accessed during cleaning up of areas. */
    // wm_gizmomaptypes_free();
    // wm_gizmogrouptype_free();
    // wm_gizmotype_free();
    /* Same for UI-list types. */
    // WM_uilisttype_free();

    KRF_exit();

    // ANIM_keyingset_infos_exit();

#ifdef WITH_PYTHON
    /* option not to close python so we can use 'atexit' */
    if (do_python && ((C == NULL) || CTX_py_init_get(C))) {
      /* NOTE: (old note)
       * before KKE_kraken_free so Python's garbage-collection happens while library still
       * exists. Needed at least for a rare crash that can happen in python-drivers.
       *
       * Update for Blender 2.5, move after #KKE_kraken_free because Blender now holds references
       * to #PyObject's so #Py_DECREF'ing them after Python ends causes bad problems every time
       * the python-driver bug can be fixed if it happens again we can deal with it then. */
      KPY_python_end();
    }
#else
    (void)do_python;
#endif

    // ED_file_exit();

    /* Delete GPU resources and context. The UI also uses GPU resources and so
     * is also deleted with the context active. */
    if (opengl_is_init) {
      // DRW_opengl_context_enable_ex(false);
      UI_exit();
      GPU_pass_cache_free();
      GPU_exit();
      // DRW_opengl_context_disable_ex(false);
      // DRW_opengl_context_destroy();
    } else {
      UI_exit();
    }

    // KKE_kraken_userpref_data_free(&U, false);

    // LUXO_exit();

    WM_anchor_exit();

    CTX_free(C);

    ANCHOR_DisposeSystemPaths();

    KLI_threadapi_exit();
    KLI_task_scheduler_exit();

    // KKE_sound_exit();

    KKE_appdir_exit();

    KKE_kraken_atexit();

    // wm_autosave_delete();

    KKE_tempdir_session_purge();

    CLG_exit();
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

  void WM_exit(kContext * C)
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
