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

#include "UNI_object.h"

#include <wabi/usd/usd/prim.h>
#include <wabi/usd/usd/stage.h>
#include <wabi/usd/usd/typed.h>

WABI_NAMESPACE_BEGIN

/**
 * Initialize a new data-block. May be NULL if there is nothing to do. */

typedef bool (*KrakenPrimInitFunction)(KrakenPrim *prim);


/**
 * Register the init function with Kraken. */

template<class PrimType>
inline void RegisterKrakenInitFunction(const KrakenPrimInitFunction &fn)
{
  static_assert(std::is_base_of<KrakenPrim, PrimType>::value, "Prim type must derive from KrakenPrim");

  RegisterKrakenInitFunction(TfType::Find<PrimType>(), fn);
}

void RegisterKrakenInitFunction(const TfType &boundableType, const KrakenPrimInitFunction &fn);

WABI_NAMESPACE_END