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

#include "WM_cursors.h"
#include "WM_operators.h"
#include "WM_debug_codes.h"
#include "WM_msgbus.h"
#include "WM_tokens.h"
#include "WM_window.hh"
#include "WM_files.h"

#include "USD_factory.h"
#include "USD_screen.h"
#include "USD_userpref.h"
#include "USD_window.h"

#include "LUXO_access.h"
#include "LUXO_define.h"

#include "KLI_rhash.h"
#include "KLI_dynstr.h"

#include "KKE_context.h"
#include "KKE_utils.h"
#include "KKE_global.h"

#define UNDOCUMENTED_OPERATOR_TIP N_("(undocumented operator)")

static RHash *global_ops_hash = NULL;
/** Counter for operator-properties that should not be tagged with #OP_PROP_TAG_ADVANCED. */
static int ot_prop_basic_count = -1;

wmOperatorType *WM_operatortype_find(const TfToken &idname)
{
  if (!idname.IsEmpty()) {
    wmOperatorType *ot;

    ot = (wmOperatorType *)KLI_rhash_lookup((RHash *)global_ops_hash, idname.data());
    if (ot) {
      return ot;
    }

    TF_DEBUG(KRAKEN_DEBUG_OPERATORS).Msg("Unknown operator '%s'\n", CHARALL(idname));
  } else {
    TF_DEBUG(KRAKEN_DEBUG_OPERATORS).Msg("Operator has no id.\n");
  }

  return NULL;
}

/* -------------------------------------------------------------------- */
/** \name Operator Type Append
 * \{ */

static wmOperatorType *wm_operatortype_append__begin(void)
{
  wmOperatorType *ot = (wmOperatorType *)MEM_callocN(sizeof(wmOperatorType), "operatortype");

  KLI_assert(ot_prop_basic_count == -1);

  ot->prim = PRIM_def_struct_ptr(KRAKEN_STAGE, SdfPath("Operators"));
  ot->cursor_pending = WM_CURSOR_PICK_AREA;

  return ot;
}

static void wm_operatortype_append__end(wmOperatorType *ot)
{
  if (ot->name == NULL) {
    printf("Operator '%s' has no name property\n", ot->idname.GetText());
  }
  KLI_assert((ot->description == NULL) || (ot->description[0]));

  /* Allow calling _begin without _end in operatortype creation. */
  // WM_operatortype_props_advanced_end(ot);

  PRIM_def_struct_identifier(KRAKEN_STAGE, ot->prim, ot->idname);

  /* XXX All ops should have a description but for now allow them not to. */
  PRIM_def_struct_ui_text(ot->prim,
                          ot->name,
                          ot->description ? ot->description : UNDOCUMENTED_OPERATOR_TIP);

  KLI_rhash_insert(global_ops_hash, (void *)ot->idname.data(), ot);
}

void WM_operatortype_append(void (*opfunc)(wmOperatorType *))
{
  /* ------ */

  wmOperatorType *ot = wm_operatortype_append__begin();
  opfunc(ot);
  wm_operatortype_append__end(ot);

  /* ------ */
}

void WM_operator_properties_free(KrakenPRIM *ptr)
{
  // IDProperty *properties = ptr->data;

  // if (properties) {
  //   IDP_FreeProperty(properties);
  //   ptr->data = NULL; /* just in case */
  // }
}

size_t WM_operator_py_idname(char *dst, const char *src)
{
  const char *sep = strstr(src, "_OT_");
  if (sep) {
    int ofs = (sep - src);

    /* NOTE: we use ascii `tolower` instead of system `tolower`, because the
     * latter depends on the locale, and can lead to `idname` mismatch. */
    memcpy(dst, src, sizeof(char) * ofs);
    KLI_str_tolower_ascii(dst, ofs);

    dst[ofs] = '.';
    return KLI_strncpy_rlen(dst + (ofs + 1), sep + 4, OP_MAX_TYPENAME - (ofs + 1)) + (ofs + 1);
  }
  /* Should not happen but support just in case. */
  return KLI_strncpy_rlen(dst, src, OP_MAX_TYPENAME);
}

