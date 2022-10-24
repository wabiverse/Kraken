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

#include "USD_object.h"
#include "USD_factory.h"
#include "USD_screen.h"
#include "USD_userpref.h"
#include "USD_window.h"
#include "USD_wm_types.h"

#include "LUXO_access.h"
#include "LUXO_define.h"

#include "KLI_path_utils.h"

#include "KKE_autoexec.h"
#include "KKE_context.h"
#include "KKE_utils.h"
#include "KKE_appdir.h"
#include "KKE_appdir.hh"
#include "KKE_report.h"
#include "KKE_global.h"

#include "ED_fileselect.h"
#include "ED_screen.h"
#include "ED_util.h"

#include "UI_interface.h"
#include "UI_resources.h"
#include "UI_tokens.h"
#include "UI_view2d.h"

#include "DRW_engine.h"

#ifdef WITH_PYTHON
#  include "KPY_extern_python.h"
#  include "KPY_extern_run.h"
#endif

#include "WM_operators.h"
#include "WM_debug_codes.h"
#include "WM_msgbus.h"
#include "WM_tokens.h"
#include "WM_files.h"
#include "WM_event_system.h"
#include "WM_window.hh"

#include <filesystem>

#include "CLG_log.h"

static CLG_LogRef LOG = {"wm.files"};

namespace fs = std::filesystem;


static int wm_user_datafiles_write_exec(kContext *C, wmOperator *op)
{
  Main *kmain = CTX_data_main(C);
  wmWindowManager *wm = CTX_wm_manager(C);

  const std::string appdir = KKE_appdir_copy_recursive(KRAKEN_SYSTEM_DATAFILES,
                                                       KRAKEN_USER_DATAFILES);
  if (appdir.empty()) {
    KKE_report(op->reports, RPT_ERROR, "Unable to create user datafiles path");
    return OPERATOR_CANCELLED;
  }

  TF_STATUS("Writing user datafiles: '%s' ", appdir.c_str());

  TF_MSG_SUCCESS("ok");
  KKE_report(op->reports, RPT_INFO, "User datafiles created");

  return OPERATOR_FINISHED;
}

void WM_OT_files_create_appdata(wmOperatorType *ot)
{
  ot->name = "Create User AppData Directory";
  ot->idname = WM_ID_(WM_OT_files_create_appdata);
  ot->description = "Install user directories at OS appdata";

  ot->exec = wm_user_datafiles_write_exec;

  ot->prim = PRIM_def_struct_ptr(KRAKEN_STAGE, SdfPath(ot->idname), ot->prim);
}

static int wm_open_mainfile_exec(kContext *C, wmOperator *op)
{
  return OPERATOR_FINISHED;
}

static int wm_open_mainfile_invoke(kContext *C, wmOperator *op, wmEvent *UNUSED(event))
{
  return OPERATOR_FINISHED;
}

void WM_OT_open_mainfile(wmOperatorType *ot)
{
  ot->name = "Open";
  ot->idname = WM_ID_(WM_OT_open_mainfile);
  ot->description = "Open a Kraken file";

  ot->invoke = wm_open_mainfile_invoke;
  ot->exec = wm_open_mainfile_exec;

  ot->prim = PRIM_def_struct_ptr(KRAKEN_STAGE, SdfPath(ot->idname), ot->prim);

  PrimFactory::BOOL::Def(ot->prim,
                         "load_ui",
                         true,
                         "Load UI",
                         "Load user interface setup in the .usd file");
  PrimFactory::BOOL::Def(ot->prim,
                         "display_file_selector",
                         true,
                         "Display File Selector",
                         "Show the UI file selector indicator");
  PrimFactory::ASSET::Def(ot->prim,
                          "filepath",
                          G.main->filepath,
                          "File Path",
                          "The path to a .usd file to open");
}

static void wm_block_file_close_cancel(kContext *C, void *arg_block, void *UNUSED(arg_data))
{
  wmWindow *win = CTX_wm_window(C);
  // UI_popup_block_close(C, win, (uiBlock *)arg_block);
}

static void wm_block_file_close_discard(kContext *C, void *arg_block, void *arg_data)
{
  wmGenericCallback *callback = WM_generic_callback_steal((wmGenericCallback *)arg_data);

  /* Close the popup before executing the callback. Otherwise
   * the popup might be closed by the callback, which will lead
   * to a crash. */
  wmWindow *win = CTX_wm_window(C);
  // UI_popup_block_close(C, win, (uiBlock *)arg_block);

  callback->exec(C, callback->user_data);
  WM_generic_callback_free(callback);
}

