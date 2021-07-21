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
 * KRAKEN Kernel.
 * Purple Underground.
 */

#include "UNI_scene.h"
#include "UNI_object.h"

#include "KKE_kraken_prim.h"
#include "KKE_main.h"
#include "KKE_scene.h"

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
 *  -------------------------------------- The Kraken Prim Registry. ----- */

class KrakenPrimRegistry : public TfWeakBase
{

 public:


  KrakenPrimRegistry()
    : m_initialized(false)
  {
    TfSingleton<KrakenPrimRegistry>::SetInstanceConstructed(*this);
    TfRegistryManager::GetInstance().SubscribeTo<KrakenPrim>();

    m_initialized.store(true, std::memory_order_release);

    TfNotice::Register(TfCreateWeakPtr(this), &KrakenPrimRegistry::DidRegisterPlugins);
  }
 

  static KrakenPrimRegistry &GetInstance()
  {
    return TfSingleton<KrakenPrimRegistry>::GetInstance();
  }


  void RegisterInitFunction(const TfType &schemaType, const KrakenPrimInitFunction &fn)
  {
    bool didInsert = false;
    {
      RWMutex::scoped_lock lock(m_mutex, /* write = */ true);
      didInsert = m_registry.emplace(schemaType, fn).second;
    }

    if (!didInsert)
    {
      TF_MSG_ERROR("UsdGeomComputeExtentFunction already registered for "
                   "prim type '%s'", schemaType.GetTypeName().c_str());
    }
  }


  KrakenPrimInitFunction GetInitFunction(const UsdPrim &prim)
  {
    WaitUntilInitialized();

    const TfType &primSchemaType = prim.GetPrimTypeInfo().GetSchemaType();
    if (!primSchemaType)
    {
      TF_MSG_ERROR("Could not find prim type '%s' for prim %s",
                   prim.GetTypeName().GetText(),
                   UsdDescribe(prim).c_str());
      return nullptr;
    }

    KrakenPrimInitFunction fn = nullptr;
    if (FindFunctionForType(primSchemaType, &fn))
    {
      return fn;
    }

    const std::vector<TfType> primSchemaTypeAndBases = GetTypesThatMayHaveRegisteredFunctions(primSchemaType);

    auto i = primSchemaTypeAndBases.cbegin();
    for (auto e = primSchemaTypeAndBases.cend(); i != e; ++i)
    {
      const TfType &type = *i;
      if (FindFunctionForType(type, &fn))
      {
        break;
      }

      if (LoadPluginForType(type))
      {
        if (FindFunctionForType(type, &fn))
        {
          break;
        }
      }
    }

    RWMutex::scoped_lock lock(m_mutex, /* write = */ true);
    for (auto it = primSchemaTypeAndBases.cbegin(); it != i; ++it)
    {
      m_registry.emplace(*it, fn);
    }

    return fn;
  }


 private:

  void WaitUntilInitialized()
  {
    while (ARCH_UNLIKELY(!m_initialized.load(std::memory_order_acquire)))
    {
      std::this_thread::yield();
    }
  }


  std::vector<TfType> GetTypesThatMayHaveRegisteredFunctions(const TfType &type) const
  {
    std::vector<TfType> result;
    type.GetAllAncestorTypes(&result);

    static const TfType krakenType = TfType::Find<KrakenPrim>();
    result.erase(
      std::remove_if(result.begin(), result.end(), [](const TfType &t) { return !t.IsA(krakenType); }),
      result.end());
    return result;
  }


  bool LoadPluginForType(const TfType &type) const
  {
    PlugRegistry &plugReg = PlugRegistry::GetInstance();

    const JsValue isKrakenBuiltinPrim = plugReg.GetDataFromPluginMetaData(type, "isKrakenBuiltinPrim");
    if (!isKrakenBuiltinPrim.Is<bool>() || !isKrakenBuiltinPrim.Get<bool>())
    {
      return false;
    }

    const PlugPluginPtr pluginForType = plugReg.GetPluginForType(type);
    if (!pluginForType)
    {
      TF_MSG_ERROR("Could not find plugin for '%s'", type.GetTypeName().c_str());
      return false;
    }

    return pluginForType->Load();
  }


  void DidRegisterPlugins(const PlugNotice::DidRegisterPlugins &n)
  {
    RWMutex::scoped_lock lock(m_mutex, /* write = */ true);
    m_registry.clear();
  }


  bool FindFunctionForType(const TfType &type, KrakenPrimInitFunction *fn) const
  {
    RWMutex::scoped_lock lock(m_mutex, /* write = */ false);
    return TfMapLookup(m_registry, type, fn);
  }


 private:

  using RWMutex = tbb::queuing_rw_mutex;
  mutable RWMutex m_mutex;

  using Registry = std::unordered_map<TfType, KrakenPrimInitFunction, TfHash>;
  Registry m_registry;

  std::atomic<bool> m_initialized;
};

/**
 *  -------------------------------------- The Kraken Prim Registry. Initialization. ----- */



TF_INSTANTIATE_SINGLETON(KrakenPrimRegistry);



static bool InitKrakenPrimsFromPlugins(KrakenPrim *prim)
{
  if (!prim)
  {
    TF_MSG_ERROR("Invalid KrakenPrim %s", UsdDescribe(prim->GetPrim()).c_str());
    return false;
  }

  const KrakenPrimInitFunction fn = KrakenPrimRegistry::GetInstance().GetInitFunction(prim->GetPrim());
  return fn && (*fn)(prim);
}


bool KrakenPrim::KrakenInitPrimsFromPlugins(KrakenPrim *prim)
{
  return InitKrakenPrimsFromPlugins(prim);
}


void RegisterKrakenInitFunction(const TfType &primType, const KrakenPrimInitFunction &fn)
{
  if (!primType.IsA<KrakenPrim>())
  {
    TF_MSG_ERROR("Prim type '%s' must derive from KrakenPrim", primType.GetTypeName().c_str());
    return;
  }

  if (!fn)
  {
    TF_CODING_ERROR("Invalid function registered for prim type '%s'", primType.GetTypeName().c_str());
    return;
  }

  KrakenPrimRegistry::GetInstance().RegisterInitFunction(primType, fn);
}


WABI_NAMESPACE_END