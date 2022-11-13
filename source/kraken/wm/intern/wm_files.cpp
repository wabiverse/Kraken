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
#include "USD_file.h"
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
#include "WM_init_exit.h"
#include "WM_msgbus.h"
#include "WM_tokens.h"
#include "WM_files.h"
#include "WM_event_system.h"
#include "WM_window.hh"

#include "RE_engine.h"

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

/* -------------------------------------------------------------------- */
/** @name Window Matching for File Reading
 * @{ */

/**
 * To be able to read files without windows closing, opening, moving
 * we try to prepare for worst case:
 * - active window gets active screen from file
 * - restoring the screens from non-active windows
 * Best case is all screens match, in that case they get assigned to proper window.
 */
static void wm_window_match_init(kContext *C, ListBase *wmlist)
{
  *wmlist = G_MAIN->wm;

  wmWindow *active_win = CTX_wm_window(C);

  /* first wrap up running stuff */
  /* code copied from wm_init_exit.c */
  LISTBASE_FOREACH(wmWindowManager *, wm, wmlist)
  {
    WM_jobs_kill_all(wm);

    LISTBASE_FOREACH(wmWindow *, win, &wm->windows)
    {
      CTX_wm_window_set(C, win); /* needed by operator close callbacks */
      WM_event_remove_handlers(C, &win->handlers);
      WM_event_remove_handlers(C, &win->modalhandlers);
      ED_screen_exit(C, win, WM_window_get_active_screen(win));
    }

    /* NOTE(@campbellbarton): Clear the message bus so it's always cleared on file load.
     * Otherwise it's cleared when "Load UI" is set (see #USER_FILENOUI & #wm_close_and_free).
     * However it's _not_ cleared when the UI is kept. This complicates use from add-ons
     * which can re-register subscribers on file-load. To support this use case,
     * it's best to have predictable behavior - always clear. */
    if (wm->message_bus != NULL) {
      WM_msgbus_destroy(wm->message_bus);
      wm->message_bus = NULL;
    }
  }

  BLI_listbase_clear(&G_MAIN->wm);

  /* reset active window */
  CTX_wm_window_set(C, active_win);

  /* XXX Hack! We have to clear context menu here, because removing all modalhandlers
   * above frees the active menu (at least, in the 'startup splash' case),
   * causing use-after-free error in later handling of the button callbacks in UI code
   * (see ui_apply_but_funcs_after()).
   * Tried solving this by always NULL-ing context's menu when setting wm/win/etc.,
   * but it broke popups refreshing (see T47632),
   * so for now just handling this specific case here. */
  CTX_wm_menu_set(C, NULL);

  ED_editors_exit(G_MAIN, true);
}

/* -------------------------------------------------------------------- */
/** @name Read Main USD File API
 * @{ */

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

const char *WM_init_state_app_template_get(void)
{
  return wm_init_state_app_template.override ? wm_init_state_app_template.app_template : NULL;
}

/** @} */

/* -------------------------------------------------------------------- */
/** @name Read Startup & Preferences USD File API
 * @{ */



void WM_homefile_read_ex(kContext *C,
                         const struct wmHomeFileRead_Params *params_homefile,
                         ReportList *reports,
                         struct wmFileReadPost_Params **r_params_file_read_post)
{
#if 0 /* UNUSED, keep as this may be needed later & the comment below isn't self evident. */
  /* Context does not always have valid main pointer here. */
  Main *kmain = G_MAIN;
#endif
  ListBase wmbase;
  bool success = false;

  /* May be enabled, when the user configuration doesn't exist. */
  const bool use_data = params_homefile->use_data;
  const bool use_userdef = params_homefile->use_userdef;
  bool use_factory_settings = params_homefile->use_factory_settings;
  /* Currently this only impacts preferences as it doesn't make much sense to keep the default
   * startup open in the case the app-template doesn't happen to define it's own startup.
   * Unlike preferences where we might want to only reset the app-template part of the preferences
   * so as not to reset the preferences for all other Blender instances, see: T96427. */
  const bool use_factory_settings_app_template_only =
    params_homefile->use_factory_settings_app_template_only;
  const bool use_empty_data = params_homefile->use_empty_data;
  const char *filepath_startup_override = params_homefile->filepath_startup_override;
  const char *app_template_override = params_homefile->app_template_override;

