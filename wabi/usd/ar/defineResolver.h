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
#ifndef WABI_USD_AR_DEFINE_RESOLVER_H
#define WABI_USD_AR_DEFINE_RESOLVER_H

/// \file ar/defineResolver.h
/// Macros for defining a custom resolver implementation.

#include "wabi/usd/ar/api.h"
#include "wabi/usd/ar/resolver.h"
#include "wabi/wabi.h"

#include "wabi/base/tf/registryManager.h"
#include "wabi/base/tf/type.h"

WABI_NAMESPACE_BEGIN

/// \def AR_DEFINE_RESOLVER
///
/// Performs registrations required for the specified resolver class
/// to be discovered by Ar's plugin mechanism. This typically would be
/// invoked in the source file defining the resolver class. For example:
///
/// \code
/// // in .cpp file
/// AR_DEFINE_RESOLVER(CustomResolverClass, ArResolver);
/// \endcode
#ifdef doxygen
#  define AR_DEFINE_RESOLVER(ResolverClass, BaseClass1, ...)
#else
#  define AR_DEFINE_RESOLVER(...) \
    TF_REGISTRY_FUNCTION(TfType) \
    { \
      Ar_DefineResolver<__VA_ARGS__>(); \
    }
#endif  // doxygen

class Ar_ResolverFactoryBase : public TfType::FactoryBase {
 public:
  AR_API
  virtual ArResolver *New() const = 0;
};

template<class T> class Ar_ResolverFactory : public Ar_ResolverFactoryBase {
 public:
  virtual ArResolver *New() const override
  {
    return new T;
  }
};

template<class Resolver, class... Bases> void Ar_DefineResolver()
{
  TfType::Define<Resolver, TfType::Bases<Bases...>>()
      .template SetFactory<Ar_ResolverFactory<Resolver>>();
}

WABI_NAMESPACE_END

#endif  // WABI_USD_AR_DEFINE_RESOLVER_H
