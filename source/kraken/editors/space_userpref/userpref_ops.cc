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
 * Editors.
 * Tools for Artists.
 */

#include <string.h>

#include "USD_factory.h"
#include "USD_userdef_types.h"
#include "USD_space_types.h"

#include "KLI_listbase.h"
#ifdef WIN32
#  include "KLI_winstuff.h"
#endif
#include "KLI_path_utils.h"

#include "KKE_context.h"
#include "KKE_main.h"

#include "KKE_report.h"

#include "UI_interface.h"

#include "USD_wm_types.h"

#include "WM_event_system.h"
#include "WM_operators.h"

#include "ED_userpref.h"

#include "MEM_guardedalloc.h"

WABI_NAMESPACE_USING

/* -------------------------------------------------------------------- */
/** \name Reset Default Theme Operator
 * \{ */

static int preferences_reset_default_theme_exec(struct kContext *C, struct wmOperator *UNUSED(op))
{
  Main *kmain = CTX_data_main(C);
  UI_theme_init_default();
  UI_style_init_default();
  // WM_reinit_gizmomap_all(kmain);
  WM_event_add_notifier(C, NC_WINDOW, NULL);
  U.runtime.is_dirty = true;
  return OPERATOR_FINISHED;
}

static void PREFERENCES_OT_reset_default_theme(wmOperatorType *ot)
{
  /* identifiers */
  ot->name = "Reset to Default Theme";
  ot->idname = UPREF_ID_(PREFERENCES_OT_reset_default_theme);
  ot->description = "Reset to the default theme colors";

  /* callbacks */
  ot->exec = preferences_reset_default_theme_exec;

  /* flags */
  ot->flag = OPTYPE_REGISTER;
}

/** \} */

/* -------------------------------------------------------------------- */
/** \name Add Auto-Execution Path Operator
 * \{ */

static int preferences_autoexec_add_exec(struct kContext *UNUSED(C), struct wmOperator *UNUSED(op))
{
  kPathCompare *path_cmp = static_cast<kPathCompare *>(
    MEM_callocN(sizeof(kPathCompare), "kPathCompare"));
  KLI_addtail(&U.autoexec_paths, path_cmp);
  U.runtime.is_dirty = true;
  return OPERATOR_FINISHED;
}

static void PREFERENCES_OT_autoexec_path_add(wmOperatorType *ot)
{
  ot->name = "Add Auto-Execution Path";
  ot->idname = UPREF_ID_(PREFERENCES_OT_autoexec_path_add);
  ot->description = "Add path to exclude from auto-execution";

  ot->exec = preferences_autoexec_add_exec;

  ot->flag = OPTYPE_INTERNAL;
}

/** \} */

/* -------------------------------------------------------------------- */
/** \name Remove Auto-Execution Path Operator
 * \{ */

static int preferences_autoexec_remove_exec(struct kContext *UNUSED(C), struct wmOperator *op)
{
  const int index = FormFactory(op->ptr->GetAttribute(TfToken("index")));
  kPathCompare *path_cmp = (kPathCompare *)KLI_findlink(&U.autoexec_paths, index);
  if (path_cmp) {
    KLI_freelinkN(&U.autoexec_paths, path_cmp);
    U.runtime.is_dirty = true;
  }
  return OPERATOR_FINISHED;
}

static void PREFERENCES_OT_autoexec_path_remove(wmOperatorType *ot)
{
  ot->name = "Remove Auto-Execution Path";
  ot->idname = UPREF_ID_(PREFERENCES_OT_autoexec_path_remove);
  ot->description = "Remove path to exclude from auto-execution";

  ot->exec = preferences_autoexec_remove_exec;

  ot->flag = OPTYPE_INTERNAL;

  CreationFactory::INT::Def(ot->prim, "index", 0, "Index", "");
}

/** \} */

/* -------------------------------------------------------------------- */
/** \name Associate File Type Operator (Windows only)
 * \{ */

static bool associate_usd_poll(struct kContext *C)
{
#ifdef WIN32
  UNUSED_VARS(C);
  return true;
#else
  CTX_wm_operator_poll_msg_set(C, "Windows-only operator");
  return false;
#endif
}

static int associate_usd_exec(struct kContext *UNUSED(C), struct wmOperator *op)
{
#ifdef WIN32
  WM_cursor_wait(true);
  if (KLI_windows_register_usd_extension(true)) {
    KKE_report(op->reports, RPT_INFO, "File association registered");
    WM_cursor_wait(false);
    return OPERATOR_FINISHED;
  } else {
    KKE_report(op->reports, RPT_ERROR, "Unable to register file association");
    WM_cursor_wait(false);
    return OPERATOR_CANCELLED;
  }
#else
  UNUSED_VARS(op);
  KLI_assert_unreachable();
  return OPERATOR_CANCELLED;
#endif
}

static void PREFERENCES_OT_associate_usd(struct wmOperatorType *ot)
{
  /* identifiers */
  ot->name = "Register File Association";
  ot->description = "Use this installation for .usd files and to display thumbnails";
  ot->idname = UPREF_ID_(PREFERENCES_OT_associate_usd);

  /* api callbacks */
  ot->exec = associate_usd_exec;
  ot->poll = associate_usd_poll;
}

/** \} */

void ED_operatortypes_userpref(void)
{
  WM_operatortype_append(PREFERENCES_OT_reset_default_theme);

  WM_operatortype_append(PREFERENCES_OT_autoexec_path_add);
  WM_operatortype_append(PREFERENCES_OT_autoexec_path_remove);

  WM_operatortype_append(PREFERENCES_OT_associate_usd);
}
