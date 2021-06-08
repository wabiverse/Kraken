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
#ifndef WABI_IMAGING_GARCH_GL_PLATFORM_CONTEXT_WINDOWS_H
#define WABI_IMAGING_GARCH_GL_PLATFORM_CONTEXT_WINDOWS_H

#include "wabi/imaging/garch/api.h"
#include "wabi/wabi.h"
#include <memory>

WABI_NAMESPACE_BEGIN

class GarchWGLContextState {
 public:
  /// Construct with the current state.
  GARCH_API
  GarchWGLContextState();

  enum class NullState { nullstate };

  /// Construct with the null state.
  GARCH_API
  GarchWGLContextState(NullState);

  /// Compare for equality.
  GARCH_API
  bool operator==(const GarchWGLContextState &rhs) const;

  /// Returns a hash value for the state.
  GARCH_API
  size_t GetHash() const;

  /// Returns \c true if the context state is valid.
  GARCH_API
  bool IsValid() const;

  /// Make the context current.
  GARCH_API
  void MakeCurrent();

  /// Make no context current.
  GARCH_API
  static void DoneCurrent();

 private:
  class _Detail;
  std::shared_ptr<_Detail> _detail;
};

// Hide the platform specific type name behind a common name.
typedef GarchWGLContextState GarchGLPlatformContextState;

WABI_NAMESPACE_END

#endif  // WABI_IMAGING_GARCH_GL_PLATFORM_CONTEXT_WINDOWS_H
