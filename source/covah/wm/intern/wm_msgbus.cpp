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

#include "WM_api.h"

#include "WM_debug_codes.h"
#include "WM_msgbus.h"
#include "WM_operators.h"
#include "WM_window.h"

#include "UNI_path_defaults.h"
#include "UNI_screen.h"
#include "UNI_userpref.h"
#include "UNI_window.h"

#include "CKE_context.h"
#include "CKE_main.h"

#include <mutex>
#include <string>
#include <vector>


/**
 *  -----  The Covah WindowManager. ----- */


WABI_NAMESPACE_BEGIN


/* ------ */


/**
 *  -----  The MsgBus Callback. ----- */


MsgBusCallback::MsgBusCallback(wmOperatorType *ot)
  : ref(1),
    notice(ot->notice),
    op({.type = ot})
{}


void MsgBusCallback::OperatorCOMM(const TfNotice &notice,
                                  MsgBus const &sender)
{
  TF_DEBUG(COVAH_DEBUG_MSGBUS).Msg("MsgBus\n");
  TF_DEBUG(COVAH_DEBUG_MSGBUS).Msg("  Type: Operator\n");
  TF_DEBUG(COVAH_DEBUG_MSGBUS).Msg("    ID: %s\n", CHARALL(op.type->idname));
  TF_DEBUG(COVAH_DEBUG_MSGBUS).Msg("    Name: %s\n", op.type->name);
  TF_DEBUG(COVAH_DEBUG_MSGBUS).Msg("    Description: %s\n", op.type->description);
  TF_DEBUG(COVAH_DEBUG_MSGBUS).Msg("\n");
  ++ref;
}


WABI_NAMESPACE_END