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
#include "WM_window.h"
#include "WM_files.h"

#include "USD_factory.h"
#include "USD_screen.h"
#include "USD_userpref.h"
#include "USD_window.h"

#include "LUXO_access.h"

#include "KKE_context.h"
#include "KKE_utils.h"

WABI_NAMESPACE_BEGIN

static RHashOp *global_ops_hash = NULL;

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

void WM_operatortype_append(void (*opfunc)(wmOperatorType *))
{
  /* ------ */

  wmOperatorType *ot = new wmOperatorType();
  opfunc(ot);

  /* ------ */

  /** Hashed. */
  global_ops_hash->insert(typename RHashOp::value_type(std::make_pair(ot->idname, ot)));

  /* ------ */

  TfNotice notice = TfNotice();
  MsgBusCallback *cb = new MsgBusCallback(ot);
  MsgBus invoker(cb);
  TfNotice::Register(invoker, &MsgBusCallback::OperatorCOMM, invoker);

  /* ------ */

  /** Operator says Hello. */
  notice.Send(invoker);
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
  // G.main->wm.at(1)
  LUXO_pointer_create(ot->pixar, NULL, ptr);
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

WABI_NAMESPACE_END