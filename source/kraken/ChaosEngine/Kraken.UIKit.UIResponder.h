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

#include "pch.h"

#include "UIKit.UIResponder.g.h"


namespace winrt::Kraken::UIKit::implementation
{
  struct UIResponder;
} // namespace winrt::Kraken::UIKit::implementation


namespace winrt::Kraken::UIKit::factory_implementation
{
  struct UIResponder : UIResponderT<UIResponder, Kraken::UIKit::UIResponder, static_lifetime>
  {
    winrt::event_token SignalTitleBarUpdateLayout(Kraken::UIKit::SignalDelegate const& handler);
    winrt::event_token SignalTitleBarIsVisible(Kraken::UIKit::SignalDelegate const& handler);
    winrt::event_token SignalNavigationIsDisplaying(Kraken::UIKit::SignalDelegate const& handler);
    winrt::event_token SignalWindowIsActivated(Kraken::UIKit::SignalDelegate const& handler);

    void SignalTitleBarUpdateLayout(winrt::event_token const& cookie);
    void SignalTitleBarIsVisible(winrt::event_token const& cookie);
    void SignalNavigationIsDisplaying(winrt::event_token const& cookie);
    void SignalWindowIsActivated(winrt::event_token const& cookie);

    void SetTitleBarLayout(winrt::CoreApplicationViewTitleBar const& titleBar);
    void SetTitleBarVisibility(Windows::UI::Xaml::Visibility const& visibility);
    void SetNavigationView(winrt::NavigationView const& view);
    void SetWindowActivated(bool activate); 
   
   private:
    winrt::event<Kraken::UIKit::SignalDelegate> m_signal;
  };
} // namespace winrt::Kraken::UIKit:::factory_implementation


namespace winrt::Kraken::UIKit::implementation
{
  struct UIResponder : UIResponderT<UIResponder>
  {
    UIResponder() = delete;

    static winrt::event_token SignalTitleBarUpdateLayout(Kraken::UIKit::SignalDelegate const& handler);
    static winrt::event_token SignalTitleBarIsVisible(Kraken::UIKit::SignalDelegate const& handler);
    static winrt::event_token SignalNavigationIsDisplaying(Kraken::UIKit::SignalDelegate const& handler);
    static winrt::event_token SignalWindowIsActivated(Kraken::UIKit::SignalDelegate const& handler);

    static void SignalTitleBarUpdateLayout(winrt::event_token const& token);
    static void SignalTitleBarIsVisible(winrt::event_token const& token);
    static void SignalNavigationIsDisplaying(winrt::event_token const& token);
    static void SignalWindowIsActivated(winrt::event_token const& token);

    static void SetTitleBarLayout(winrt::CoreApplicationViewTitleBar const& titleBar);
    static void SetTitleBarVisibility(Windows::UI::Xaml::Visibility const& visibility);
    static void SetNavigationView(winrt::NavigationView const& view);
    static void SetWindowActivated(bool activate);    
  };
}  // namespace winrt::Kraken::UIKit::implementation

