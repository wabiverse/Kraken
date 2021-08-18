r /*
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
 * Creator.
 * Creating Chaos.
 */

#pragma once

#include <wabi/base/arch/defines.h>

#if defined(ARCH_OS_WINDOWS)

#  include "creator_xaml_typeinfo.h"
#  include "creator_xaml_metadata.h"

  namespace winrt::Kraken::implementation
{

  template<typename D, typename... Interfaces>
  struct CreatorT : public ::winrt::Windows::UI::Xaml::ApplicationT<D, ::winrt::Windows::UI::Xaml::Markup::IXamlMetadataProvider, Interfaces...>
  {
    using IXamlType = ::winrt::Windows::UI::Xaml::Markup::IXamlType;

    void InitializeComponent()
    {}

    IXamlType GetXamlType(::winrt::Windows::UI::Xaml::Interop::TypeName const &type)
    {
      return AppProvider()->GetXamlType(type);
    }

    IXamlType GetXamlType(::winrt::hstring const &fullName)
    {
      return AppProvider()->GetXamlType(fullName);
    }

    ::winrt::com_array<::winrt::Windows::UI::Xaml::Markup::XmlnsDefinition> GetXmlnsDefinitions()
    {
      return AppProvider()->GetXmlnsDefinitions();
    }

   private:
    bool _contentLoaded{false};
    winrt::com_ptr<XamlMetaDataProvider> _appProvider;
    winrt::com_ptr<XamlMetaDataProvider> AppProvider()
    {
      if (!_appProvider)
      {
        _appProvider = winrt::make_self<XamlMetaDataProvider>();
      }
      return _appProvider;
    }
  };

  struct Creator : CreatorT<Creator>
  {
    Creator();

    void OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs const &);
    void OnSuspending(IInspectable const &, Windows::ApplicationModel::SuspendingEventArgs const &);
    void OnNavigationFailed(IInspectable const &, Windows::UI::Xaml::Navigation::NavigationFailedEventArgs const &);
  };

}  // namespace winrt::Kraken::implementation

#else /* ARCH_OS_WINDOWS */

/**
 * Intentionally left empty for Linux & Darwin. */

#endif /* ARCH_OS_LINUX || ARCH_OS_DARWIN */