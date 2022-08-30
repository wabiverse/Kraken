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
#include "wabi/imaging/hgiMetal/capabilities.h"
#include "wabi/imaging/hgiMetal/hgi.h"

#include "wabi/base/arch/defines.h"
#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

WABI_NAMESPACE_BEGIN

HgiMetalCapabilities::HgiMetalCapabilities(MTL::Device *device)
{
  _SetFlag(HgiDeviceCapabilitiesBitsConcurrentDispatch, true);

  defaultStorageMode = MTL::ResourceStorageModeShared;
  bool unifiedMemory = false;
  bool barycentrics = false;
  bool hasAppleSilicon = false;
#if defined(ARCH_OS_IOS) || (defined(__MAC_10_15) && __MAC_OS_X_VERSION_MAX_ALLOWED >= __MAC_10_15)
  unifiedMemory = device->hasUnifiedMemory();
#else
  unifiedMemory = device->lowPower();
#endif
  // On macOS 10.15 and 11.0 the AMD drivers reported the wrong value for
  // supportsShaderBarycentricCoordinates so check both flags.
  barycentrics = device->supportsShaderBarycentricCoordinates() ||
                 device->barycentricCoordsSupported();

  hasAppleSilicon = device->hasUnifiedMemory() && !device->lowPower();

  _SetFlag(HgiDeviceCapabilitiesBitsUnifiedMemory, unifiedMemory);

  _SetFlag(HgiDeviceCapabilitiesBitsBuiltinBarycentrics, barycentrics);

  _SetFlag(HgiDeviceCapabilitiesBitsShaderDoublePrecision, false);

  _SetFlag(HgiDeviceCapabilitiesBitsDepthRangeMinusOnetoOne, false);

  _SetFlag(HgiDeviceCapabilitiesBitsCppShaderPadding, true);

  _SetFlag(HgiDeviceCapabilitiesBitsMetalTessellation, true);

  _SetFlag(HgiDeviceCapabilitiesBitsMultiDrawIndirect, true);

  // This is done to decide whether to use a workaround for post tess
  // patch primitive ID lookup. The bug causes the firstPatch offset
  // to be included incorrectly in the primitive ID. Our workaround
  // is to subtract it based on the base primitive offset
  // Found in MacOS 13. If confirmed fixed for MacOS 14, add a check
  // if we are on MacOS 14 or less
  // bool isMacOs13OrLess = NSProcessInfo.processInfo.operatingSystemVersion.majorVersion <= 13
  // bool requireBasePrimitiveOffset = hasAppleSilicon && isMacOs13OrLess;
  bool requiresBasePrimitiveOffset = hasAppleSilicon;
  _SetFlag(HgiDeviceCapabilitiesBasePrimitiveOffset, requiresBasePrimitiveOffset);

  if (!unifiedMemory) {
    defaultStorageMode = MTL::ResourceStorageModeManaged;
  }

  _maxUniformBlockSize = 64 * 1024;
  _maxShaderStorageBlockSize = 1 * 1024 * 1024 * 1024;
  _uniformBufferOffsetAlignment = 16;
  _maxClipDistances = 8;
  _pageSizeAlignment = 4096;

  // Apple Silicon only support memory barriers between vertex stages after
  // macOS 12.3.
  hasVertexMemoryBarrier = !hasAppleSilicon;

#if defined(__MAC_12_3) && __MAC_OS_X_VERSION_MAX_ALLOWED >= __MAC_12_3
  if (hasAppleSilicon) {
    hasVertexMemoryBarrier = true;
  }
#endif /* defined(__MAC_12_3) && __MAC_OS_X_VERSION_MAX_ALLOWED >= __MAC_12_3 */

  // Vega GPUs require a fix to the indirect draw before macOS 12.2
  requiresIndirectDrawFix = false;

#if defined(__MAC_12_2) && __MAC_OS_X_VERSION_MAX_ALLOWED <= __MAC_12_2
  if (device->name()
        ->rangeOfString(NS::String::string("Vega", NS::UTF8StringEncoding),
                        NS::CaseInsensitiveSearch)
        .location != NS::NotFound) {
    requiresIndirectDrawFix = true;
  }
#endif
  useParallelEncoder = true;
}

HgiMetalCapabilities::~HgiMetalCapabilities() = default;

int HgiMetalCapabilities::GetAPIVersion() const
{
#ifdef ARCH_OS_MACOS
  if (NS::ProcessInfo::processInfo()->isOperatingSystemAtLeastVersion(
        NS::OperatingSystemVersion{10, 15, 0})) {
    return APIVersion_Metal3_0;
  }

  if (NS::ProcessInfo::processInfo()->isOperatingSystemAtLeastVersion(
        NS::OperatingSystemVersion{10, 13, 0})) {
    return APIVersion_Metal2_0;
  }
#elif defined(ARCH_OS_IOS)
  if (NS::ProcessInfo::processInfo()->isOperatingSystemAtLeastVersion(
        NS::OperatingSystemVersion{13, 0, 0})) {
    return APIVersion_Metal3_0;
  }

  if (NS::ProcessInfo::processInfo()->isOperatingSystemAtLeastVersion(
        NS::OperatingSystemVersion{11, 0, 0})) {
    return APIVersion_Metal2_0;
  }
#endif /* ARCH_OS_MACOS */

  return APIVersion_Metal1_0;
}

int HgiMetalCapabilities::GetShaderVersion() const
{
  // Note: This is not the Metal Shader Language version. It is provided for
  // compatibility with code that is asking for the GLSL version.
  return 450;
}

WABI_NAMESPACE_END
