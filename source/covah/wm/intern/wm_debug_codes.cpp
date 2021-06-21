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

#include "WM_debug_codes.h"

#include <wabi/base/tf/registryManager.h>
#include <wabi/wabi.h>

WABI_NAMESPACE_BEGIN


TF_REGISTRY_FUNCTION(TfDebug)
{
  TF_DEBUG_ENVIRONMENT_SYMBOL(COVAH_DEBUG_MSGBUS, "Show Covah MsgBus messages for debugging purposes");
  TF_DEBUG_ENVIRONMENT_SYMBOL(COVAH_DEBUG_OPERATORS, "Show Covah Operator messages for debugging purposes");
}


WABI_NAMESPACE_END