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

#include "wabi/base/arch/demangle.h"
#include "wabi/base/tf/diagnosticLite.h"
#include "wabi/base/tf/stringUtils.h"
#include "wabi/usd/ar/resolverContext.h"
#include "wabi/wabi.h"

#include <functional>
#include <typeindex>

WABI_NAMESPACE_BEGIN

ArResolverContext::_Untyped::~_Untyped()
{}

ArResolverContext::ArResolverContext(const std::vector<ArResolverContext> &ctxs)
{
  for (const ArResolverContext &ctx : ctxs) {
    _Add(ctx);
  }
}

void ArResolverContext::_Add(const ArResolverContext &ctx)
{
  for (const auto &obj : ctx._contexts) {
    _Add(std::shared_ptr<_Untyped>(obj->Clone()));
  }
}

bool ArResolverContext::operator==(const ArResolverContext &rhs) const
{
  if (_contexts.size() != rhs._contexts.size()) {
    return false;
  }

  for (size_t i = 0; i < _contexts.size(); ++i) {
    const auto &lhsContext = _contexts[i];
    const auto &rhsContext = rhs._contexts[i];
    if (!lhsContext->IsHolding(rhsContext->GetTypeid()) || !lhsContext->Equals(*rhsContext)) {
      return false;
    }
  }

  return true;
}

bool ArResolverContext::operator<(const ArResolverContext &rhs) const
{
  if (_contexts.size() < rhs._contexts.size()) {
    return true;
  }
  else if (_contexts.size() > rhs._contexts.size()) {
    return false;
  }

  for (size_t i = 0; i < _contexts.size(); ++i) {
    const auto &lhsContext = _contexts[i];
    const auto &rhsContext = rhs._contexts[i];
    if (lhsContext->IsHolding(rhsContext->GetTypeid())) {
      if (lhsContext->LessThan(*rhsContext)) {
        return true;
      }
      else if (!lhsContext->Equals(*rhsContext)) {
        return false;
      }
    }
    else {
      return std::type_index(lhsContext->GetTypeid()) < std::type_index(rhsContext->GetTypeid());
    }
  }

  return false;
}

void ArResolverContext::_Add(std::shared_ptr<_Untyped> &&context)
{
  auto insertIt = std::lower_bound(
      _contexts.begin(),
      _contexts.end(),
      context,
      [](const std::shared_ptr<_Untyped> &a, const std::shared_ptr<_Untyped> &b) {
        return std::type_index(a->GetTypeid()) < std::type_index(b->GetTypeid());
      });

  if (insertIt != _contexts.end() && (*insertIt)->IsHolding(context->GetTypeid())) {
    return;
  }

  _contexts.insert(insertIt, std::move(context));
}

std::string ArResolverContext::GetDebugString() const
{
  std::string s;
  for (const auto &context : _contexts) {
    s += context->GetDebugString();
    s += "\n";
  }
  return s;
}

std::string Ar_GetDebugString(const std::type_info &info, void const *context)
{
  return TfStringPrintf("<'%s' @ %p>", ArchGetDemangled(info).c_str(), context);
}

WABI_NAMESPACE_END
