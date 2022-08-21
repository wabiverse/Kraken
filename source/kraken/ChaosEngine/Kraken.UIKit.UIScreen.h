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
 * Copyright 2022, Wabi Animation Studios, Ltd. Co.
 */

/**
 * @file
 * Microsoft.
 * KrakenRT.
 */

#pragma once

#ifdef WITH_WINUI3
#  include "Kraken.UIKit.UIScreen.g.h"

namespace winrt::Kraken::UIKit::implementation
{
  struct UIScreen : UIScreenT<UIScreen>
  {
    UIScreen();

    winrt::AppWindow AppWindow();

   private:

    winrt::AppWindow GetAppWindowForCurrentWindow();

    winrt::AppWindow m_mainAppWindow{nullptr};
    hstring m_windowTitle = L"Kraken";

    bool m_contentLoaded;
  };
}  // namespace winrt::Kraken::UIKit::implementation


namespace winrt::Kraken::factory_implementation
{
  struct UIScreen : UIScreenT<UIScreen, implementation::UIScreen>
  {};
}  // namespace winrt::Kraken::factory_implementation

#endif /* WITH_WINUI3 */