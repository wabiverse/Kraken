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

#pragma once

/**
 * @file
 * Window Manager.
 * Making GUI Fly.
 */

#include "WM_api.h"
#include "WM_window.hh"
#include "WM_msgbus.h"
#include "WM_operators.h"

#include "KKE_context.h"

#include "WM_reports.h"

struct wmHomeFileRead_Params
{
  /** Load data, disable when only loading user preferences. */
  unsigned int use_data : 1;
  /** Load factory settings as well as startup file (disabled for "File New"). */
  unsigned int use_userdef : 1;

  /**
   * Ignore on-disk startup file, use bundled `datatoc_startup_usd` instead.
   * Used for "Restore Factory Settings".
   */
  unsigned int use_factory_settings : 1;
  /** Read factory settings from the app-templates only (keep other defaults). */
  unsigned int use_factory_settings_app_template_only : 1;
  /**
   * Load the startup file without any data-blocks.
   * Useful for automated content generation, so the file starts without data.
   */
  unsigned int use_empty_data : 1;
  /**
   * Optional path pointing to an alternative blend file (may be NULL).
   */
  const char *filepath_startup_override;
  /**
   * Template to use instead of the template defined in user-preferences.
   * When not-null, this is written into the user preferences.
   */
  const char *app_template_override;
};


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
 * Called on startup, (context entirely filled with NULLs)
 * or called for 'New File' both `startup.usd` and `userpref.usd` are checked.
 *
 * @param r_params_file_read_post: Support postponed initialization,
 * needed for initial startup when only some sub-systems have been initialized.
 * When non-null, #wm_file_read_post doesn't run, instead it's arguments are stored
 * in this return argument.
 * The caller is responsible for calling #wm_homefile_read_post with this return argument. */
void WM_homefile_read_ex(struct kContext *C,
                         const struct wmHomeFileRead_Params *params_homefile,
                         struct ReportList *reports,
                         struct wmFileReadPost_Params **r_params_file_read_post);
void WM_homefile_read(struct kContext *C,
                      const struct wmHomeFileRead_Params *params_homefile,
                      struct ReportList *reports);

void WM_file_autoexec_init(const char *filepath);
void WM_file_read_report(struct kContext *C, Main *kmain);
void WM_file_operators_register(void);

void WM_files_init(struct kContext *C);

void WM_init_state_app_template_set(const char *app_template);
const char *WM_init_state_app_template_get(void);

void WM_close_file_dialog(struct kContext *C, struct wmGenericCallback *post_action);

void WM_test_autorun_revert_action_set(struct wmOperatorType *ot, struct KrakenPRIM *ptr);
