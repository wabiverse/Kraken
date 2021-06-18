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
 *  -----  The Base Class. ----- */


BaseNotice::BaseNotice(const std::string &what)
  : TfNotice(),
    m_what(what)
{}


BaseNotice::~BaseNotice()
{}


const std::string &BaseNotice::GetWhat() const
{
  return m_what;
}


BaseNotice::~BaseNotice()
{}


/* ------ */


/**
 *  -----  The Msg Notice. ----- */


MainNotice::MainNotice(const std::string &what)
  : BaseNotice(what)
{}


/* ------ */


/**
 *  -----  The Msg Listener. ----- */


MainListener::MainListener()
  : TfWeakBase()
{
  TfWeakPtr<MainListener> me(this);
  TfNotice::Register(me, &MainListener::ProcessNotice);
  _processMainKey = TfNotice::Register(me, &MainListener::ProcessMainNotice);
}


void MainListener::WM_msgbus_dump(std::ostream *log, std::vector<std::string> *li, std::mutex *mutex)
{
  std::lock_guard<std::mutex> lock(*mutex);
  std::sort(li->begin(), li->end());
  TF_FOR_ALL (lines, li)
  {
    *log << *lines << std::endl;
  }

  li->clear();
}


void MainListener::Revoke()
{
  TfNotice::Revoke(_processMainKey);
}


void MainListener::ProcessNotice(const TfNotice &n)
{
  std::lock_guard<std::mutex> lock(_mainThreadLock);
  mainThreadList.push_back(
    "MainListener::ProcessNotice"
    " got notice of type " +
    TfType::Find(n).GetTypeName());

  printf("%s\n", mainThreadList.data()->c_str());
}


void MainListener::ProcessMainNotice(const MainNotice &n)
{
  std::lock_guard<std::mutex> lock(_mainThreadLock);
  mainThreadList.push_back("MainListener::ProcessMainNotice got " + n.GetWhat());

  printf("%s\n", mainThreadList.data()->c_str());
}


/**
 *  -----  MsgBus Initialization. ----- */


void WM_msgbus_register(void)
{
  MainListener *listener = new MainListener();
  {
    MainNotice("Covah Notice.").Send();
  }
}


/**
 *  -----  Registration with TfRegistry. ----- */


TF_REGISTRY_FUNCTION(TfType)
{
  TfType::Define<MainNotice, TfType::Bases<TfNotice>>();
}


WABI_NAMESPACE_END