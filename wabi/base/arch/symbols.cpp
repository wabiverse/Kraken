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

#include "wabi/base/arch/symbols.h"
#include "wabi/base/arch/defines.h"
#include "wabi/base/arch/fileSystem.h"
#include "wabi/wabi.h"
#if defined(ARCH_OS_LINUX)
#  include <dlfcn.h>
#elif defined(ARCH_OS_DARWIN)
#  include <dlfcn.h>
#elif defined(ARCH_OS_WINDOWS)
#  include <Windows.h>
#  include <stdio.h>
#  include <dbghelp.h>
#  include <shlwapi.h>
#  include <tlhelp32.h>
#  include <DbgHelp.h>
#  include <Psapi.h>
#  include <Windows.h>
#endif

WABI_NAMESPACE_BEGIN

bool ArchGetAddressInfo(void *address,
                        std::string *objectPath,
                        void **baseAddress,
                        std::string *symbolName,
                        void **symbolAddress)
{
#if defined(_GNU_SOURCE) || defined(ARCH_OS_DARWIN)

  Dl_info info;
  if (dladdr(address, &info))
  {
    if (objectPath)
    {
      // The object filename may be a relative path if, for instance,
      // the given address comes from an executable that was invoked
      // with a relative path, or from a shared library that was
      // dlopen'd with a relative path. We want to always return
      // absolute paths, so do the resolution here.
      //
      // This may be incorrect if the current working directory was
      // changed after the source object was loaded.
      *objectPath = ArchAbsPath(info.dli_fname);
    }
    if (baseAddress)
    {
      *baseAddress = info.dli_fbase;
    }
    if (symbolName)
    {
      *symbolName = info.dli_sname ? info.dli_sname : "";
    }
    if (symbolAddress)
    {
      *symbolAddress = info.dli_saddr;
    }
    return true;
  }
  return false;

#elif defined(ARCH_OS_WINDOWS)

  if (!address)
  {
    return false;
  }

  HMODULE module = nullptr;
  if (!::GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                             GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                           reinterpret_cast<LPCWSTR>(address),
                           &module))
  {
    return false;
  }

  if (objectPath)
  {
    char modName[MAX_PATH] = {0};
    if (GetModuleFileName(module, (LPWSTR)modName, MAX_PATH))
    {
      objectPath->assign(modName);
    }
  }

  if (baseAddress || symbolName || symbolAddress)
  {
    HANDLE process = GetCurrentProcess();

  /**
   * Doesn't work on UWP */
#ifndef WINAPI_PARTITION_DESKTOP
    DWORD displacement;
    SymInitialize(process, NULL, BOOL(TRUE));

    // Symbol
    ULONG64 symBuffer[(sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR) + sizeof(ULONG64) - 1) /
                       sizeof(ULONG64)];
    SYMBOL_INFO *sym = (SYMBOL_INFO *)symBuffer;
    sym->MaxNameLen = MAX_SYM_NAME;
    sym->SizeOfStruct = sizeof(SYMBOL_INFO);

    // Line
    IMAGEHLP_LINE64 line = {0};
    line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

    DWORD64 dwAddress = (DWORD64)address;
    SymFromAddr(process, dwAddress, NULL, sym);
    if (!SymGetLineFromAddr64(process, dwAddress, &displacement, &line))
    {
      return false;
    }

    if (baseAddress)
    {
      MODULEINFO moduleInfo = {0};
      if (!GetModuleInformation(process, module, &moduleInfo, sizeof(moduleInfo)))
      {
        return false;
      }
      *baseAddress = moduleInfo.lpBaseOfDll;
    }

    if (symbolName)
    {
      *symbolName = sym->Name ? sym->Name : "";
    }

    if (symbolAddress)
    {
      *symbolAddress = (void *)sym->Address;
    }
#else /* WINAPI_PARTITION_DESKTOP */

    /**
     * We can Retrieve base address on WinRT. */
    if (baseAddress)
    {
      MODULEINFO moduleInfo = {0};
      if (!GetModuleInformation(process, module, &moduleInfo, sizeof(moduleInfo)))
      {
        return false;
      }
      *baseAddress = moduleInfo.lpBaseOfDll;

      /**
       * We can return true, so long as we did
       * not request a symbol name or symbol
       * address -- so we passthrough here.
       * 
       * :: pass :: */
    }

    /**
     * We cannot retrieve symbol name on WinRT. */
    if (symbolName)
    {
      *symbolName = std::string();
    }

    /**
     * We cannot retrieve symbol address on WinRT. */
    if (symbolAddress)
    {
      *symbolAddress = nullptr;
    }

    if (symbolName || symbolAddress)
    {
      return false;
    }

#endif /* WINRT */
  }
  return true;

#else

  return false;

#endif
}

WABI_NAMESPACE_END
