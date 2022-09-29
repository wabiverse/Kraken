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
#include "WM_window.h"
#include "WM_files.h"

#include "USD_factory.h"
#include "USD_screen.h"
#include "USD_userpref.h"
#include "USD_window.h"

#include "LUXO_access.h"
#include "LUXO_define.h"

#include "KKE_context.h"
#include "KKE_utils.h"

#define UNDOCUMENTED_OPERATOR_TIP N_("(undocumented operator)")

KRAKEN_NAMESPACE_BEGIN

static RHash *global_ops_hash = NULL;
/** Counter for operator-properties that should not be tagged with #OP_PROP_TAG_ADVANCED. */
static int ot_prop_basic_count = -1;

wmOperatorType::wmOperatorType() {}

wmOperatorType *WM_operatortype_find(const TfToken &idname)
{
  if (!idname.IsEmpty()) {
    wmOperatorType *ot;

    ot = (wmOperatorType *)KKE_rhash_lookup((RHash *)global_ops_hash, idname);
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

  /* XXX All ops should have a description but for now allow them not to. */
  PRIM_def_struct_ui_text(ot->prim, ot->name, ot->description ? ot->description : UNDOCUMENTED_OPERATOR_TIP);
  PRIM_def_struct_identifier(KRAKEN_STAGE, ot->prim, ot->idname);

  KKE_rhash_insert(global_ops_hash, ot->idname, ot);
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

void WM_operator_properties_create_ptr(KrakenPRIM *ptr, wmOperatorType *ot)
{
  LUXO_pointer_create(&G.main->wm.front()->id, ot->prim, NULL, ptr);
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
  for (auto &prop : ptr->GetAttributes())
  {
    if (ptr->ClearActive()) {
      changed = true;
    }
  }

  return changed;
}

void WM_operators_init(kContext *C)
{
  wmWindowManager *wm = CTX_wm_manager(C);
  global_ops_hash = new RHashOp();
}

void WM_operators_register(kContext *C)
{
  WM_window_operators_register();
  WM_file_operators_register();
}

KRAKEN_NAMESPACE_END