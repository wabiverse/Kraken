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

#include "WM_msgbus.h"
#include "WM_debug_codes.h"
#include "WM_operators.h"
#include "WM_window.h"

#include "UNI_screen.h"
#include "UNI_userpref.h"
#include "UNI_window.h"

#include "CKE_context.h"

#include <mutex>
#include <string>
#include <vector>


/**
 *  -----  The Covah WindowManager. ----- */


WABI_NAMESPACE_BEGIN


/* ------ */


/**
 *  -----  The MsgBus Callback. ----- */


MsgBusCallback::MsgBusCallback(int identity, TfNotice const &notice)
  : m_counter(1),
    m_ident(identity),
    m_notice(notice)
{}


void MsgBusCallback::PushNotif(const TfNotice &notice,
                               MsgBus const &sender)
{
  TF_DEBUG(COVAH_DEBUG_MSGBUS).Msg("!! Hello from MsgBus.\n");
  m_flag = (sender->m_ident == m_ident) ? true : false;
  m_name = (m_ident == IDENT_ROMEO) ? STRINGIFY_ARG(IDENT_ROMEO) :
                                      STRINGIFY_ARG(IDENT_JULIET);
  TF_DEBUG(COVAH_DEBUG_MSGBUS).Msg("%s\n", m_flag ? "true" : "false");
  TF_DEBUG(COVAH_DEBUG_MSGBUS).Msg("ID: %s\n", CHARSTR(m_name));
  TF_DEBUG(COVAH_DEBUG_MSGBUS).Msg("Count: %s\n", CHARALL(m_counter));
  ++m_counter;
}


/**
 *  -----  MsgBus Initialization. ----- */


void WM_msgbus_register(void)
{
  /* ------ */


  /** 
   * Sends a notification. */

  TfNotice notice = TfNotice();


  /**
   *  -----  Romeo. ----- */

  /** 
   * A callback function instance. */
  MsgBusCallback *callbackFunctionRomeo = new MsgBusCallback(IDENT_ROMEO, notice);

  /** 
   * A sender, invokes a callback function. */
  MsgBus romeo(callbackFunctionRomeo);

  /** 
   * This sender is the only one allowed to
   * call this callback, since it was registered
   * to it via the TfNotice Register call. This
   * sender will remain able to continue sending 
   * push notifications until it's key is revoked. */
  auto romeokey = TfNotice::Register(romeo, &MsgBusCallback::PushNotif, romeo);


  /**
   *  -----  Juliet. ----- */

  /** 
   * Now Create Juliet. */
  MsgBusCallback *callbackFunctionJuliet = new MsgBusCallback(IDENT_JULIET, notice);
  MsgBus juliet(callbackFunctionJuliet);
  TfNotice::Register(juliet, &MsgBusCallback::PushNotif, juliet);


  /** 
   * Have Romeo Subscribe to Juliet. */
  TfNotice::Register(romeo, &MsgBusCallback::PushNotif, juliet);


  notice.Send(romeo);
  notice.Send(juliet);


  TfNotice::Revoke(romeokey);
  notice.Send(romeo);
}


WABI_NAMESPACE_END