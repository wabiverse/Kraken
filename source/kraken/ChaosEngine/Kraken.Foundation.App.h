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

#include "Kraken.Foundation.App.Xaml.g.h"

#include "KLI_utildefines.h"

#include "ChaosEngine/Kraken.UIKit.UIResponder.h"

namespace winrt::Kraken::Foundation::implementation
{
  struct App : AppT<App>
  {
    App();

    void OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs const &args);

   private:

    UIResponder m_UIResponder;
  };
}  // namespace winrt::Kraken::Foundation::implementation
