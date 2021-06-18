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
#include "wabi/usd/ar/defaultResolverContext.h"
#include "wabi/wabi.h"

#include "wabi/base/tf/diagnostic.h"
#include "wabi/base/tf/hash.h"
#include "wabi/base/tf/ostreamMethods.h"
#include "wabi/base/tf/pathUtils.h"
#include "wabi/base/tf/stringUtils.h"

#include <boost/functional/hash.hpp>

WABI_NAMESPACE_BEGIN

ArDefaultResolverContext::ArDefaultResolverContext(const std::vector<std::string> &searchPath)
{
  _searchPath.reserve(searchPath.size());
  for (const std::string &p : searchPath)
  {
    if (p.empty())
    {
      continue;
    }

    const std::string absPath = TfAbsPath(p);
    if (absPath.empty())
    {
      TF_WARN(
        "Could not determine absolute path for search path prefix "
        "'%s'",
        p.c_str());
      continue;
    }

    _searchPath.push_back(absPath);
  }
}

bool ArDefaultResolverContext::operator<(const ArDefaultResolverContext &rhs) const
{
  return _searchPath < rhs._searchPath;
}

bool ArDefaultResolverContext::operator==(const ArDefaultResolverContext &rhs) const
{
  return _searchPath == rhs._searchPath;
}

bool ArDefaultResolverContext::operator!=(const ArDefaultResolverContext &rhs) const
{
  return !(*this == rhs);
}

std::string ArDefaultResolverContext::GetAsString() const
{
  std::string result = "Search path: ";
  if (_searchPath.empty())
  {
    result += "[ ]";
  }
  else
  {
    result += "[\n    ";
    result += TfStringJoin(_searchPath, "\n    ");
    result += "\n]";
  }
  return result;
}

size_t hash_value(const ArDefaultResolverContext &context)
{
  size_t hash = 0;
  for (const std::string &p : context.GetSearchPath())
  {
    boost::hash_combine(hash, TfHash()(p));
  }
  return hash;
}

WABI_NAMESPACE_END
