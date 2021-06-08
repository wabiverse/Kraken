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
#include "wabi/base/tf/dl.h"
#include "wabi/base/arch/library.h"
#include "wabi/base/tf/debugCodes.h"
#include "wabi/base/tf/registryManager.h"

#ifdef WABI_PYTHON_SUPPORT_ENABLED
#  include "wabi/base/tf/scriptModuleLoader.h"
#endif  // WABI_PYTHON_SUPPORT_ENABLED

#include "wabi/base/tf/getenv.h"
#include <stdlib.h>
#include <string>

using std::string;

WABI_NAMESPACE_BEGIN

static bool _opening = false, _closing = false;

bool Tf_DlOpenIsActive()
{
  return _opening;
}

bool Tf_DlCloseIsActive()
{
  return _closing;
}

void *TfDlopen(const std::string &filename, int flag, std::string *error, bool loadScriptBindings)
{
  TF_DEBUG(TF_DLOPEN).Msg("TfDlopen: [opening] '%s' (flag=%x)...\n", filename.c_str(), flag);

  // Try to open the dynamic library
  bool state   = _opening;
  _opening     = true;
  void *handle = ArchLibraryOpen(filename.c_str(), flag);
  _opening     = state;

  TF_DEBUG(TF_DLOPEN).Msg("TfDlopen: [opened] '%s' (handle=%p)\n", filename.c_str(), handle);

  std::string err = ArchLibraryError();
  if (!err.empty()) {
    TF_DEBUG(TF_DLOPEN).Msg(
        "TfDlopen: [error on opening] '%s': %s\n", filename.c_str(), err.c_str());
    if (error) {
      *error = std::move(err);
    }
  }
  else {
    if (error) {
      error->clear();
    }
  }

#ifdef WABI_PYTHON_SUPPORT_ENABLED
  // If we successfully opened the shared library, load any script bindings if
  // scripting is initialized.
  if (handle && loadScriptBindings) {
    TfScriptModuleLoader::GetInstance().LoadModules();
  }
#endif  // WABI_PYTHON_SUPPORT_ENABLED

  return handle;
}

int TfDlclose(void *handle)
{
  bool state = _closing;
  _closing   = true;

  TF_DEBUG(TF_DLCLOSE).Msg("TfDlclose: handle = %p\n", handle);

  int status = ArchLibraryClose(handle);

  _closing = state;

  return status;
}

WABI_NAMESPACE_END
