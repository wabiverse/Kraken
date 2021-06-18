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
 * Creator.
 * Creating Chaos.
 */

#ifdef _WIN32
#  include <Windows.h>
#endif

#include "CKE_context.h"
#include "CKE_main.h"

#include "WM_api.h"
#include "WM_init_exit.h"
#include "WM_window.h"

#include "environment.h"

int main(int argc, const char **argv)
{
#ifdef _WIN32
  HWND hwnd = GetConsoleWindow();
  ShowWindow(hwnd, 0);
#endif

  wabi::CREATOR_covah_env_init();

  wabi::cContext C = wabi::CTX_create();

  wabi::CKE_covah_globals_init();
  wabi::CKE_covah_main_init(C, argc, (const char **)argv);

  wabi::WM_init(C, argc, (const char **)argv);
  wabi::WM_main(C);
  return wabi::COVAH_SUCCESS;
}