static void wm_block_file_close_save(kContext *C, void *arg_block, void *arg_data)
{
  const Main *kmain = CTX_data_main(C);
  wmGenericCallback *callback = WM_generic_callback_steal((wmGenericCallback *)arg_data);
  bool execute_callback = true;

  wmWindow *win = CTX_wm_window(C);
  // UI_popup_block_close(C, win, (uiBlock *)arg_block);

  // int modified_images_count = ED_image_save_all_modified_info(CTX_data_main(C), NULL);
  // if (modified_images_count > 0 && save_images_when_file_is_closed) {
  //   if (ED_image_should_save_modified(kmain)) {
  //     ReportList *reports = CTX_wm_reports(C);
  //     ED_image_save_all_modified(C, reports);
  //     WM_report_banner_show();
  //   }
  //   else {
  //     execute_callback = false;
  //   }
  // }

  bool file_has_been_saved_before = !std::string(KKE_main_usdfile_path(kmain)).empty();

  if (file_has_been_saved_before) {
    if (WM_operator_name_call(C, WM_ID_(WM_OT_save_mainfile), WM_OP_EXEC_DEFAULT, NULL) &
        OPERATOR_CANCELLED) {
      execute_callback = false;
    }
  } else {
    WM_operator_name_call(C, WM_ID_(WM_OT_save_mainfile), WM_OP_INVOKE_DEFAULT, NULL);
    execute_callback = false;
  }

  if (execute_callback) {
    callback->exec(C, callback->user_data);
  }
  WM_generic_callback_free(callback);
}

static void wm_block_file_close_save_button(uiBlock *block, wmGenericCallback *post_action)
{
  uiBut *but = uiDefIconTextBut(block,
                                UI_BTYPE_BUT,
                                0,
                                0,
                                IFACE_("Save"),
                                0,
                                0,
                                0,
                                UI_UNIT_Y,
                                0,
                                0,
                                0,
                                0,
                                0,
                                "");
  UI_but_func_set(but, wm_block_file_close_save, block, post_action);
  UI_but_drawflag_disable(but, UI_BUT_TEXT_LEFT);
  UI_but_flag_enable(but, UI_BUT_ACTIVE_DEFAULT);
}

static void wm_block_file_close_cancel_button(uiBlock *block, wmGenericCallback *post_action)
{
  uiBut *but = uiDefIconTextBut(block,
                                UI_BTYPE_BUT,
                                0,
                                0,
                                IFACE_("Cancel"),
                                0,
                                0,
                                0,
                                UI_UNIT_Y,
                                0,
                                0,
                                0,
                                0,
                                0,
                                "");
  UI_but_func_set(but, wm_block_file_close_cancel, block, post_action);
  UI_but_drawflag_disable(but, UI_BUT_TEXT_LEFT);
}

static void wm_block_file_close_discard_button(uiBlock *block, wmGenericCallback *post_action)
{
  uiBut *but = uiDefIconTextBut(block,
                                UI_BTYPE_BUT,
                                0,
                                0,
                                IFACE_("Don't Save"),
                                0,
                                0,
                                0,
                                UI_UNIT_Y,
                                0,
                                0,
                                0,
                                0,
                                0,
                                "");
  UI_but_func_set(but, wm_block_file_close_discard, block, post_action);
  UI_but_drawflag_disable(but, UI_BUT_TEXT_LEFT);
}

