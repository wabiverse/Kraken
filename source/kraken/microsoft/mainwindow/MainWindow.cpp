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

#include "Kraken/Microsoft/MainWindow.h"

#if __has_include("MainWindow.g.cpp")
#  include "MainWindow.g.cpp"
#endif /* MainWindow.g.cpp */
#if __has_include("Kraken/Microsoft/MainWindow/MainWindow.Xaml.g.hpp")
#  include "Kraken/Microsoft/MainWindow/MainWindow.Xaml.g.hpp"
#endif /* MainWindow.Xaml.g.hpp */


using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;


namespace winrt::kraken::implementation
{
  MainWindow::MainWindow()
  {
    InitializeComponent();
    m_mainAppWindow = GetAppWindowForCurrentWindow();
    this->Title(m_windowTitle);
  }

  winrt::AppWindow MainWindow::AppWindow()
  {
    return m_mainAppWindow;
  }

  winrt::AppWindow MainWindow::GetAppWindowForCurrentWindow()
  {
    winrt::kraken::MainWindow thisWindow = *this;
    winrt::com_ptr<IWindowNative> windowNative = thisWindow.as<IWindowNative>();

    HWND hWnd;
    windowNative->get_WindowHandle(&hWnd);

    winrt::WindowId windowId;
    winrt::GetWindowIdFromWindowHandle(hWnd, &windowId);

    Microsoft::UI::Windowing::AppWindow appWindow = Microsoft::UI::Windowing::AppWindow::GetFromWindowId(windowId);

    return appWindow;
  }
}  // namespace winrt::kraken::implementation

#endif /* WITH_WINUI3 */