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

#pragma once

/**
 * @file
 * Window Manager.
 * Making GUI Fly.
 */

#include "WM_api.h"

KRAKEN_NAMESPACE_BEGIN

void WM_init(kContext *C, int argc = 0, const char **argv = NULL);
void WM_exit(kContext *C);
void WM_exit_ex(kContext *C, const bool do_python);

void WM_init_manager(kContext *C);

void WM_init_default_styles();
void WM_jobs_kill_all(wmWindowManager *wm);
void WM_main(kContext *C);
void wm_add_default(struct Main *kmain, kContext *C);
void WM_check(kContext *C);

enum eWmModes
{
  KRAKEN_NORMAL_MODE = 0,
  KRAKEN_DEBUG_MODE
};

enum eWmKernelPaths
{
  EXE_PATH = 0,
  DATAFILES_PATH,
  ICONS_PATH,
  PROJECT_FILE,

  VERSION_DECIMAL
};

KRAKEN_NAMESPACE_END