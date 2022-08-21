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

#include "pch.h"

#include "ChaosEngine/Kraken.UIKit.UIEvent.h"

#include "UIKit.UIEvent.g.cpp"

namespace winrt::Kraken::UIKit::implementation
{
  UIEvent::UIEvent(CoreApplicationViewTitleBar const &titleBar)
    : m_titleBar(titleBar),
      m_navigationView(nullptr),
      m_window(nullptr)
  {}

  UIEvent::UIEvent(NavigationView const &view)
    : m_titleBar(nullptr),
      m_navigationView(view),
      m_window(nullptr)
  {}

  UIEvent::UIEvent(IInspectable const &window)
    : m_titleBar(nullptr),
      m_navigationView(nullptr),
      m_window(window)
  {}

  CoreApplicationViewTitleBar UIEvent::TitleBar()
  {
    return m_titleBar;
  }

  NavigationView UIEvent::NavigationView()
  {
    return m_titleBar;
  }

  IInspectable UIEvent::Window()
  {
    return m_window;
  }
}  // namespace winrt::Kraken::UIKit::implementation