static uiBlock *block_create__close_file_dialog(struct kContext *C,
                                                struct ARegion *region,
                                                void *arg1)
{
  wmGenericCallback *post_action = (wmGenericCallback *)arg1;
  Main *kmain = CTX_data_main(C);

  uiBlock *block = UI_block_begin(C, region, UI_ID(UI_POPUP_file_close), UI_EMBOSS);
  UI_block_flag_enable(block,
                       UI_BLOCK_KEEP_OPEN | UI_BLOCK_LOOP | UI_BLOCK_NO_WIN_CLIP |
                         UI_BLOCK_NUMSELECT);
  UI_block_theme_style_set(block, UI_BLOCK_THEME_STYLE_POPUP);

  uiLayout *layout = uiItemsAlertBox(block, 34, ALERT_ICON_QUESTION);

  /* Title. */
  uiItemL_ex(layout, TIP_("Save changes before closing?"), ICON_NONE, true, false);

  /* Filename. */
  const std::filesystem::path usdfile_path = KKE_main_usdfile_path(CTX_data_main(C));
  std::string filename;
  if (!usdfile_path.empty()) {
    filename = usdfile_path.filename();
  } else {
    filename = "untitled.usda";
  }
  uiItemL(layout, filename.c_str(), ICON_NONE);

  /* Image Saving Warnings. */
  ReportList reports;
  KKE_reports_init(&reports, RPT_STORE);
  // uint modified_images_count = ED_image_save_all_modified_info(kmain, &reports);

  LISTBASE_FOREACH(const Report *, report, &reports.list)
  {
    uiLayout *row = uiLayoutColumn(layout, false);
    uiLayoutSetScaleY(row, 0.6f);
    uiItemS(row);

    /* Error messages created in ED_image_save_all_modified_info() can be long,
     * but are made to separate into two parts at first colon between text and paths.
     */
    char *message = KLI_strdupn(report->message, report->len);
    char *path_info = strstr(message, ": ");
    if (path_info) {
      /* Terminate message string at colon. */
      path_info[1] = '\0';
      /* Skip over the ": " */
      path_info += 2;
    }
    uiItemL_ex(row, message, ICON_NONE, false, true);
    if (path_info) {
      uiItemL_ex(row, path_info, ICON_NONE, false, true);
    }
    delete message;
  }

  /* Used to determine if extra separators are needed. */
  bool has_extra_checkboxes = false;

  /* Modified Images Checkbox. */
  // if (modified_images_count > 0) {
  //   char message[64];
  //   KLI_snprintf(message, sizeof(message), "Save %u modified image(s)", modified_images_count);
  //   /* Only the first checkbox should get extra separation. */
  //   if (!has_extra_checkboxes) {
  //     uiItemS(layout);
  //   }
  //   uiDefButBitC(block,
  //                UI_BTYPE_CHECKBOX,
  //                1,
  //                0,
  //                message,
  //                0,
  //                0,
  //                0,
  //                UI_UNIT_Y,
  //                &save_images_when_file_is_closed,
  //                0,
  //                0,
  //                0,
  //                0,
  //                "");
  //   has_extra_checkboxes = true;
  // }

  // if (KKE_asset_library_has_any_unsaved_catalogs()) {
  //   static char save_catalogs_when_file_is_closed;

  //   save_catalogs_when_file_is_closed =
  //   ED_asset_catalogs_get_save_catalogs_when_file_is_saved();

  //   /* Only the first checkbox should get extra separation. */
  //   if (!has_extra_checkboxes) {
  //     uiItemS(layout);
  //   }
  //   uiBut *but = uiDefButBitC(block,
  //                             UI_BTYPE_CHECKBOX,
  //                             1,
  //                             0,
  //                             "Save modified asset catalogs",
  //                             0,
  //                             0,
  //                             0,
  //                             UI_UNIT_Y,
  //                             &save_catalogs_when_file_is_closed,
  //                             0,
  //                             0,
  //                             0,
  //                             0,
  //                             "");
  //   UI_but_func_set(
  //       but, save_catalogs_when_file_is_closed_set_fn, &save_catalogs_when_file_is_closed,
  //       NULL);
  //   has_extra_checkboxes = true;
  // }

  KKE_reports_clear(&reports);

  uiItemS_ex(layout, has_extra_checkboxes ? 2.0f : 4.0f);

  /* Buttons. */
#ifdef _WIN32
  const bool windows_layout = true;
#else
  const bool windows_layout = false;
#endif

  if (windows_layout) {
    /* Windows standard layout. */

    uiLayout *split = uiLayoutSplit(layout, 0.0f, true);
    uiLayoutSetScaleY(split, 1.2f);

    uiLayoutColumn(split, false);
    wm_block_file_close_save_button(block, post_action);

    uiLayoutColumn(split, false);
    wm_block_file_close_discard_button(block, post_action);

    uiLayoutColumn(split, false);
    wm_block_file_close_cancel_button(block, post_action);
  } else {
    /* Non-Windows layout (macOS and Linux). */

    uiLayout *split = uiLayoutSplit(layout, 0.3f, true);
    uiLayoutSetScaleY(split, 1.2f);

    uiLayoutColumn(split, false);
    wm_block_file_close_discard_button(block, post_action);

    uiLayout *split_right = uiLayoutSplit(split, 0.1f, true);

    uiLayoutColumn(split_right, false);
    /* Empty space. */

    uiLayoutColumn(split_right, false);
    wm_block_file_close_cancel_button(block, post_action);

    uiLayoutColumn(split_right, false);
    wm_block_file_close_save_button(block, post_action);
  }

  UI_block_bounds_set_centered(block, 14 * U.dpi_fac);
  return block;
}

