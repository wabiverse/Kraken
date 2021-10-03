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

#ifdef WITH_WINUI3

#  include "ChaosEngine/Kraken.UIKit.UIScreen.h"

#  if __has_include("Kraken.UIKit.UIScreen.g.cpp")
#    include "Kraken.UIKit.UIScreen.g.cpp"
#  endif /* Kraken.UIKit.UIScreen.g.cpp */


namespace winrt::Kraken::UIKit::implementation
{
  UIScreen::UIScreen()
  {
    InitializeComponent();
    m_mainAppWindow = GetAppWindowForCurrentWindow();
    this->Title(m_windowTitle);
  }

  winrt::AppWindow UIScreen::AppWindow()
  {
    return m_mainAppWindow;
  }

  winrt::AppWindow UIScreen::GetAppWindowForCurrentWindow()
  {
    winrt::Kraken::UIScreen thisWindow = *this;
    winrt::com_ptr<IWindowNative> windowNative = thisWindow.as<IWindowNative>();

    HWND hWnd;
    windowNative->get_WindowHandle(&hWnd);

    winrt::WindowId windowId;
    winrt::GetWindowIdFromWindowHandle(hWnd, &windowId);

    Microsoft::UI::Windowing::AppWindow appWindow = Microsoft::UI::Windowing::AppWindow::GetFromWindowId(windowId);

    return appWindow;
  }
}  // namespace winrt::Kraken::implementation

#endif /* WITH_WINUI3 */