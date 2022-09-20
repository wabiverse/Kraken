/*
 * KrakenOS/Kraken.OS.Window.hpp
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

#ifndef KRAKEN_OS_WINDOW_HPP
#define KRAKEN_OS_WINDOW_HPP

// -------------------------------------------------------------------------------------------------------------------------------------------------------------

#include "Kraken.OS.Defines.hpp"
#include "Kraken.OS.HeaderBridge.hpp"
#include "Kraken.OS.Private.hpp"

#ifdef __APPLE__
#  include <Foundation/Foundation.hpp>
#endif /* __APPLE__ */

#include "Kraken.OS.Window.hpp"

// -------------------------------------------------------------------------------------------------------------------------------------------------------------

namespace KRKN
{
  class Window : public NS::Copying<Window>
  {
   public:

    static class Window *alloc();

    class Window *init();

    NS::String *title() const;
  };

  KRKN::Window *CreateWindow(NS::String *title,
                             CGFloat left,
                             CGFloat top,
                             CGFloat width,
                             CGFloat height,
                             bool dialog,
                             KRKN::Window *parent);
}  // namespace KRKN

// -------------------------------------------------------------------------------------------------------------------------------------------------------------

/** static method: alloc */
KRKN_INLINE KRKN::Window *KRKN::Window::alloc()
{
  return NS::Object::alloc<KRKN::Window>(KRKN_PRIVATE_CLS(KRKNWindow));
}

/** method: init */
KRKN_INLINE KRKN::Window *KRKN::Window::init()
{
  return NS::Object::init<KRKN::Window>();
}

/** property: title */
KRKN_INLINE NS::String *KRKN::Window::title() const
{
  return Object::sendMessage<NS::String*>(this, KRKN_PRIVATE_SEL(title));
}

#if defined(KRKN_PRIVATE_IMPLEMENTATION)

extern "C" KRKN::Window *KRKNCreateWindow(NS::String *title,
                                          CGFloat left,
                                          CGFloat top,
                                          CGFloat width,
                                          CGFloat height,
                                          bool dialog,
                                          KRKN::Window *parent);

_NS_EXPORT KRKN::Window *KRKN::CreateWindow(NS::String *title,
                                            CGFloat left,
                                            CGFloat top,
                                            CGFloat width,
                                            CGFloat height,
                                            bool dialog,
                                            KRKN::Window *parent)
{
  return ::KRKNCreateWindow(title, left, top, width, height, dialog, parent);
}

#endif /* KRKN_PRIVATE_IMPLEMENTATION */

// -------------------------------------------------------------------------------------------------------------------------------------------------------------

#endif /* KRAKEN_OS_WINDOW_HPP */
