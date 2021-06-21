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
 * Window Manager.
 * Making GUI Fly.
 */

#include "WM_api.h"

WABI_NAMESPACE_BEGIN

COVAH_WM_API
void WM_init_default_styles();

COVAH_WM_API
void WM_init(cContext C, int argc, const char **argv);

COVAH_WM_API
void WM_main(const cContext &C);

COVAH_WM_API
void WM_check(const cContext &C);

enum eWmModes
{
  COVAH_NORMAL_MODE = 0,
  COVAH_DEBUG_MODE
};

enum eWmKernelPaths
{
  EXE_PATH = 0,
  DATAFILES_PATH,
  STYLES_PATH,
  ICONS_PATH,
  PROJECT_FILE,

  VERSION_DECIMAL
};

WABI_NAMESPACE_END