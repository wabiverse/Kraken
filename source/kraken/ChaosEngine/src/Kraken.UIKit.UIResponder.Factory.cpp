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
 * Kraken.UIKit.UIResponder.Factory
 * KrakenRT.
 */

#include "pch.h"

#include "ChaosEngine/Kraken.UIKit.UIEvent.h"
#include "ChaosEngine/Kraken.UIKit.UIResponder.h"
#include "ChaosEngine/Kraken.UIKit.UIView.h"

/**
 * The Kraken::UIKit::UIResponder is a singleton instance.
 * All core functionality for the UIResponder is located
 * within this file -- As it is necessary for the static
 * methods to be implemented on the instance class, and
 * have those forward to the activation factory. */

namespace winrt::Kraken::UIKit::factory_implementation
{
  /**
   * Signal Handlers. */

  winrt::event_token UIResponder::SignalTitleBarUpdateLayout(Kraken::UIKit::SignalDelegate const& handler)
  {
    return m_signal.add(handler);
  }

  winrt::event_token UIResponder::SignalTitleBarIsVisible(Kraken::UIKit::SignalDelegate const& handler)
  {
    return m_signal.add(handler);
  }

  winrt::event_token UIResponder::SignalNavigationIsDisplaying(Kraken::UIKit::SignalDelegate const& handler)
  {
    return m_signal.add(handler);
  }

  winrt::event_token UIResponder::SignalWindowIsActivated(Kraken::UIKit::SignalDelegate const& handler)
  {
    return m_signal.add(handler);
  }

  /**
   * Cookie Eaters. */

  void UIResponder::SignalTitleBarUpdateLayout(winrt::event_token const& cookie)
  {
    m_signal.remove(cookie);
  }

  void UIResponder::SignalTitleBarIsVisible(winrt::event_token const& cookie)
  {
    m_signal.remove(cookie);
  }

  void UIResponder::SignalNavigationIsDisplaying(winrt::event_token const& cookie)
  {
    m_signal.remove(cookie);
  }

  void UIResponder::SignalWindowIsActivated(winrt::event_token const& cookie)
  {
    m_signal.remove(cookie);
  }

  /**
   * Core functionality. Signaling upon completion. */

  void UIResponder::SetTitleBarLayout(Border const& layout)
  {
    auto coreTitleBar = Windows::ApplicationModel::Core::CoreApplication::GetCurrentView().TitleBar();

    layout.Height(coreTitleBar.Height());

    Windows::UI::Xaml::Thickness currMargin = layout.Margin();
    Windows::UI::Xaml::Thickness margin = {};
    margin.Left = currMargin.Left;
    margin.Top = currMargin.Top;
    margin.Right = coreTitleBar.SystemOverlayRightInset();
    margin.Bottom = currMargin.Bottom;

    layout.Margin(margin);

    m_signal();
  }

  void UIResponder::SetTitleBarIsVisible(CoreApplicationViewTitleBar const& titleBar)
  {
    if (titleBar.IsVisible()) {
      auto args = winrt::make_self<winrt::Kraken::UIKit::implementation::UIEvent>(Windows::UI::Xaml::Visibility::Visible);
      m_titleBarIsVisibleEvent(*this, *args);
    } else {
      auto args = winrt::make_self<winrt::Kraken::UIKit::implementation::UIEvent>(Windows::UI::Xaml::Visibility::Collapsed);
      m_titleBarIsVisibleEvent(*this, *args);
    }

    m_signal();
  }

  void UIResponder::SetNavigationView(NavigationView const& view)
  {
    auto args = winrt::make_self<winrt::Kraken::UIKit::implementation::UIEvent>(view);
    m_setNavigationViewEvent(*this, *args);

    m_signal();
  }

  void UIResponder::SetWindowActivated(bool activate)
  {
    auto args = winrt::make_self<winrt::Kraken::UIKit::implementation::UIEvent>(WindowIsActivated());
    m_windowActivatedEvent(*this, *args);

    m_signal();
  }
} // namespace winrt::Kraken::UIKit::factory_implementation