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


/**
 *  -----  The Covah WindowManager. ----- */


WABI_NAMESPACE_BEGIN


/* ------ */


/**
 *  -----  The Base Notice. ----- */


struct BaseNotice : public TfNotice
{
  BaseNotice(const std::string &what);

  const std::string &GetWhat() const;

  ~BaseNotice();

 protected:
  const std::string m_what;
};


/* ------ */


/**
 *  -----  The Msg Notice. ----- */


struct MainNotice : public BaseNotice
{
  MainNotice(const std::string &what);
};


/* ------ */


/**
 *  -----  The Msg Listener. ----- */


struct MainListener : public TfWeakBase
{
  MainListener();

  void Revoke();
  void ProcessNotice(const TfNotice &n);
  void ProcessMainNotice(const MainNotice &n);

  /** For Debugging. Diagnostics. */
  static void WM_msgbus_dump(std::ostream *log,
                             std::vector<std::string> *li,
                             std::mutex *mutex);

 private:
  TfNotice::Key _processMainKey;
  std::vector<std::string> mainThreadList;
  std::mutex _mainThreadLock;
};


/* ------ */


/**
 *  -----  MsgBus Initialization. ----- */


void WM_msgbus_register(void);


WABI_NAMESPACE_END