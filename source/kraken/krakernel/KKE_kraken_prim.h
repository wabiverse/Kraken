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

/**
 * Initialize a new data-block. May be NULL if there is nothing to do. */

typedef bool (*KrakenPrimInitFunction)(KrakenPrim *prim);


class KrakenPrimRegistry : public TfWeakBase
{
 public:

  KrakenPrimRegistry();

  static KrakenPrimRegistry &GetInstance();
  void RegisterInitFunction(const TfType &schemaType, const KrakenPrimInitFunction &fn);
  KrakenPrimInitFunction GetInitFunction(const UsdPrim &prim);


 private:

  void WaitUntilInitialized();
  std::vector<TfType> GetTypesThatMayHaveRegisteredFunctions(const TfType &type) const;
  bool LoadPluginForType(const TfType &type) const;
  void DidRegisterPlugins(const PlugNotice::DidRegisterPlugins &n);
  bool FindFunctionForType(const TfType &type, KrakenPrimInitFunction *fn) const;


 private:

  using RWMutex = tbb::queuing_rw_mutex;
  mutable RWMutex m_mutex;

  using Registry = std::unordered_map<TfType, KrakenPrimInitFunction, TfHash>;
  Registry m_registry;

  std::atomic<bool> m_initialized;
};

/**
 * Register the init function with Kraken. */

template<class PrimType> inline void RegisterKrakenInitFunction(const KrakenPrimInitFunction &fn)
{
  static_assert(std::is_base_of<KrakenPrim, PrimType>::value,
                "Prim type must derive from KrakenPrim");

  RegisterKrakenInitFunction(TfType::Find<PrimType>(), fn);
}

void RegisterKrakenInitFunction(const TfType &boundableType, const KrakenPrimInitFunction &fn);

WABI_NAMESPACE_END