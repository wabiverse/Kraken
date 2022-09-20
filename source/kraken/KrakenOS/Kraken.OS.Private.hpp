/*
 * KrakenOS/Kraken.OS.Private.hpp
 *
 * Copyright 2022, Wabi Animation Studios, Ltd. Co.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KRAKEN_OS_PRIVATE_HPP
#define KRAKEN_OS_PRIVATE_HPP

// -------------------------------------------------------------------------------------------------------------------------------------------------------------

#include "Kraken.OS.Defines.hpp"

#ifdef __APPLE__
#  include <objc/runtime.h>
#endif /* __APPLE__ */

// -------------------------------------------------------------------------------------------------------------------------------------------------------------

#define KRKN_PRIVATE_CLS(symbol) (Private::Class::s_k##symbol)
#define KRKN_PRIVATE_SEL(accessor) (Private::Selector::s_k##accessor)

// -------------------------------------------------------------------------------------------------------------------------------------------------------------

#if defined(KRKN_PRIVATE_IMPLEMENTATION)

#  ifdef KRAKENCPP_SYMBOL_VISIBILITY_HIDDEN
#    define _KRKN_PRIVATE_VISIBILITY __attribute__((visibility("hidden")))
#  else
#    define _KRKN_PRIVATE_VISIBILITY __attribute__((visibility("default")))
#  endif /* KRAKENCPP_SYMBOL_VISIBILITY_HIDDEN */

#  define _KRKN_PRIVATE_IMPORT __attribute__((weak_import))

#  ifdef __OBJC__
#    define _KRKN_PRIVATE_OBJC_LOOKUP_CLASS(symbol) ((__bridge void *)objc_lookUpClass(#    symbol))
#    define _KRKN_PRIVATE_OBJC_GET_PROTOCOL(symbol) ((__bridge void *)objc_getProtocol(#    symbol))
#  else
#    define _KRKN_PRIVATE_OBJC_LOOKUP_CLASS(symbol) objc_lookUpClass(#    symbol)
#    define _KRKN_PRIVATE_OBJC_GET_PROTOCOL(symbol) objc_getProtocol(#    symbol)
#  endif  // __OBJC__

#  define KRKN_PRIVATE_DEF_CLS(symbol) \
    void *s_k##symbol _KRKN_PRIVATE_VISIBILITY = _KRKN_PRIVATE_OBJC_LOOKUP_CLASS(symbol)
#  define KRKN_PRIVATE_DEF_PRO(symbol) \
    void *s_k##symbol _KRKN_PRIVATE_VISIBILITY = _KRKN_PRIVATE_OBJC_GET_PROTOCOL(symbol)
#  define KRKN_PRIVATE_DEF_SEL(accessor, symbol) \
    SEL s_k##accessor _KRKN_PRIVATE_VISIBILITY = sel_registerName(symbol)

#  include <dlfcn.h>
#  define KRKN_DEF_FUNC(name, signature) \
    using Fn##name = signature;          \
    Fn##name name = reinterpret_cast<Fn##name>(dlsym(RTLD_DEFAULT, #name))

#  if defined(__MAC_10_16) || defined(__MAC_11_0) || defined(__MAC_12_0) ||    \
    defined(__MAC_13_0) || defined(__IPHONE_14_0) || defined(__IPHONE_15_0) || \
    defined(__IPHONE_16_0) || defined(__TVOS_14_0) || defined(__TVOS_15_0) ||  \
    defined(__TVOS_16_0)

#    define KRKN_PRIVATE_DEF_STR(type, symbol)                  \
      KRKN_EXTERN type const KRKN##symbol _KRKN_PRIVATE_IMPORT; \
      type const KRKN::symbol = (nullptr != &KRKN##symbol) ? KRKN##symbol : nullptr

#  else

namespace KRKN
{
  namespace Private
  {

    template<typename _Type> inline _Type const LoadSymbol(const char *pSymbol)
    {
      const _Type *pAddress = static_cast<_Type *>(dlsym(RTLD_DEFAULT, pSymbol));

      return pAddress ? *pAddress : nullptr;
    }

  }  // namespace Private
}  // namespace KRKN

#    define KRKN_PRIVATE_DEF_STR(type, symbol) \
      KRKN_EXTERN type const KRKN##symbol;     \
      type const KRKN::symbol = Private::LoadSymbol<type>("KRKN" #symbol)

#  endif  // defined(__MAC_10_16) || defined(__MAC_11_0) || defined(__MAC_12_0) ||
          // defined(__MAC_13_0) || defined(__IPHONE_14_0) || defined(__IPHONE_15_0) ||
          // defined(__IPHONE_16_0) || defined(__TVOS_14_0) || defined(__TVOS_15_0) ||
          // defined(__TVOS_16_0)

#else /* KRKN_PRIVATE_IMPLEMENTATION */

#  define KRKN_PRIVATE_DEF_CLS(symbol) extern void *s_k##symbol
#  define KRKN_PRIVATE_DEF_PRO(symbol) extern void *s_k##symbol
#  define KRKN_PRIVATE_DEF_SEL(accessor, symbol) extern SEL s_k##accessor
#  define KRKN_PRIVATE_DEF_STR(type, symbol) extern type const KRKN::symbol

#endif /* KRKN_PRIVATE_IMPLEMENTATION */

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

namespace KRKN
{
  namespace Private
  {
    namespace Class
    {}  // namespace Class
  }     // namespace Private
}  // namespace KRKN

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

namespace KRKN
{
  namespace Private
  {
    namespace Protocol
    {}  // namespace Protocol
  }     // namespace Private
}  // namespace KRKN

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

namespace KRKN
{
  namespace Private
  {
    namespace Selector
    {

      KRKN_PRIVATE_DEF_SEL(beginScope, "beginScope");
      KRKN_PRIVATE_DEF_SEL(endScope, "endScope");
    }  // namespace Selector
  }    // namespace Private
}  // namespace KRKN

//-------------------------------------------------------------------------------------------------------------------------------------------------------------

#endif /* KRAKEN_OS_PRIVATE_HPP */
