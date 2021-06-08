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
#ifndef WABI_IMAGING_GARCH_GL_PLATFORM_DEBUG_CONTEXT_H
#define WABI_IMAGING_GARCH_GL_PLATFORM_DEBUG_CONTEXT_H

#include "wabi/base/tf/declarePtrs.h"
#include "wabi/base/tf/weakBase.h"
#include "wabi/imaging/garch/api.h"
#include "wabi/wabi.h"

#include <memory>

WABI_NAMESPACE_BEGIN

class GarchGLPlatformDebugContextPrivate;

TF_DECLARE_WEAK_AND_REF_PTRS(GarchGLPlatformDebugContext);

/// \class GarchGLPlatformDebugContext
///
/// Platform specific context (e.g. X11/GLX) which supports debug output.
///
class GarchGLPlatformDebugContext : public TfRefBase, public TfWeakBase {
 public:
  static GarchGLPlatformDebugContextRefPtr New(int majorVersion,
                                               int minorVersion,
                                               bool coreProfile,
                                               bool directRenderering)
  {
    return TfCreateRefPtr(new GarchGLPlatformDebugContext(
        majorVersion, minorVersion, coreProfile, directRenderering));
  }

  virtual ~GarchGLPlatformDebugContext();

  GARCH_API
  static bool IsEnabledDebugOutput();

  GARCH_API
  static bool IsEnabledCoreProfile();

  GARCH_API
  void makeCurrent();

  GARCH_API
  void *chooseMacVisual();

 public:
  std::unique_ptr<GarchGLPlatformDebugContextPrivate> _private;
  bool _coreProfile;

 protected:
  GARCH_API
  GarchGLPlatformDebugContext(int majorVersion,
                              int minorVersion,
                              bool coreProfile,
                              bool directRenderering);
};

WABI_NAMESPACE_END

#endif  // WABI_IMAGING_GARCH_GL_PLATFORM_DEBUG_CONTEXT_H
