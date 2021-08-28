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
 * ⚓︎ Anchor.
 * Bare Metal.
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
#  include <winrt/Windows.Foundation.h>
#  include <winrt/Windows.Foundation.Collections.h>
#  include <winrt/Windows.ApplicationModel.Activation.h>
#  include <winrt/Microsoft.UI.Composition.h>
#  include <winrt/Microsoft.UI.Xaml.h>
#  include <winrt/Microsoft.UI.Xaml.Controls.h>
#  include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h>
#  include <winrt/Microsoft.UI.Xaml.Data.h>
#  include <winrt/Microsoft.UI.Xaml.Interop.h>
#  include <winrt/Microsoft.UI.Xaml.Markup.h>
#  include <winrt/Microsoft.UI.Xaml.Media.h>
#  include <winrt/Microsoft.UI.Xaml.Navigation.h>
#  include <winrt/Microsoft.UI.Xaml.Shapes.h>
#  include <winrt/Microsoft.UI.Dispatching.h>

#  pragma comment(lib, "windowsapp")

#endif /* ARCH_OS_WINDOWS */