  bool filepath_startup_is_factory = true;
  char filepath_startup[FILE_MAX];
  char filepath_userdef[FILE_MAX];

  /* When 'app_template' is set:
   * '{KRAKEN_USER_CONFIG}/{app_template}' */
  char app_template_system[FILE_MAX];
  /* When 'app_template' is set:
   * '{KRAKEN_SYSTEM_SCRIPTS}/startup/bl_app_templates_system/{app_template}' */
  char app_template_config[FILE_MAX];

  eKLOReadSkip skip_flags = 0;

  if (use_data == false) {
    skip_flags |= KLO_READ_SKIP_DATA;
  }
  if (use_userdef == false) {
    skip_flags |= KLO_READ_SKIP_USERDEF;
  }

  /* True if we load startup.blend from memory
   * or use app-template startup.blend which the user hasn't saved. */
  bool is_factory_startup = true;

  const char *app_template = NULL;
  bool update_defaults = false;

  if (filepath_startup_override != NULL) {
    /* pass */
  } else if (app_template_override) {
    /* This may be clearing the current template by setting to an empty string. */
    app_template = app_template_override;
  } else if (!use_factory_settings && U.app_template[0]) {
    app_template = U.app_template;
  }

  const bool reset_app_template = ((!app_template && U.app_template[0]) ||
                                   (app_template && !STREQ(app_template, U.app_template)));

  /* Options exclude each other. */
  KLI_assert((use_factory_settings && filepath_startup_override) == 0);

  if ((G.f & G_FLAG_SCRIPT_OVERRIDE_PREF) == 0) {
    SET_FLAG_FROM_TEST(G.f, (U.flag & USER_SCRIPT_AUTOEXEC_DISABLE) == 0, G_FLAG_SCRIPT_AUTOEXEC);
  }

  if (use_data) {
    if (reset_app_template) {
      /* Always load UI when switching to another template. */
      G.fileflags &= ~G_FILE_NO_UI;
    }
  }

  if (use_userdef || reset_app_template) {
#ifdef WITH_PYTHON
    /* This only runs once Blender has already started. */
    if (CTX_py_init_get(C)) {
      /* This is restored by 'wm_file_read_post', disable before loading any preferences
       * so an add-on can read their own preferences when un-registering,
       * and use new preferences if/when re-registering, see T67577.
       *
       * Note that this fits into 'wm_file_read_pre' function but gets messy
       * since we need to know if 'reset_app_template' is true. */
      KPY_run_string_eval(C, (const char *[]){"addon_utils", NULL}, "addon_utils.disable_all()");
    }
#endif /* WITH_PYTHON */
  }

  /* For regular file loading this only runs after the file is successfully read.
   * In the case of the startup file, the in-memory startup file is used as a fallback
   * so we know this will work if all else fails. */
  wm_file_read_pre(C, use_data, use_userdef);

  if (use_data) {
    /* put aside screens to match with persistent windows later */
    wm_window_match_init(C, &wmbase);
  }

  filepath_startup[0] = '\0';
  filepath_userdef[0] = '\0';
  app_template_system[0] = '\0';
  app_template_config[0] = '\0';

  const char *const cfgdir = KKE_appdir_folder_id(KRAKEN_USER_CONFIG, NULL);
  if (!use_factory_settings) {
    if (cfgdir) {
      KLI_path_join(filepath_startup, sizeof(filepath_startup), cfgdir, KRAKEN_STARTUP_FILE);
      filepath_startup_is_factory = false;
      if (use_userdef) {
        KLI_path_join(filepath_userdef, sizeof(filepath_startup), cfgdir, KRAKEN_USERPREF_FILE);
      }
    } else {
      use_factory_settings = true;
    }

    if (filepath_startup_override) {
      KLI_strncpy(filepath_startup, filepath_startup_override, FILE_MAX);
      filepath_startup_is_factory = false;
    }
  }

