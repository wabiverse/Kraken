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

#include "pch.h"
#include "Kraken/Microsoft/UIEvent.h"
#include "Kraken/Microsoft/UIResponder.h"
#include "Kraken/Microsoft/MainPage.h"
#include "UIResponder.g.cpp"


namespace winrt::Kraken::UIKit:::factory_implementation
{
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


  void UIResponder::SignalTitleBarUpdateLayout(winrt::event_token const& token)
  {
    m_signal.remove(token);
  }

  void UIResponder::SignalTitleBarIsVisible(winrt::event_token const& token)
  {
    m_signal.remove(token);
  }

  void UIResponder::SignalNavigationIsDisplaying(winrt::event_token const& token)
  {
    m_signal.remove(token);
  }

  void UIResponder::SignalWindowIsActivated(winrt::event_token const& token)
  {
    m_signal.remove(token);
  }


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
      auto args = winrt::make_self<winrt::kraken::UIKit::implementation::UIEvent>(Windows::UI::Xaml::Visibility::Visible);
      m_titleBarIsVisibleEvent(*this, *args);
    } else {
      auto args = winrt::make_self<winrt::kraken::UIKit::implementation::UIEvent>(Windows::UI::Xaml::Visibility::Collapsed);
      m_titleBarIsVisibleEvent(*this, *args);
    }
  }

  void UIResponder::SetNavigationView(NavigationView const& view)
  {
    auto args = winrt::make_self<winrt::kraken::UIKit::implementation::UIEvent>(view);
    m_setNavigationViewEvent(*this, *args);
  }

  void UIResponder::SetWindowActivated(bool activate)
  {
    auto args = winrt::make_self<winrt::kraken::UIKit::implementation::UIEvent>(WindowIsActivated());
    m_windowActivatedEvent(*this, *args);
  }
}


namespace winrt::Kraken::UIKit::implementation
{
  winrt::event_token UIResponder::SignalTitleBarUpdateLayout(Kraken::UIKit::SignalDelegate const& handler)
  {
    return make_self<factory_implementation::UIResponder>()->SignalTitleBarUpdateLayout(handler);
  }

  winrt::event_token UIResponder::SignalTitleBarIsVisible(Kraken::UIKit::SignalDelegate const& handler)
  {
    return make_self<factory_implementation::UIResponder>()->SignalTitleBarIsVisible(handler);
  }

  winrt::event_token UIResponder::SignalNavigationIsDisplaying(Kraken::UIKit::SignalDelegate const& handler)
  {
    return make_self<factory_implementation::UIResponder>()->SignalNavigationIsDisplaying(handler);
  }

  winrt::event_token UIResponder::SignalWindowIsActivated(Kraken::UIKit::SignalDelegate const& handler)
  {
    return make_self<factory_implementation::UIResponder>()->SignalWindowIsActivated(handler);
  }


  void UIResponder::SignalTitleBarUpdateLayout(winrt::event_token const& cookie)
  {
    return make_self<factory_implementation::UIResponder>()->SignalTitleBarUpdateLayout(handler);
  }

  void UIResponder::SignalTitleBarIsVisible(winrt::event_token const& cookie)
  {
    return make_self<factory_implementation::UIResponder>()->SignalTitleBarIsVisible(handler);
  }

  void UIResponder::SignalNavigationIsDisplaying(winrt::event_token const& cookie)
  {
    return make_self<factory_implementation::UIResponder>()->SignalNavigationIsDisplaying(handler);
  }

  void UIResponder::SignalWindowIsActivated(winrt::event_token const& cookie)
  {
    return make_self<factory_implementation::UIResponder>()->SignalWindowIsActivated(handler);
  }


  void UIResponder::SetTitleBarLayout(CoreApplicationViewTitleBar const& layout)
  {
    return make_self<factory_implementation::UIResponder>()->SetTitleBarLayout(layout);
  }

  void UIResponder::SetTitleBarIsVisible(CoreApplicationViewTitleBar const& titleBar)
  {
    return make_self<factory_implementation::UIResponder>()->SetTitleBarIsVisible(titleBar);
  }

  void UIResponder::SetNavigationView(NavigationView const& view)
  {
    return make_self<factory_implementation::UIResponder>()->SetNavigationView(view);
  }

  void UIResponder::SetWindowActivated(bool activate)
  {
    return make_self<factory_implementation::UIResponder>()->SetWindowActivated(activate);
  }
}

