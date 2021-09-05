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

#include <wabi/base/arch/defines.h>

#if defined(ARCH_OS_WINDOWS)
#  include <windows.h>
#  include <unknwn.h>
#  include <restrictederrorinfo.h>
#  include <hstring.h>

#  ifdef GetCurrentTime
/**
 * Resolve a conflict between
 * windows.h and winrt which
 * both define this macro. */
#    undef GetCurrentTime
#  endif /* GetCurrentTime */

#  include <winrt/base.h>
#  include <winrt/Kraken.h>

#  include <winrt/Windows.Foundation.h>
#  include <winrt/Windows.Foundation.Collections.h>
#  include <winrt/Windows.ApplicationModel.Activation.h>
#  include <winrt/Windows.ApplicationModel.Core.h>
#  include <winrt/Windows.UI.h>
#  include <winrt/Windows.UI.Core.h>
#  include <winrt/Windows.UI.Xaml.Interop.h>
#  include <winrt/Windows.UI.Xaml.Markup.h>

#  include <winrt/Microsoft.UI.Xaml.Automation.Peers.h>
#  include <winrt/Microsoft.UI.Xaml.Controls.h>
#  include <winrt/Microsoft.UI.Xaml.Controls.AnimatedVisuals.h>
#  include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h>
#  include <winrt/Microsoft.UI.Xaml.Media.h>
#  include <winrt/Microsoft.UI.Xaml.XamlTypeInfo.h>

#  ifdef WITH_WINUI3
#    include <winrt/Microsoft.UI.h>
#    include <winrt/Microsoft.UI.Composition.h>
#    include <winrt/Microsoft.UI.Interop.h>
#    include <winrt/Microsoft.UI.Windowing.h>
#    include <winrt/Microsoft.UI.Xaml.h>
#    include <winrt/Microsoft.UI.Xaml.Data.h>
#    include <winrt/Microsoft.UI.Xaml.Interop.h>
#    include <winrt/Microsoft.UI.Xaml.Markup.h>
#    include <winrt/Microsoft.UI.Xaml.Navigation.h>
#    include <winrt/Microsoft.UI.Xaml.Shapes.h>
#    include <winrt/Microsoft.UI.Xaml.Input.h>
#    include <Microsoft.UI.Xaml.Window.h>
#  endif /* WITH_WINUI3 */

#  pragma comment(lib, "windowsapp")

namespace winrt
{
#  ifdef WITH_WINUI3
  using namespace Microsoft::UI;
  using namespace Microsoft::UI::Xaml;
  using namespace Microsoft::UI::Xaml::Controls;
  using namespace Microsoft::UI::Xaml::Navigation;
  using namespace Microsoft::UI::Windowing;
#  endif /* WITH_WINUI3 */
  using namespace Windows::ApplicationModel;
  using namespace Windows::ApplicationModel::Core;
  using namespace Windows::Graphics;
  using namespace Windows::Foundation;
  using namespace Windows::Foundation::Collections;
  using namespace Windows::UI;
  using namespace Windows::UI::Core;
}  // namespace winrt

#  ifdef WITH_WINUI3
#    include "Kraken/Microsoft/MainWindow.h"
#  else 
#    include "Kraken/Microsoft/MainPage.h" 
#  endif /* WITH_WINUI3 */

#endif /* ARCH_OS_WINDOWS */