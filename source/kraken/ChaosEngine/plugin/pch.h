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

#include "wabi/wabi.h"
#include "wabi/base/arch/defines.h"

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
#  include <winrt/Windows.Foundation.h>
#  include <winrt/Windows.Foundation.Collections.h>
#  include <winrt/Windows.ApplicationModel.Activation.h>

#  pragma comment(lib, "windowsapp")

#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#endif /* ARCH_OS_WINDOWS */