  /* load preferences before startup.blend */
  if (use_userdef) {
    if (use_factory_settings_app_template_only) {
      /* Use the current preferences as-is (only load in the app_template preferences). */
      skip_flags = static_cast<eKLOReadSkip>(skip_flags | KLO_READ_SKIP_USERDEF);
    } else if (!use_factory_settings && KLI_exists(filepath_userdef)) {
      UserDef *userdef = CTX_data_prefs(C);
      if (userdef != nullptr) {
        SWAP(UserDef, *(&U), *userdef);
        MEM_delete(userdef);
        userdef = nullptr;

        skip_flags = static_cast<eKLOReadSkip>(skip_flags | KLO_READ_SKIP_USERDEF);
        printf("Read prefs: %s\n", filepath_userdef);
      }
    }
  }

  if ((app_template != NULL) && (app_template[0] != '\0')) {
    if (!KKE_appdir_app_template_id_search(app_template,
                                           app_template_system,
                                           sizeof(app_template_system))) {
      /* Can safely continue with code below, just warn it's not found. */
      KKE_reportf(reports, RPT_WARNING, "Application Template '%s' not found", app_template);
    }

    /* Insert template name into startup file. */

    /* note that the path is being set even when 'use_factory_settings == true'
     * this is done so we can load a templates factory-settings */
    if (!use_factory_settings) {
      KLI_path_join(app_template_config, sizeof(app_template_config), cfgdir, app_template);
      KLI_path_join(filepath_startup,
                    sizeof(filepath_startup),
                    app_template_config,
                    KRAKEN_STARTUP_FILE);
      filepath_startup_is_factory = false;
      if (KLI_access(filepath_startup, R_OK) != 0) {
        filepath_startup[0] = '\0';
      }
    } else {
      filepath_startup[0] = '\0';
    }

    if (filepath_startup[0] == '\0') {
      KLI_path_join(filepath_startup,
                    sizeof(filepath_startup),
                    app_template_system,
                    KRAKEN_STARTUP_FILE);
      filepath_startup_is_factory = true;

      /* Update defaults only for system templates. */
      update_defaults = true;
    }
  }

  if (!use_factory_settings || (filepath_startup[0] != '\0')) {
    if (KLI_access(filepath_startup, R_OK) == 0) {
      const struct KrakenFileReadParams params = {
        .is_startup = true,
        .skip_flags = skip_flags | KLO_READ_SKIP_USERDEF,
      };
      KrakenFileReadReport bf_reports = {.reports = reports};
      struct KrakenFileData *bfd = wm_usdfile_read(filepath_startup, &params, &bf_reports);

      if (bfd != NULL) {
        KKE_usdfile_read_setup_ex(C,
                                  bfd,
                                  &params,
                                  &bf_reports,
                                  update_defaults && use_data,
                                  app_template);
        success = true;
      }
    }
    if (success) {
      is_factory_startup = filepath_startup_is_factory;
    }
  }

  if (use_userdef) {
    if ((skip_flags & KLO_READ_SKIP_USERDEF) == 0) {
      UserDef *userdef_default = KKE_usdfile_userdef_from_defaults();
      KKE_blender_userdef_data_set_and_free(userdef_default);
      skip_flags |= KLO_READ_SKIP_USERDEF;
    }
  }

  if (success == false && filepath_startup_override && reports) {
    /* We can not return from here because wm is already reset */
    KKE_reportf(reports, RPT_ERROR, "Could not read '%s'", filepath_startup_override);
  }

  if (success == false) {
    const struct KrakenFileReadParams params = {
      .is_startup = true,
      .skip_flags = skip_flags,
    };
    struct KrakenFileData *bfd = KKE_usdfile_read_from_memory(datatoc_startup_blend,
                                                              datatoc_startup_blend_size,
                                                              &params,
                                                              NULL);
    if (bfd != NULL) {
      KKE_usdfile_read_setup_ex(C, bfd, &params, &(KrakenFileReadReport){NULL}, true, NULL);
      success = true;
    }

    if (use_data && KLI_listbase_is_empty(&wmbase)) {
      WM_clear_default_size(C);
    }
  }

  if (use_empty_data) {
    KKE_usdfile_read_make_empty(C);
  }

