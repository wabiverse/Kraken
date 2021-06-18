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

#include "WM_api.h"

#include "CKE_context.h"

#include <wabi/base/tf/notice.h>

#include <tbb/atomic.h>

/**
 *  -----  The Covah WindowManager. ----- */


WABI_NAMESPACE_BEGIN


/* ------ */


/**
 *  -----  Forward Declarations. ----- */


TF_DECLARE_WEAK_AND_REF_PTRS(MsgBusCallback);

typedef MsgBusCallbackPtr MsgBus;


#define IDENT_ROMEO 48484
#define IDENT_JULIET 64121

/**
 *  -----  The MsgBus Callback. ----- */


struct MsgBusCallback : public TfWeakBase
{

  MsgBusCallback(int identity, TfNotice const &notice);

  void PushNotif(const TfNotice &notice,
                 MsgBus const &sender);

 private:
  int m_ident;
  std::string m_name;
  TfNotice m_notice;
  tbb::atomic<int> m_counter;
  bool m_flag = true;
};


/* ------ */


/**
 *  -----  MsgBus Initialization. ----- */


void WM_msgbus_register(void);


WABI_NAMESPACE_END