/*
 * KrakenOS/Kraken.OS.System.hpp
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

#ifndef KRAKEN_OS_SYSTEM_HPP
#define KRAKEN_OS_SYSTEM_HPP

// -------------------------------------------------------------------------------------------------------------------------------------------------------------

#include "Kraken.OS.Defines.hpp"
#include "Kraken.OS.HeaderBridge.hpp"
#include "Kraken.OS.Private.hpp"

#ifdef __APPLE__
#  include <Foundation/Foundation.hpp>
#endif /* __APPLE__ */

#include "Kraken.OS.System.hpp"

// -------------------------------------------------------------------------------------------------------------------------------------------------------------

namespace KRKN
{
  class System : public NS::Referencing<System>
  {
   public:

    static class System *alloc();

    class System *init();

    bool processEvents() const;
  };

  KRKN::System *CreateSystem();
}  // namespace KRKN

// -------------------------------------------------------------------------------------------------------------------------------------------------------------

/** static method: alloc */
KRKN_INLINE KRKN::System *KRKN::System::alloc()
{
  return NS::Object::alloc<KRKN::System>(KRKN_PRIVATE_CLS(KRKNSystem));
}

/** method: init */
KRKN_INLINE KRKN::System *KRKN::System::init()
{
  return NS::Object::init<KRKN::System>();
}

/** method: processEvents */
KRKN_INLINE bool KRKN::System::processEvents() const
{
  return Object::sendMessage<bool>(this, KRKN_PRIVATE_SEL(processEvents));
}

#if defined(KRKN_PRIVATE_IMPLEMENTATION)

extern "C" KRKN::System *KRKNCreateSystem();

_NS_EXPORT KRKN::System *KRKN::CreateSystem()
{
  return ::KRKNCreateSystem();
}

#endif /* KRKN_PRIVATE_IMPLEMENTATION */

// -------------------------------------------------------------------------------------------------------------------------------------------------------------

#endif /* KRAKEN_OS_SYSTEM_HPP */
