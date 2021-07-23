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
/// \file glPlatformContext.cpp

#include "wabi/imaging/garch/glPlatformContext.h"
#include <boost/functional/hash.hpp>

WABI_NAMESPACE_BEGIN

//
// GarchGLXContextState
//

GarchGLXContextState::GarchGLXContextState()
  : display(glXGetCurrentDisplay()),
    drawable(glXGetCurrentDrawable()),
    context(glXGetCurrentContext()),
    _defaultCtor(true)
{
  // Do nothing
}

GarchGLXContextState::GarchGLXContextState(Display *display_, GLXDrawable drawable_, GLXContext context_)
  : display(display_),
    drawable(drawable_),
    context(context_),
    _defaultCtor(false)
{
  // Do nothing
}

bool GarchGLXContextState::operator==(const GarchGLXContextState &rhs) const
{
  return display == rhs.display && drawable == rhs.drawable && context == rhs.context;
}

size_t GarchGLXContextState::GetHash() const
{
  size_t result = 0;
  boost::hash_combine(result, display);
  boost::hash_combine(result, drawable);
  boost::hash_combine(result, context);
  return result;
}

bool GarchGLXContextState::IsValid() const
{
  return display && drawable && context;
}

void GarchGLXContextState::MakeCurrent()
{
  if (IsValid())
  {
    glXMakeCurrent(display, drawable, context);
  } else if (_defaultCtor)
  {
    DoneCurrent();
  }
}

void GarchGLXContextState::DoneCurrent()
{
  if (Display *display = glXGetCurrentDisplay())
  {
    glXMakeCurrent(display, None, NULL);
  }
}

GarchGLPlatformContextState GarchGetNullGLPlatformContextState()
{
  return GarchGLXContextState(NULL, None, NULL);
}

WABI_NAMESPACE_END
