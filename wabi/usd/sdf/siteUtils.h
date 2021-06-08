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
#ifndef WABI_USD_SDF_SITE_UTILS_H
#define WABI_USD_SDF_SITE_UTILS_H

/// \file sdf/siteUtils.h
///
/// Convenience API for working with SdfSite.
///
/// These functions simply forward to the indicated functions on SdfLayer.

#include "wabi/usd/sdf/layer.h"
#include "wabi/usd/sdf/primSpec.h"
#include "wabi/usd/sdf/propertySpec.h"
#include "wabi/usd/sdf/site.h"
#include "wabi/usd/sdf/spec.h"
#include "wabi/wabi.h"

WABI_NAMESPACE_BEGIN

inline SdfSpecHandle SdfGetObjectAtPath(const SdfSite &site)
{
  return site.layer->GetObjectAtPath(site.path);
}

inline SdfPrimSpecHandle SdfGetPrimAtPath(const SdfSite &site)
{
  return site.layer->GetPrimAtPath(site.path);
}

inline SdfPropertySpecHandle SdfGetPropertyAtPath(const SdfSite &site)
{
  return site.layer->GetPropertyAtPath(site.path);
}

inline bool SdfHasField(const SdfSite &site, const TfToken &field)
{
  return site.layer->HasField(site.path, field);
}

template<class T> inline bool SdfHasField(const SdfSite &site, const TfToken &field, T *value)
{
  return site.layer->HasField(site.path, field, value);
}

inline const VtValue SdfGetField(const SdfSite &site, const TfToken &field)
{
  return site.layer->GetField(site.path, field);
}

template<class T>
inline T SdfGetFieldAs(const SdfSite &site, const TfToken &field, const T &defaultValue = T())
{
  return site.layer->GetFieldAs<T>(site.path, field, defaultValue);
}

WABI_NAMESPACE_END

#endif  // WABI_USD_SDF_SITE_UTILS_H
