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
 * KRAKEN Kernel.
 * Purple Underground.
 */

#pragma once

#include <wabi/base/arch/defines.h>

#if defined(ARCH_OS_WINDOWS)
#  include <unknwn.h>
#  include <winrt/Windows.Foundation.h>
#  include <winrt/Windows.Foundation.Collections.h>
#  include <winrt/Windows.UI.Composition.h>
#  include <winrt/Windows.UI.Xaml.h>
#  include <winrt/Windows.UI.Xaml.Controls.h>
#  include <winrt/Windows.UI.Xaml.Controls.Primitives.h>
#  include <winrt/Windows.UI.Xaml.Data.h>
#  include <winrt/Windows.UI.Xaml.Markup.h>
#  include <winrt/Windows.UI.Xaml.Navigation.h>
#  include <winrt/Microsoft.UI.Xaml.Automation.Peers.h>
#  include <winrt/Microsoft.UI.Xaml.Controls.h>
#  include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h>
#  include <winrt/Microsoft.UI.Xaml.Media.h>
#  include <winrt/Microsoft.UI.Xaml.XamlTypeInfo.h>
#  pragma comment(lib, "windowsapp")

#endif /* ARCH_OS_WINDOWS */
