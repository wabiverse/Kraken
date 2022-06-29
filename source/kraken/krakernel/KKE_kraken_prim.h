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

#pragma once

/**
 * @file
 * KRAKEN Kernel.
 * Purple Underground.
 */

#include "KKE_api.h"
#include "KKE_context.h"
#include "KKE_robinhood.h"

#include "USD_object.h"

#include <wabi/usd/usd/prim.h>
#include <wabi/usd/usd/stage.h>
#include <wabi/usd/usd/typed.h>

#include <wabi/base/js/value.h>
#include <wabi/base/plug/notice.h>
#include <wabi/base/plug/plugin.h>
#include <wabi/base/plug/registry.h>
#include <wabi/base/tf/instantiateSingleton.h>
#include <wabi/base/tf/singleton.h>
#include <wabi/base/tf/weakBase.h>

#include <memory>
#include <tbb/queuing_rw_mutex.h>
#include <unordered_map>

WABI_NAMESPACE_BEGIN


WABI_NAMESPACE_END