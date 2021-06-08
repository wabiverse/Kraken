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
#ifndef WABI_IMAGING_HGI_GL_DIAGNOSTIC_H
#define WABI_IMAGING_HGI_GL_DIAGNOSTIC_H

#include "wabi/base/arch/functionLite.h"
#include "wabi/imaging/hgiGL/api.h"
#include "wabi/wabi.h"
#include <string>

WABI_NAMESPACE_BEGIN

/// Posts diagnostic errors for all GL errors in the current context.
/// This macro tags the diagnostic errors with the name of the calling
/// function.
#define HGIGL_POST_PENDING_GL_ERRORS() HgiGLPostPendingGLErrors(__ARCH_PRETTY_FUNCTION__)

/// Returns true if GL debug is enabled
HGIGL_API
bool HgiGLDebugEnabled();

/// Posts diagnostic errors for all GL errors in the current context.
HGIGL_API
void HgiGLPostPendingGLErrors(std::string const &where = std::string());

/// Setup OpenGL 4 debug facilities
HGIGL_API
void HgiGLSetupGL4Debug();

HGIGL_API
bool HgiGLMeetsMinimumRequirements();

/// Calls glObjectLabel making sure the label is not too long.
HGIGL_API
void HgiGLObjectLabel(uint32_t identifier, uint32_t name, const std::string &label);

WABI_NAMESPACE_END

#endif