  /* Load template preferences,
   * unlike regular preferences we only use some of the settings,
   * see: KKE_blender_userdef_set_app_template */
  if (app_template_system[0] != '\0') {
    char temp_path[FILE_MAX];
    temp_path[0] = '\0';
    if (!use_factory_settings) {
      KLI_path_join(temp_path, sizeof(temp_path), app_template_config, KRAKEN_USERPREF_FILE);
      if (KLI_access(temp_path, R_OK) != 0) {
        temp_path[0] = '\0';
      }
    }

    if (temp_path[0] == '\0') {
      KLI_path_join(temp_path, sizeof(temp_path), app_template_system, KRAKEN_USERPREF_FILE);
    }

    if (use_userdef) {
      UserDef *userdef_template = NULL;
      /* just avoids missing file warning */
      if (KLI_exists(temp_path)) {
        userdef_template = KKE_usdfile_userdef_read(temp_path, NULL);
      }
      if (userdef_template == NULL) {
        /* we need to have preferences load to overwrite preferences from previous template */
        userdef_template = KKE_usdfile_userdef_from_defaults();
      }
      if (userdef_template) {
        KKE_blender_userdef_app_template_data_set_and_free(userdef_template);
        userdef_template = NULL;
      }
    }
  }

  if (app_template_override) {
    KLI_strncpy(U.app_template, app_template_override, sizeof(U.app_template));
  }

  Main *kmain = CTX_data_main(C);

  if (use_userdef) {
    /* check userdef before open window, keymaps etc */
    wm_init_userdef(kmain);
  }

  if (use_data) {
    /* match the read WM with current WM */
    wm_window_match_do(C, &wmbase, &kmain->wm, &kmain->wm);
  }

  if (use_userdef) {
    /* Clear keymaps because the current default keymap may have been initialized
     * from user preferences, which have been reset. */
    LISTBASE_FOREACH(wmWindowManager *, wm, &kmain->wm)
    {
      if (wm->defaultconf) {
        wm->defaultconf->flag &= ~KEYCONF_INIT_DEFAULT;
      }
    }
  }

  if (use_data) {
    WM_check(C); /* opens window(s), checks keymaps */

    kmain->filepath[0] = '\0';
  }

  {
    const struct wmFileReadPost_Params params_file_read_post = {
      .use_data = use_data,
      .use_userdef = use_userdef,
      .is_startup_file = true,
      .is_factory_startup = is_factory_startup,
      .reset_app_template = reset_app_template,
    };
    if (r_params_file_read_post == NULL) {
      wm_file_read_post(C, &params_file_read_post);
    } else {
      *r_params_file_read_post = MEM_mallocN(sizeof(struct wmFileReadPost_Params), __func__);
      **r_params_file_read_post = params_file_read_post;

      /* Match #wm_file_read_post which leaves the window cleared too. */
      CTX_wm_window_set(C, NULL);
    }
  }
}

void wm_homefile_read(kContext *C,
                      const struct wmHomeFileRead_Params *params_homefile,
                      ReportList *reports)
{
  wm_homefile_read_ex(C, params_homefile, reports, NULL);
}

void wm_homefile_read_post(struct kContext *C,
                           const struct wmFileReadPost_Params *params_file_read_post)
{
  wm_file_read_post(C, params_file_read_post);
  MEM_freeN((void *)params_file_read_post);
}

/** @} */

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

/* -------------------------------------------------------------------- */
/** @name Read USD-File Shared Utilities
 * @{ */

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
  LISTBASE_FOREACH(Scene *, scene, &kmain->scenes)
  {
    if (scene->r.engine[0] &&
        KLI_findstring(&R_engines, scene->r.engine, offsetof(RenderEngineType, idname)) == NULL) {
      if (reports == NULL) {
        reports = CTX_wm_reports(C);
      }

      KKE_reportf(reports,
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

/**
 * Logic shared between #WM_file_read & #WM_homefile_read,
 * call before loading a file.
 * @note In the case of #WM_file_read the file may fail to load.
 * Change here shouldn't cause user-visible changes in that case.
 */
static void wm_file_read_pre(kContext *C, bool use_data, bool UNUSED(use_userdef))
{
  if (use_data) {
    KKE_callback_exec_null(CTX_data_main(C), KKE_CB_EVT_LOAD_PRE);
    KLI_timer_on_file_load();
  }

  /* Always do this as both startup and preferences may have loaded in many font's
   * at a different zoom level to the file being loaded. */
  UI_view2d_zoom_cache_reset();

  ED_preview_restart_queue_free();
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
 * Logic shared between #WM_file_read & #WM_homefile_read,
 * updates to make after reading a file. */
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
    CTX_wm_window_set(C, (wmWindow *)wm->windows.first);
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

/** @} */