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
#ifndef WABI_IMAGING_GLF_UNIFORM_BLOCK_H
#define WABI_IMAGING_GLF_UNIFORM_BLOCK_H

/// \file glf/uniformBlock.h

#include "wabi/base/tf/declarePtrs.h"
#include "wabi/base/tf/refBase.h"
#include "wabi/base/tf/weakBase.h"
#include "wabi/imaging/garch/glApi.h"
#include "wabi/imaging/glf/api.h"
#include "wabi/wabi.h"
#include <string>

WABI_NAMESPACE_BEGIN

TF_DECLARE_WEAK_AND_REF_PTRS(GlfUniformBlock);
TF_DECLARE_WEAK_PTRS(GlfBindingMap);

/// \class GlfUniformBlock
///
/// Manages a GL uniform buffer object.
///
class GlfUniformBlock : public TfRefBase, public TfWeakBase
{
 public:

  /// Returns a new instance.
  GLF_API
  static GlfUniformBlockRefPtr New(char const *label = nullptr);

  GLF_API
  virtual ~GlfUniformBlock();

  /// Binds the uniform buffer using a bindingMap and identifier.
  GLF_API
  void Bind(GlfBindingMapPtr const &bindingMap, std::string const &identifier);

  /// Updates the content of the uniform buffer. If the size
  /// is different, the buffer will be reallocated.
  GLF_API
  void Update(const void *data, int size);

 protected:

  GLF_API
  GlfUniformBlock(char const *label);

 private:

  GLuint _buffer;
  int _size;
  std::string _debugLabel;
};

WABI_NAMESPACE_END

#endif
