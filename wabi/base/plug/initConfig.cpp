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

#include "wabi/base/arch/attributes.h"
#include "wabi/base/arch/fileSystem.h"
#include "wabi/base/arch/symbols.h"
#include "wabi/base/arch/systemInfo.h"
#include "wabi/base/plug/info.h"
#include "wabi/base/tf/diagnosticLite.h"
#include "wabi/base/tf/getenv.h"
#include "wabi/base/tf/pathUtils.h"
#include "wabi/base/tf/preprocessorUtilsLite.h"
#include "wabi/base/tf/stringUtils.h"
#include "wabi/wabi.h"

WABI_NAMESPACE_BEGIN

namespace
{

  const char *pathEnvVarName = TF_PP_STRINGIZE(WABI_PLUGINPATH_NAME);
  const char *buildLocation = TF_PP_STRINGIZE(WABI_BUILD_LOCATION);
  const char *pluginBuildLocation = TF_PP_STRINGIZE(WABI_PLUGIN_BUILD_LOCATION);

#ifdef WABI_INSTALL_LOCATION
  const char *installLocation = TF_PP_STRINGIZE(WABI_INSTALL_LOCATION);
#endif  // WABI_INSTALL_LOCATION

  void _AppendPathList(std::vector<std::string> *result,
                       const std::string &paths,
                       const std::string &sharedLibPath)
  {
    for (const auto &path : TfStringSplit(paths, ARCH_PATH_LIST_SEP))
    {
      if (path.empty())
      {
        continue;
      }

      // Anchor all relative paths to the shared library path.
      const bool isLibraryRelativePath = TfIsRelativePath(path);
      if (isLibraryRelativePath)
      {
        result->push_back(TfStringCatPaths(sharedLibPath, path));
      } else
      {
        result->push_back(path);
      }
    }
  }

  ARCH_CONSTRUCTOR(Plug_InitConfig, 2, void)
  {
    std::vector<std::string> result;

    std::vector<std::string> debugMessages;

    // Determine the absolute path to the Plug shared library.  Any relative
    // paths specified in the plugin search path will be anchored to this
    // directory, to allow for relocatability.  Note that this can fail when wabi
    // is built as a static library.  In that case, fall back to using
    // ArchGetExecutablePath().  Also provide some diagnostic output if the
    // PLUG_INFO_SEARCH debug flag is enabled.
    std::string binaryPath;
    if (!ArchGetAddressInfo(reinterpret_cast<void *>(&Plug_InitConfig),
                            &binaryPath,
                            nullptr,
                            nullptr,
                            nullptr))
    {
      debugMessages.emplace_back(
        "Failed to determine absolute path for Plug search "
        "using using ArchGetAddressInfo().  This is expected "
        "if wabi is linked as a static library.\n");
    }

    if (binaryPath.empty())
    {
      debugMessages.emplace_back(
        "Using ArchGetExecutablePath() to determine absolute "
        "path for Plug search location.\n");
      binaryPath = ArchGetExecutablePath();
    }

    binaryPath = TfGetPathName(binaryPath);

    debugMessages.emplace_back(
      TfStringPrintf("Plug will search for plug infos under '%s'\n", binaryPath.c_str()));

    // Environment locations.
    _AppendPathList(&result, TfGetenv(pathEnvVarName), binaryPath);

    // Fallback locations.
    _AppendPathList(&result, buildLocation, binaryPath);
    _AppendPathList(&result, pluginBuildLocation, binaryPath);

#ifdef WABI_INSTALL_LOCATION
    _AppendPathList(&result, installLocation, binaryPath);
#endif  // WABI_INSTALL_LOCATION

    // Plugin registration must process these paths in order
    // to ensure deterministic behavior when the same plugin
    // exists in different paths.  The first path containing
    // a particular plug-in will "win".
    const bool pathsAreOrdered = true;
    Plug_SetPaths(result, debugMessages, pathsAreOrdered);
  }

}  // namespace

WABI_NAMESPACE_END
