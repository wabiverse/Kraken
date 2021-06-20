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

#pragma once

#include <wabi/base/tf/notice.h>
#include <wabi/base/tf/refBase.h>
#include <wabi/usd/usd/common.h>

#include <tbb/atomic.h>

#include "WM_api.h"
#include "WM_operators.h"

#include "CKE_context.h"
#include "CKE_robinhood.h"

/**
 *  -----  The Covah WindowManager. ----- */


WABI_NAMESPACE_BEGIN


/* ------ */


/**
 *  -----  Forward Declarations. ----- */


TF_DECLARE_WEAK_AND_REF_PTRS(MsgBusCallback);

typedef MsgBusCallbackPtr MsgBus;

/**
 *  -----  The MsgBus Callback. ----- */


struct MsgBusCallback : public TfWeakBase
{
  /** Runs & Invokes all the Ops. */
  MsgBusCallback(wmOperatorType *ot);

  /** Comm Link. You've got mail. */
  void COMM(const TfNotice &notice,
            MsgBus const &sender);

  /** Reference Count. */
  tbb::atomic<int> ref;

  /** Notify @ Subscribe MsgBus. */
  TfNotice notice;

  struct
  {
    wmOperatorType *type;
  } op;
};


void WM_msgbus_register(const cContext &C);

/* ------ */

WABI_NAMESPACE_END