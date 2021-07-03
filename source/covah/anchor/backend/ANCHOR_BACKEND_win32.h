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

#pragma once

/**
 * @file
 * Anchor.
 * Bare Metal.
 */

#include "ANCHOR_api.h"

#ifdef _WIN32

ANCHOR_BACKEND_API bool     ANCHOR_ImplWin32_Init(void* hwnd);
ANCHOR_BACKEND_API void     ANCHOR_ImplWin32_Shutdown();
ANCHOR_BACKEND_API void     ANCHOR_ImplWin32_NewFrame();

// Win32 message handler.
#if 0
extern ANCHOR_BACKEND_API LRESULT ANCHOR_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

// DPI-related helpers (optional)
// - Use to enable DPI awareness without having to create an application manifest.
// - Your own app may already do this via a manifest or explicit calls. This is mostly useful for our examples/ apps.
// - In theory we could call simple functions from Windows SDK such as SetProcessDPIAware(), SetProcessDpiAwareness(), etc.
//   but most of the functions provided by Microsoft require Windows 8.1/10+ SDK at compile time and Windows 8/10+ at runtime,
//   neither we want to require the user to have. So we dynamically select and load those functions to avoid dependencies.
ANCHOR_BACKEND_API void     ANCHOR_ImplWin32_EnableDpiAwareness();
ANCHOR_BACKEND_API float    ANCHOR_ImplWin32_GetDpiScaleForHwnd(void* hwnd);       // HWND hwnd
ANCHOR_BACKEND_API float    ANCHOR_ImplWin32_GetDpiScaleForMonitor(void* monitor); // HMONITOR monitor

// Transparency related helpers (optional) [experimental]
// - Use to enable alpha compositing transparency with the desktop.
// - Use together with e.g. clearing your framebuffer with zero-alpha.
ANCHOR_BACKEND_API void     ANCHOR_ImplWin32_EnableAlphaCompositing(void* hwnd);   // HWND hwnd

#elif

/* This file is ignored on all platforms outside Windows. */

#endif /* _WIN32 */