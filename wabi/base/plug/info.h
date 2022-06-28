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
#ifndef WABI_BASE_PLUG_INFO_H
#define WABI_BASE_PLUG_INFO_H

#include "wabi/wabi.h"
#include "wabi/base/arch/attributes.h"
#include "wabi/base/js/value.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

WABI_NAMESPACE_BEGIN

class JsValue;

/// Data describing the plugin itself.
class Plug_RegistrationMetadata
{
 public:

  enum Type
  {
    UnknownType,
    LibraryType,
    PythonType,
    ResourceType
  };

  Plug_RegistrationMetadata() : type(UnknownType) {}
  Plug_RegistrationMetadata(const JsValue &,
                            const std::string &valuePathname,
                            const std::string &locationForErrorReporting);

  Type type;
  std::string pluginName;
  std::string pluginPath;
  JsObject plugInfo;
  std::string libraryPath;
  std::string resourcePath;
};

/// A task arena for reading plug info.
class Plug_TaskArena
{
 public:

  class Synchronous
  {};  // For single-threaded debugging.
  Plug_TaskArena();
  Plug_TaskArena(Synchronous);
  ~Plug_TaskArena();

  /// Schedule \p fn to run.
  template<class Fn> void Run(Fn const &fn);

  /// Wait for all scheduled tasks to complete.
  void Wait();

 private:

  class _Impl;
  std::unique_ptr<_Impl> _impl;
};

/// Reads several plugInfo files, recursively loading any included files.
/// \p addPlugin is invoked each time a plugin is found.  The order in
/// which plugins is discovered is undefined.  \p addPlugin is invoked
/// by calling \c Run() on \p taskArena.
///
/// \p addVisitedPath is called each time a plug info file is found;  if it
/// returns \c true then the file is processed, otherwise it is ignored.
/// Clients should return \c true or \c false the first time a given path
/// is passed and \c false all subsequent times.
void Plug_ReadPlugInfo(const std::vector<std::string> &pathnames,
                       bool pathsAreOrdered,
                       const std::function<bool(const std::string &)> &addVisitedPath,
                       const std::function<void(const Plug_RegistrationMetadata &)> &addPlugin,
                       Plug_TaskArena *taskArena);

/// Sets the paths to the bootstrap plugInfo JSON files, also any diagnostic
/// messages that should be reported when plugins are registered (if any).
/// The priority order of elements of the path is honored if pathsAreOrdered.
/// Defined in registry.cpp.
void Plug_SetPaths(const std::vector<std::string> &,
                   const std::vector<std::string> &,
                   bool pathsAreOrdered);

WABI_NAMESPACE_END

#endif  // WABI_BASE_PLUG_INFO_H
