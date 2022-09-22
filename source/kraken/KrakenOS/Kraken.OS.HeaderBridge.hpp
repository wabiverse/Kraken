/*
 * KrakenOS/Kraken.OS.HeaderBridge.hpp
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

#ifndef KRAKEN_OS_HEADER_BRIDGE_HPP
#define KRAKEN_OS_HEADER_BRIDGE_HPP

// -------------------------------------------------------------------------------------------------------------------------------------------------------------

#include "Kraken.OS.Private.hpp"

namespace KRKN::Private::Class
{
  KRKN_PRIVATE_DEF_CLS(KRKNSystem);
  KRKN_PRIVATE_DEF_CLS(KRKNWindow);
}

namespace KRKN::Private::Protocol
{
  // KRKN_PRIVATE_DEF_PRO(...)
}

namespace KRKN::Private::Selector
{
  KRKN_PRIVATE_DEF_SEL(getMainDisplayDimensions, "getMainDisplayDimensions");
  KRKN_PRIVATE_DEF_SEL(processEvents, "processEvents");
  KRKN_PRIVATE_DEF_SEL(title, "title");
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------

#endif /* KRAKEN_OS_HEADER_BRIDGE_HPP */
