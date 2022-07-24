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
 * Window Manager.
 * Making GUI Fly.
 */

#include "WM_operators.h"
#include "WM_debug_codes.h"
#include "WM_msgbus.h"
#include "WM_tokens.h"
#include "WM_files.h"
#include "WM_event_system.h"

#include "USD_factory.h"
#include "USD_screen.h"
#include "USD_userpref.h"
#include "USD_window.h"

#include "LUXO_access.h"

#include "KKE_context.h"
#include "KKE_utils.h"
#include "KKE_appdir.h"
#include "KKE_report.h"

#include <filesystem>

namespace fs = std::filesystem;

WABI_NAMESPACE_BEGIN

static int wm_user_datafiles_write_exec(kContext *C, wmOperator *op)
{
  Main *bmain = CTX_data_main(C);
  wmWindowManager *wm = CTX_wm_manager(C);

  const char *const appdir = KKE_appdir_copy_recursive(KRAKEN_SYSTEM_DATAFILES,
                                                       KRAKEN_USER_DATAFILES);
  if (appdir == NULL) {
    KKE_report(op->reports, RPT_ERROR, "Unable to create user datafiles path");
    return OPERATOR_CANCELLED;
  }

  TF_STATUS("Writing user datafiles: '%s' ", appdir);

  TF_MSG_SUCCESS("ok");
  KKE_report(op->reports, RPT_INFO, "User datafiles created");

  return OPERATOR_FINISHED;
}

void WM_OT_files_create_appdata(wmOperatorType *ot)
{
  ot->name = "Create User AppData Directory";
  ot->idname = IDNAME(WM_OT_files_create_appdata);
  ot->description = "Install user directories at OS appdata";

  ot->exec = wm_user_datafiles_write_exec;
}

void WM_file_operators_register(void)
{
  /* ------ */

  WM_operatortype_append(WM_OT_files_create_appdata);

  /* ------ */
}

void WM_files_init(kContext *C)
{
  /* --- Probably temporary, just create user appdirs for now. --- */

#if BLENDER_CAN_DO_THIS_AND_WE_CANT
  // Apple doesn't like this at all, will have to check
  // how to obtain this policy -- likely due to us having
  // to set XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME in order
  // to be notarized by apple, but somehow Blender doesn't
  // have to worry about this.

  KrakenPRIM props_ptr;
  wmOperatorType *ot = WM_operatortype_find(IDNAME(WM_OT_files_create_appdata));

  WM_operator_properties_create_ptr(&props_ptr, ot);

  WM_operator_name_call_ptr(C, ot, WM_OP_INVOKE_DEFAULT, &props_ptr);
  WM_operator_properties_free(&props_ptr);
#endif /* BLENDER_CAN_DO_THIS_AND_WE_CANT */

  /* --- */
}

WABI_NAMESPACE_END