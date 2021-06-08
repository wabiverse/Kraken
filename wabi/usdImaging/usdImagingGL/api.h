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
#ifndef WABI_USD_IMAGING_USD_IMAGING_GL_API_H
#define WABI_USD_IMAGING_USD_IMAGING_GL_API_H

#include "wabi/base/arch/export.h"

#if defined(WABI_STATIC)
#  define USDIMAGINGGL_API
#  define USDIMAGINGGL_API_TEMPLATE_CLASS(...)
#  define USDIMAGINGGL_API_TEMPLATE_STRUCT(...)
#  define USDIMAGINGGL_LOCAL
#else
#  if defined(USDIMAGINGGL_EXPORTS)
#    define USDIMAGINGGL_API ARCH_EXPORT
#    define USDIMAGINGGL_API_TEMPLATE_CLASS(...) ARCH_EXPORT_TEMPLATE(class, __VA_ARGS__)
#    define USDIMAGINGGL_API_TEMPLATE_STRUCT(...) ARCH_EXPORT_TEMPLATE(struct, __VA_ARGS__)
#  else
#    define USDIMAGINGGL_API ARCH_IMPORT
#    define USDIMAGINGGL_API_TEMPLATE_CLASS(...) ARCH_IMPORT_TEMPLATE(class, __VA_ARGS__)
#    define USDIMAGINGGL_API_TEMPLATE_STRUCT(...) ARCH_IMPORT_TEMPLATE(struct, __VA_ARGS__)
#  endif
#  define USDIMAGINGGL_LOCAL ARCH_HIDDEN
#endif

#endif  // WABI_USD_IMAGING_USD_IMAGING_GL_API_H
