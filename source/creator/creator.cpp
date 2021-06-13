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

#include "environment.h"

int main(int argc, char *argv[])
{
  cContext *C;

#ifdef _WIN32
  HWND hwnd = GetConsoleWindow();
  ShowWindow(hwnd, 0);
#endif

  CREATOR_covah_env_init();

  C = CTX_create();

  CKE_covah_globals_init();
  CKE_covah_main_init(argc, argv, C);
  return 0;
}
