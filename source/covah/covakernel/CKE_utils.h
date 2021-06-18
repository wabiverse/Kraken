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
 * COVAH Kernel.
 * Purple Underground.
 */

#pragma once

#include "CKE_api.h"
#include "CKE_main.h"

#include <wabi/base/arch/systemInfo.h>
#include <wabi/base/tf/stringUtils.h>

WABI_NAMESPACE_BEGIN

std::string covah_exe_path_init(void);
std::string covah_system_tempdir_path(void);

std::string covah_datafiles_path_init(Global KERNEL_GLOBALS);
std::string covah_python_path_init(Global KERNEL_GLOBALS);
std::string covah_icon_path_init(Global KERNEL_GLOBALS);
std::string covah_styles_path_init(Global KERNEL_GLOBALS);
std::string covah_startup_file_init(Global KERNEL_GLOBALS);

WABI_NAMESPACE_END