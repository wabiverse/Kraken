//
// Copyright 2020 Pixar
//
// Licensed under the Apache License, Version 2.0 (the "Apache License")
// with the following modification; you may not use this file except in
// compliance with the Apache License and the following modification to it:
// Section 6. Trademarks. is deleted and replaced with:
//
// 6. Trademarks. This License does not grant permission to use the trade
//    names, trademarks, service marks, or product names of the Licensor
//    and its affiliates, except as required to comply with Section 4(c) of
//    the License and to reproduce the content of the NOTICE file.
//
// You may obtain a copy of the Apache License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the Apache License with the above modification is
// distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied. See the Apache License for the specific
// language governing permissions and limitations under the Apache License.
//
#include "wabi/base/tf/diagnostic.h"
#include "wabi/base/tf/envSetting.h"

#include "wabi/imaging/hgiGL/device.h"
#include "wabi/imaging/hgiGL/diagnostic.h"


WABI_NAMESPACE_BEGIN

HgiGLDevice::HgiGLDevice()
{
  _activeArena = &_defaultArena;
  HgiGLSetupGL4Debug();
}

HgiGLDevice::~HgiGLDevice() {}

void HgiGLDevice::SubmitOps(HgiGLOpsVector const &ops)
{
  for (HgiGLOpsFn const &f : ops) {
    f();
  }
}

void HgiGLDevice::SetCurrentArena(HgiGLContextArenaHandle const &arena)
{
  if (arena) {
    _activeArena = arena.Get();
  } else {
    _activeArena = &_defaultArena;
  }
}

uint32_t HgiGLDevice::AcquireFramebuffer(HgiGraphicsCmdsDesc const &desc, bool resolved)
{
  return _GetArena()->_AcquireFramebuffer(desc, resolved);
}

void HgiGLDevice::GarbageCollect()
{
  return _GetArena()->_GarbageCollect();
}

HgiGLContextArena const *HgiGLDevice::_GetArena() const
{
  return _activeArena;
}
HgiGLContextArena *HgiGLDevice::_GetArena()
{
  return _activeArena;
}

std::ofstream &operator<<(std::ofstream &out, const HgiGLDevice &dev)
{
  out << *dev._GetArena();
  return out;
}

WABI_NAMESPACE_END
