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

#include "Kraken/Microsoft/MainPage.h"
#include "MainPage.g.cpp"

#pragma warning(push)
#pragma warning(disable: 4100)

namespace winrt::kraken::implementation
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
  void MainPageT<D, I...>::InitializeComponent()
  {
    if (!_contentLoaded)
    {
      _contentLoaded = true;
      Uri resourceLocator{ L"ms-appx:///kraken/microsoft/mainpage/MainPage.xaml" };
      Application::LoadComponent(*this, resourceLocator, ComponentResourceLocation::Application);
    }
  }

  template <typename D, typename... I>
  void MainPageT<D, I...>::Connect(int32_t, IInspectable const&)
  {
    _contentLoaded = true;
  }

  template <typename D, typename ... I>
  void MainPageT<D, I...>::DisconnectUnloadedObject(int32_t)
  {
    throw ::winrt::hresult_invalid_argument { L"No unloadable objects to disconnect." };
  }

  template <typename D, typename ... I>
  void MainPageT<D, I...>::UnloadObject(DependencyObject const&)
  {
    throw ::winrt::hresult_invalid_argument { L"No unloadable objects." };
  }


  template <typename D, typename... I>
  IComponentConnector MainPageT<D, I...>::GetBindingConnector(int32_t, IInspectable const&)
  {
    return nullptr;
  }

  template struct MainPageT<struct MainPage>;
} // namespace winrt::kraken::implementation

#pragma warning(pop)


namespace winrt::kraken::implementation
{
  MainPage::MainPage()
  {
    InitializeComponent();
  }
}  // namespace winrt::kraken::implementation
