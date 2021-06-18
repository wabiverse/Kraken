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
#ifndef WABI_IMAGING_HD_DRIVER_H
#define WABI_IMAGING_HD_DRIVER_H

#include "wabi/imaging/hd/api.h"
#include "wabi/wabi.h"

#include "wabi/base/tf/token.h"
#include "wabi/base/vt/value.h"

WABI_NAMESPACE_BEGIN

/// HdDriver represents a device object, commonly a render device, that is owned
/// by the application and passed to HdRenderIndex. The RenderIndex passes it to
/// the render delegate and rendering tasks.
/// The application manages the lifetime (destruction) of HdDriver and must
/// ensure it remains valid while Hydra is running.
class HdDriver
{
 public:
  TfToken name;
  VtValue driver;
};

WABI_NAMESPACE_END

#endif