static void free_post_file_close_action(void *arg)
{
  wmGenericCallback *action = (wmGenericCallback *)arg;
  WM_generic_callback_free(action);
}

void WM_close_file_dialog(kContext *C, wmGenericCallback *post_action)
{
  if (!UI_popup_block_name_exists(CTX_wm_screen(C), UI_ID(UI_POPUP_file_close))) {
    UI_popup_block_invoke(C,
                          block_create__close_file_dialog,
                          post_action,
                          free_post_file_close_action);
  } else {
    WM_generic_callback_free(post_action);
  }
}

void WM_file_operators_register(void)
{
  /* ------ */

  WM_operatortype_append(WM_OT_files_create_appdata);
  WM_operatortype_append(WM_OT_open_mainfile);

  /* ------ */
}

void WM_files_init(kContext *C)
{
  KrakenPRIM props_ptr;

  wmOperatorType *ot = WM_operatortype_find(WM_ID_(WM_OT_files_create_appdata));

  WM_operator_properties_create_ptr(&props_ptr, ot);

  WM_operator_name_call_ptr(C, ot, WM_OP_INVOKE_DEFAULT, &props_ptr);
  WM_operator_properties_free(&props_ptr);
}

static struct
{
  char app_template[64];
  bool override;
} wm_init_state_app_template = {{0}};

void WM_init_state_app_template_set(const char *app_template)
{
  if (app_template) {
    STRNCPY(wm_init_state_app_template.app_template, app_template);
    wm_init_state_app_template.override = true;
  } else {
    wm_init_state_app_template.app_template[0] = '\0';
    wm_init_state_app_template.override = false;
  }
}


/**
 * Store the action needed if the user needs to reload the file with Python scripts enabled.
 *
 * When left to NULL, this is simply revert.
 * When loading files through the recover auto-save or session,
 * we need to revert using other operators.
 */
static struct
{
  wmOperatorType *ot;
  KrakenPRIM *ptr;
} wm_test_autorun_revert_action_data = {
  .ot = nullptr,
  .ptr = nullptr,
};

void WM_test_autorun_revert_action_set(wmOperatorType *ot, KrakenPRIM *ptr)
{
  KLI_assert(!G.background);
  wm_test_autorun_revert_action_data.ot = NULL;
  if (wm_test_autorun_revert_action_data.ptr != NULL) {
    WM_operator_properties_free(wm_test_autorun_revert_action_data.ptr);
    MEM_freeN(wm_test_autorun_revert_action_data.ptr);
    wm_test_autorun_revert_action_data.ptr = NULL;
  }
  wm_test_autorun_revert_action_data.ot = ot;
  wm_test_autorun_revert_action_data.ptr = ptr;
}

/**
 * Parameters for #wm_file_read_post, also used for deferred initialization.
 */
struct wmFileReadPost_Params
{
  uint use_data : 1;
  uint use_userdef : 1;

  uint is_startup_file : 1;
  uint is_factory_startup : 1;
  uint reset_app_template : 1;
};

/**
 * Logic shared between #WM_file_read & #wm_homefile_read,
 * updates to make after reading a file.
 */
