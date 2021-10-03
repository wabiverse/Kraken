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

#include "ChaosEngine/Kraken.UIKit.UIView.h"
#include "Kraken.UIKit.UIView.g.cpp"

#pragma warning(push)
#pragma warning(disable: 4100) // unreferenced formal parameter

namespace winrt::Kraken::UIKit::implementation
{
using Application = ::winrt::Windows::UI::Xaml::Application;
using ComponentResourceLocation = ::winrt::Windows::UI::Xaml::Controls::Primitives::ComponentResourceLocation;
using DataTemplate = ::winrt::Windows::UI::Xaml::DataTemplate;
using DependencyObject = ::winrt::Windows::UI::Xaml::DependencyObject;
using DependencyProperty = ::winrt::Windows::UI::Xaml::DependencyProperty;
using IComponentConnector = ::winrt::Windows::UI::Xaml::Markup::IComponentConnector;
using Uri = ::winrt::Windows::Foundation::Uri;
using XamlBindingHelper = ::winrt::Windows::UI::Xaml::Markup::XamlBindingHelper;

template <typename D, typename ... I>
void UIViewT<D, I...>::InitializeComponent()
{
  if (!_contentLoaded)
  {
    _contentLoaded = true;
    Uri resourceLocator{ L"ms-appx:///Kraken/UIKit/UIView/UIView.xaml" };
    Application::LoadComponent(*this, resourceLocator, ComponentResourceLocation::Application);
  }
}

template <typename D, typename ... I>
void UIViewT<D, I...>::Connect(int32_t connectionId, IInspectable const& target)
{
  switch (connectionId)
  {
    case 2:
    {
      auto targetElement = target.as<::winrt::Windows::UI::Xaml::Controls::Border>();
      this->AppTitleBar(targetElement);
    }
    break;
    case 3:
    {
      auto targetElement = target.as<::winrt::Microsoft::UI::Xaml::Controls::NavigationView>();
      this->NavigationViewControl(targetElement);
      auto weakThis = ::winrt::make_weak<class_type>(*this);
      targetElement.DisplayModeChanged([weakThis](::winrt::Microsoft::UI::Xaml::Controls::NavigationView const& p0, ::winrt::Microsoft::UI::Xaml::Controls::NavigationViewDisplayModeChangedEventArgs const& p1){
        if (auto t = weakThis.get())
        {
          ::winrt::get_self<D>(t)->NavigationViewControl_DisplayModeChanged(p0, p1);
        }
      });
    }
    break;
    case 4:
    {
      auto targetElement = target.as<::winrt::Windows::UI::Xaml::Controls::Frame>();
      this->contentFrame(targetElement);
    }
    break;
    case 5:
    {
      auto targetElement = target.as<::winrt::Windows::UI::Xaml::Controls::Image>();
      this->AppFontIcon(targetElement);
    }
    break;
    case 6:
    {
      auto targetElement = target.as<::winrt::Windows::UI::Xaml::Controls::TextBlock>();
      this->AppTitle(targetElement);
    }
    break;
  }
  _contentLoaded = true;
}

template <typename D, typename ... I>
void UIViewT<D, I...>::DisconnectUnloadedObject(int32_t)
{
  throw ::winrt::hresult_invalid_argument { L"No unloadable objects to disconnect." };
}

template <typename D, typename ... I>
void UIViewT<D, I...>::UnloadObject(DependencyObject const&)
{
  throw ::winrt::hresult_invalid_argument { L"No unloadable objects." };
}

template <typename D, typename... I>
IComponentConnector UIViewT<D, I...>::GetBindingConnector(int32_t, IInspectable const&)
{
  return nullptr;
}

template struct UIViewT<struct UIView>;


#pragma warning(pop)


UIView::UIView()
{
  InitializeComponent();

  Windows::UI::ViewManagement::ApplicationViewTitleBar titleBar = Windows::UI::ViewManagement::ApplicationView::GetForCurrentView().TitleBar();

  titleBar.ButtonBackgroundColor(Windows::UI::Colors::Transparent());
  titleBar.ButtonInactiveBackgroundColor(Windows::UI::Colors::Transparent());

  auto coreTitleBar = Windows::ApplicationModel::Core::CoreApplication::GetCurrentView().TitleBar();
  coreTitleBar.ExtendViewIntoTitleBar(true);
  UIResponder::SetTitleBarLayout(coreTitleBar);

  Windows::UI::Xaml::Window currentWindow = Windows::UI::Xaml::Window::Current();

  currentWindow.SetTitleBar(AppTitleBar());

  coreTitleBar.LayoutMetricsChanged(&TitleBarLayoutEvent);
  coreTitleBar.IsVisibleChanged(&TitleBarVisibilityEvent);
  currentWindow.Activated(&WindowActivatedEvent);
}


void UIView::TitleBarLayoutEvent(Windows::ApplicationModel::Core::CoreApplicationViewTitleBar const& sender,
                                   IInspectable const& event)
{
  UIResponder::SetTitleBarLayout(sender);
}


void UIView::TitleBarVisibilityEvent(Windows::ApplicationModel::Core::CoreApplicationViewTitleBar const& sender,
                                       IInspectable const& event)
{
  if (sender.IsVisible())
  {
    AppTitleBar().Visibility(Windows::UI::Xaml::Visibility::Visible);
  }
  else
  {
    AppTitleBar().Visibility(Windows::UI::Xaml::Visibility::Collapsed);
  }
}


void UIView::WindowActivatedEvent(IInspectable const& sender,
                                    Windows::UI::Core::WindowActivatedEventArgs const& event)
{
  Uri resPrimary { L"TextFillColorPrimaryBrush" };
  auto primaryTextFillColor { 
    Application::Current().Resources().Lookup(resPrimary).as<Windows::UI::Xaml::Media::SolidColorBrush>()
  };

  Uri resDisabled { L"TextFillColorDisabledBrush" };
  auto disabledTextFillColor {
    Application::Current().Resources().Lookup(resDisabled).as<Windows::UI::Xaml::Media::SolidColorBrush>()
  };


  if (event.WindowActivationState() == Windows::UI::Core::CoreWindowActivationState::Deactivated)
  {
    AppTitle().Foreground(disabledTextFillColor);
  }
  else
  {
    AppTitle().Foreground(primaryTextFillColor);
  }
}


void UIView::NavigationViewControl_DisplayModeChanged(Microsoft::UI::Xaml::Controls::NavigationView const& sender,
                                                      Microsoft::UI::Xaml::Controls::NavigationViewDisplayModeChangedEventArgs const& event)
{
  const int topIndent = 16;
  const int expandedIndent = 48;
  int minimalIndent = 104;

  if (NavigationViewControl().IsBackButtonVisible() == Microsoft::UI::Xaml::Controls::NavigationViewBackButtonVisible::Collapsed)
  {
    minimalIndent = 48;
  }

  Windows::UI::Xaml::Thickness currMargin = AppTitleBar().Margin();
  Windows::UI::Xaml::Thickness newMargin = {};

  if (sender.PaneDisplayMode() == Microsoft::UI::Xaml::Controls::NavigationViewPaneDisplayMode::Top) {
    newMargin.Left = topIndent;
    newMargin.Top = currMargin.Top;
    newMargin.Right = currMargin.Right;
    newMargin.Bottom = currMargin.Bottom;
  }

  else if (sender.DisplayMode() == Microsoft::UI::Xaml::Controls::NavigationViewDisplayMode::Minimal) {
    newMargin.Left = minimalIndent;
    newMargin.Top = currMargin.Top;
    newMargin.Right = currMargin.Right;
    newMargin.Bottom = currMargin.Bottom;
  }

  else {
    newMargin.Left = expandedIndent;
    newMargin.Top = currMargin.Top;
    newMargin.Right = currMargin.Right;
    newMargin.Bottom = currMargin.Bottom;
  }

  AppTitleBar().Margin(newMargin);
}

} // namespace winrt::Kraken::implementation
