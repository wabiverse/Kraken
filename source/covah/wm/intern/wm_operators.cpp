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
#include "WM_msgbus.h"
#include "WM_tokens.h"
#include "WM_window.h"

#include "UNI_screen.h"
#include "UNI_userpref.h"
#include "UNI_window.h"

#include "CKE_context.h"

WABI_NAMESPACE_BEGIN

void WM_operatortype_append(const cContext &C, void (*opfunc)(wmOperatorType *))
{
  /* ------ */

  wmOperatorType *ot = new wmOperatorType();
  opfunc(ot);

  /* ------ */

  /** Hashed. */
  wmWindowManager wm = CTX_wm_manager(C);
  wm->operators->insert(typename RHashOp::value_type(
    std::make_pair(ot->idname, ot)));

  /* ------ */

  TfNotice notice = TfNotice();
  MsgBusCallback *cb = new MsgBusCallback(ot);
  MsgBus invoker(cb);
  TfNotice::Register(invoker, &MsgBusCallback::OperatorCOMM, invoker);

  /* ------ */

  /** Operator says Hello. */
  notice.Send(invoker);
}

void WM_operators_init(const cContext &C)
{
  wmWindowManager wm = CTX_wm_manager(C);
  wm->operators = new RHashOp();
}

void WM_operators_register(const cContext &C)
{
  WM_window_operators_register(C);
}

WABI_NAMESPACE_END