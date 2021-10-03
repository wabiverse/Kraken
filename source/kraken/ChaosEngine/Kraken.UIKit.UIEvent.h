/*
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
 *
 * Copyright 2021, Wabi.
 */

/**
 * @file
 * Microsoft.
 * KrakenRT.
 */

#pragma once

#include "Kraken.UIKit.UIEvent.g.h"

namespace winrt::Kraken::UIKit::implementation
{
  struct UIEvent : UIEventT<UIEvent>
  {
    UIEvent() = default();
    UIEvent(CoreApplicationViewTitleBar const& titleBar);
    UIEvent(NavigationView const& view);
    UIEvent(IInspectable const& window);

    CoreApplicationViewTitleBar TitleBar();
    NavigationView NavigationView();
    IInspectable Window();

   private:
    CoreApplicationViewTitleBar m_titleBar{ nullptr };
    NavigationView m_navigationView{ nullptr };
    IInspectable m_window{ nullptr };
  };
}  // namespace winrt::Kraken::implementation


namespace winrt::Kraken::UIKit::factory_implementation
{
  struct UIEvent : UIEventT<UIEvent, implementation::UIEvent>
  {};
}  // namespace winrt::Kraken::factory_implementation
