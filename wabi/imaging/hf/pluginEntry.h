/*
 * Copyright 2021 Pixar. All Rights Reserved.
 *
 * Portions of this file are derived from original work by Pixar
 * distributed with Universal Scene Description, a project of the
 * Academy Software Foundation (ASWF). https://www.aswf.io/
 *
 * Licensed under the Apache License, Version 2.0 (the "Apache License")
 * with the following modification; you may not use this file except in
 * compliance with the Apache License and the following modification:
 * Section 6. Trademarks. is deleted and replaced with:
 *
 * 6. Trademarks. This License does not grant permission to use the trade
 *    names, trademarks, service marks, or product names of the Licensor
 *    and its affiliates, except as required to comply with Section 4(c)
 *    of the License and to reproduce the content of the NOTICE file.
 *
 * You may obtain a copy of the Apache License at:
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the Apache License with the above modification is
 * distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF
 * ANY KIND, either express or implied. See the Apache License for the
 * specific language governing permissions and limitations under the
 * Apache License.
 *
 * Modifications copyright (C) 2020-2021 Wabi.
 */
#ifndef WABI_IMAGING_HF_PLUGIN_ENTRY_H
#define WABI_IMAGING_HF_PLUGIN_ENTRY_H

#include "wabi/base/tf/token.h"
#include "wabi/base/tf/type.h"
#include "wabi/imaging/hf/perfLog.h"
#include "wabi/wabi.h"
#include <functional>
#include <string>

WABI_NAMESPACE_BEGIN

class HfPluginBase;
struct HfPluginDesc;

///
/// Internal class that manages a single plugin.
///
class Hf_PluginEntry final {
 public:
  HF_MALLOC_TAG_NEW("new Hf_PluginEntry");

  ///
  /// Functor that is used to create a plugin.
  /// This is used instead of using TfType::FactoryBase
  /// as that would require exposing the class hierarchy publicly
  /// due to templating and the idea is that this class is _Factory
  /// below are private.
  ///
  typedef std::function<HfPluginBase *()> _PluginFactoryFn;

  ///
  /// Constructors a new plugin entry from information in the
  /// plugins metadata file.  See HfPluginRegistry.
  ///
  Hf_PluginEntry(const TfType &type, const std::string &displayName, int priority);
  ~Hf_PluginEntry();

  ///
  /// For containers, allow moving only (no copying)
  ///
  Hf_PluginEntry(Hf_PluginEntry &&source);
  Hf_PluginEntry &operator=(Hf_PluginEntry &&source);

  ///
  /// Simple Accessors
  ///
  const TfType &GetType() const
  {
    return _type;
  }
  const std::string &GetDisplayName() const
  {
    return _displayName;
  }
  int GetPriority() const
  {
    return _priority;
  }
  HfPluginBase *GetInstance() const
  {
    return _instance;
  }

  ///
  /// Returns the internal name of the plugin that is used by the API's.
  ///
  TfToken GetId() const;

  ///
  /// Fills in a plugin description structure that is used to communicate
  /// with the application information about this plugin.
  ///
  void GetDesc(HfPluginDesc *desc) const;

  ///
  /// Ref counting the instance of the plugin.  Each plugin is only
  /// instantiated once.
  ///
  void IncRefCount();
  void DecRefCount();

  ///
  /// For sorting:
  /// Entries are ordered by priority then alphabetical order of type name
  ///
  bool operator<(const Hf_PluginEntry &other) const;

  ///
  /// Type Factory Creation
  ///
  static void SetFactory(TfType &type, _PluginFactoryFn &func);

 private:
  ///
  /// Factory class used for plugin registration.
  /// Even though this class adds another level of indirection
  /// it's purpose is to abstract away the need to derive the factory
  /// from TfType::FactoryBase, which because of templating was exposing
  /// this class rather than keeping it private.
  ///
  class _Factory final : public TfType::FactoryBase {
   public:
    _Factory(_PluginFactoryFn &func) : _func(func)
    {}

    HfPluginBase *New() const
    {
      return _func();
    }

   private:
    _PluginFactoryFn _func;
  };

  TfType _type;
  std::string _displayName;
  int _priority;
  HfPluginBase *_instance;
  int _refCount;

  ///
  /// Don't allow copying
  ///
  Hf_PluginEntry()                       = delete;
  Hf_PluginEntry(const Hf_PluginEntry &) = delete;
  Hf_PluginEntry &operator=(const Hf_PluginEntry &) = delete;
};

WABI_NAMESPACE_END

#endif  // WABI_IMAGING_HF_PLUGIN_ENTRY_H
