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

#ifndef WABI_BASE_TRACE_CATEGORY_H
#define WABI_BASE_TRACE_CATEGORY_H

/// \file trace/category.h

#include "wabi/wabi.h"

#include "wabi/base/tf/singleton.h"
#include "wabi/base/trace/api.h"
#include "wabi/base/trace/stringHash.h"

#include <cstdint>
#include <map>
#include <string>
#include <vector>

WABI_NAMESPACE_BEGIN

/// Categories that a TraceReporter can use to filter events.
using TraceCategoryId = uint32_t;

///////////////////////////////////////////////////////////////////////////////
///
/// \class TraceCategory
///
/// This singleton class provides a way to mark TraceEvent instances with
/// category Ids which can be used to filter them. This class also provides a
/// way to associate TraceCategoryId values with human readable names.
///
class TraceCategory {
 public:
  /// Computes an id for the given a string literal \p str.
  template<int N> static constexpr TraceCategoryId CreateTraceCategoryId(const char (&str)[N])
  {
    return TraceStringHash::Hash(str);
  }

  /// Default category if none are explicitly specified when creating a
  /// TraceEvent.
  enum : TraceCategoryId { Default = 0 };

  /// Associates the \p id with \p name. These associates are not necessarily
  /// unique.
  TRACE_API void RegisterCategory(TraceCategoryId id, const std::string &name);

  /// Returns all names associated with the \p id.
  TRACE_API std::vector<std::string> GetCategories(TraceCategoryId id) const;

  /// Singleton accessor.
  TRACE_API static TraceCategory &GetInstance();

 private:
  friend class TfSingleton<TraceCategory>;

  TraceCategory();

  // Mapping of ids to names.
  std::multimap<TraceCategoryId, std::string> _idToNames;
};

TRACE_API_TEMPLATE_CLASS(TfSingleton<TraceCategory>);

WABI_NAMESPACE_END

#endif  // WABI_BASE_TRACE_CATEGORY_H
