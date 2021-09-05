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

#pragma once

#include "App.Xaml.g.h"

#include "KLI_utildefines.h"

#ifdef WITH_WINUI3
#  include "Kraken/Microsoft/MainWindow.h"
#endif /* WITH_WINUI3 */

namespace winrt::kraken::implementation
{
  struct App : AppT<App>
  {
    App();

    void OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs const &args);

#ifdef WITH_WINUI3

   private:
    winrt::Microsoft::UI::Xaml::Window window{nullptr};
#endif /* WITH_WINUI3 */
  };
}  // namespace winrt::kraken::implementation
