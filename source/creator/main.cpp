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
 * Creator.
 * Creating Chaos. */

#include "main.h"

using namespace winrt;
using namespace winrt::Windows;
using namespace winrt::Windows::Foundation;
using namespace Windows::UI::Xaml;

void *winrt_make_Kraken_Main()
{
  return winrt::detach_abi(winrt::make<winrt::Kraken::factory_implementation::Main>());
}

WINRT_EXPORT namespace winrt::Kraken
{
  Main::Main()
    : Main(make<Kraken::implementation::Main>())
  {}
}

namespace winrt::Kraken::implementation
{
  Main::Main()
  {
    InitializeComponent();
  }

  int32_t Main::MyProperty()
  {
    throw hresult_not_implemented();
  }

  void Main::MyProperty(int32_t /* value */)
  {
    throw hresult_not_implemented();
  }

  void Main::ClickHandler(IInspectable const&, RoutedEventArgs const&)
  {
    krakenLogoButton().Content(box_value(L"Clicked"));
  }
}