char *WM_operator_pystring_ex(kContext *C,
                              wmOperator *op,
                              const bool all_args,
                              const bool macro_args,
                              wmOperatorType *ot,
                              KrakenPRIM *opptr)
{
  char idname_py[OP_MAX_TYPENAME];

  /* for building the string */
  DynStr *dynstr = KLI_dynstr_new();

  /* arbitrary, but can get huge string with stroke painting otherwise */
  int max_prop_length = 10;

  WM_operator_py_idname(idname_py, ot->idname.GetText());
  KLI_dynstr_appendf(dynstr, "kpy.ops.%s(", idname_py);

  if (op && op->macro.front()) {
    /* Special handling for macros, else we only get default values in this case... */
    wmOperator *opm;
    bool first_op = true;

    for (auto opm : op->macro) {
      KrakenPRIM *opmptr = opm->ptr;
      KrakenPRIM opmptr_default;
      if (opmptr == NULL) {
        WM_operator_properties_create_ptr(&opmptr_default, opm->type);
        opmptr = &opmptr_default;
      }

      char *cstring_args = LUXO_pointer_as_string_id(C, opmptr);
      if (first_op) {
        KLI_dynstr_appendf(dynstr, "%s=%s", opm->type->idname.GetText(), cstring_args);
        first_op = false;
      } else {
        KLI_dynstr_appendf(dynstr, ", %s=%s", opm->type->idname.GetText(), cstring_args);
      }
      MEM_freeN(cstring_args);

      if (opmptr == &opmptr_default) {
        WM_operator_properties_free(&opmptr_default);
      }
    }
  } else {
    /* only to get the original props for comparisons */
    KrakenPRIM opptr_default;
    const bool macro_args_test = ot->macro.front() ? macro_args : true;

    if (opptr == NULL) {
      WM_operator_properties_create_ptr(&opptr_default, ot);
      opptr = &opptr_default;
    }

    char *cstring_args =
      LUXO_pointer_as_string_keywords(C, opptr, false, all_args, macro_args_test, max_prop_length);
    KLI_dynstr_append(dynstr, cstring_args);
    MEM_freeN(cstring_args);

    if (opptr == &opptr_default) {
      WM_operator_properties_free(&opptr_default);
    }
  }

  KLI_dynstr_append(dynstr, ")");

  char *cstring = KLI_dynstr_get_cstring(dynstr);
  KLI_dynstr_free(dynstr);
  return cstring;
}

void WM_operator_properties_create_ptr(KrakenPRIM *ptr, wmOperatorType *ot)
{
  LUXO_pointer_create(&((wmWindowManager *)G.main->wm.first)->id, ot->prim, NULL, ptr);
}

const char *WM_operatortype_name(struct wmOperatorType *ot, KrakenPRIM *properties)
{
  const char *name = NULL;

  if (properties) {
    ot->name = properties->GetName().GetText();
    name = ot->name;
  } else {
    name = ot->name;
  }

  return (name && name[0]) ? name : ot->prim->GetName().GetText();
}

bool WM_operator_properties_default(KrakenPRIM *ptr, bool do_update)
{
  bool changed = false;
  for (auto &prop : ptr->GetAttributes()) {
    if (ptr->ClearActive()) {
      changed = true;
    }
  }

  return changed;
}

static void operatortype_rhash_free_cb(wmOperatorType *ot)
{
  // if (ot->last_properties) {
  //   IDP_FreeProperty(ot->last_properties);
  // }

  if (!ot->macro.empty()) {
    for (auto &otmacro : ot->macro) {
      if (otmacro->ptr && otmacro->ptr->IsValid()) {
        WM_operator_properties_free(otmacro->ptr);
        MEM_delete(otmacro->ptr);
      }
    }
    ot->macro.clear();
  }

  // if (ot->prim_ext.prim) {
  /* python operator, allocs own string */
  // MEM_freeN((void *)ot->idname);
  // }

  MEM_delete(ot);
}

void WM_operators_free(void)
{
  KLI_rhash_free(global_ops_hash, NULL, (RHashValFreeFP)operatortype_rhash_free_cb);
  global_ops_hash = NULL;
}

void WM_operators_init(kContext *C)
{
  wmWindowManager *wm = CTX_wm_manager(C);
  global_ops_hash = KLI_rhash_str_new_ex("WM_operators_init rh", 2048);
}

void WM_operators_register(kContext *C)
{
  WM_window_operators_register();
  WM_file_operators_register();
}