static void wm_file_read_post(kContext *C, const struct wmFileReadPost_Params *params)
{
  wmWindowManager *wm = CTX_wm_manager(C);

  const bool use_data = params->use_data;
  const bool use_userdef = params->use_userdef;
  const bool is_startup_file = params->is_startup_file;
  const bool is_factory_startup = params->is_factory_startup;
  const bool reset_app_template = params->reset_app_template;

  bool addons_loaded = false;

  if (use_data) {
    if (!G.background) {
      /* remove windows which failed to be added via WM_check */
      WM_window_anchorwindows_remove_invalid(C, wm);
    }
    CTX_wm_window_set(C, (wmWindow*)wm->windows.first);
  }

#ifdef WITH_PYTHON
  if (is_startup_file) {
    /* On startup (by default), Python won't have been initialized.
     *
     * The following block handles data & preferences being reloaded
     * which requires resetting some internal variables. */
    if (CTX_py_init_get(C)) {
      bool reset_all = use_userdef;
      if (use_userdef || reset_app_template) {
        /* Only run when we have a template path found. */
        if (KKE_appdir_app_template_any()) {
          KPY_run_string_eval(C,
                              (const char *[]){"kr_app_template_utils", NULL},
                              "kr_app_template_utils.reset()");
          reset_all = true;
        }
      }
      if (reset_all) {
        KPY_run_string_exec(
          C,
          (const char *[]){"kpy", "addon_utils", NULL},
          /* Refresh scripts as the preferences may have changed the user-scripts path.
           *
           * This is needed when loading settings from the previous version,
           * otherwise the script path stored in the preferences would be ignored. */
          "kpy.utils.refresh_script_paths()\n"
          /* Sync add-ons, these may have changed from the defaults. */
          "addon_utils.reset_all()");
      }
      if (use_data) {
        KPY_python_reset(C);
      }
      addons_loaded = true;
    }
  } else {
    /* run any texts that were loaded in and flagged as modules */
    if (use_data) {
      KPY_python_reset(C);
    }
    addons_loaded = true;
  }
#else
  UNUSED_VARS(is_startup_file, reset_app_template);
#endif /* WITH_PYTHON */

  Main *kmain = CTX_data_main(C);

  if (use_userdef) {
    if (is_factory_startup) {
      KKE_callback_exec_null(kmain, KKE_CB_EVT_LOAD_FACTORY_USERDEF_POST);
    }
  }

  if (use_data) {
    /* important to do before NULL'ing the context */
    KKE_callback_exec_null(kmain, KKE_CB_EVT_VERSION_UPDATE);
    KKE_callback_exec_null(kmain, KKE_CB_EVT_LOAD_POST);
    if (is_factory_startup) {
      KKE_callback_exec_null(kmain, KKE_CB_EVT_LOAD_FACTORY_STARTUP_POST);
    }
  }

  if (use_data) {
    WM_operatortype_last_properties_clear_all();

    // wm_event_do_hydra(C, true);

    ED_editors_init(C);

#if 1
    WM_event_add_notifier(C, NC_WM | ND_FILEREAD, NULL);
#else
    WM_msg_publish_static(CTX_wm_message_bus(C), WM_MSG_STATICTYPE_FILE_READ);
#endif
  }

  /* report any errors.
   * currently disabled if addons aren't yet loaded */
  if (addons_loaded) {
    WM_file_read_report(C, kmain);
  }

  if (use_data) {
    if (!G.background) {
    //   if (wm->undo_stack == nullptr) {
    //     wm->undo_stack = KKE_undosys_stack_create();
    //   } else {
    //     KKE_undosys_stack_clear(wm->undo_stack);
    //   }
    //   KKE_undosys_stack_init_from_main(wm->undo_stack, kmain);
    //   KKE_undosys_stack_init_from_context(wm->undo_stack, C);
    }
  }

  if (use_data) {
    if (!G.background) {
      /* in background mode this makes it hard to load
       * a usd file and do anything since the screen
       * won't be set to a valid value again */
      CTX_wm_window_set(C, NULL); /* exits queues */

      /* Ensure auto-run action is not used from a previous blend file load. */
      WM_test_autorun_revert_action_set(NULL, NULL);

      /* Ensure tools are registered. */
      // WM_toolsystem_init(C);
    }
  }
}

/* -------------------------------------------------------------------- */
/** \name Read USD-File Shared Utilities
 * \{ */

void WM_file_autoexec_init(const char *filepath)
{
  if (G.f & G_FLAG_SCRIPT_OVERRIDE_PREF) {
    return;
  }

  if (G.f & G_FLAG_SCRIPT_AUTOEXEC) {
    char path[FILE_MAX];
    KLI_split_dir_part(filepath, path, sizeof(path));
    if (KKE_autoexec_match(path)) {
      G.f &= ~G_FLAG_SCRIPT_AUTOEXEC;
    }
  }
}

void WM_file_read_report(kContext *C, Main *kmain)
{
  ReportList *reports = NULL;
  LISTBASE_FOREACH (Scene *, scene, &kmain->scenes) {
    if (scene->r.engine[0] &&
        KLI_findstring(&R_engines, scene->r.engine, offsetof(RenderEngineType, idname)) == NULL) {
      if (reports == NULL) {
        reports = CTX_wm_reports(C);
      }

      BKE_reportf(reports,
                  RPT_ERROR,
                  "Engine '%s' not available for scene '%s' (an add-on may need to be installed "
                  "or enabled)",
                  scene->r.engine,
                  scene->id.name + 2);
    }
  }

  if (reports) {
    if (!G.background) {
      WM_report_banner_show();
    }
  }
}
