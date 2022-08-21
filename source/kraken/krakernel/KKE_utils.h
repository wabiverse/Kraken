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

/**
 * @file
 * KRAKEN Kernel.
 * Purple Underground.
 */

#pragma once

#include "KKE_api.h"
#include "KKE_main.h"
#include "KKE_robinhood.h"

#include <wabi/base/arch/systemInfo.h>
#include <wabi/base/tf/stringUtils.h>
#include <wabi/base/tf/token.h>

WABI_NAMESPACE_BEGIN

std::string kraken_exe_path_init(void);
std::string kraken_system_tempdir_path(void);

std::string kraken_datafiles_path_init(void);
std::string kraken_fonts_path_init(void);
std::string kraken_python_path_init(void);
std::string kraken_icon_path_init(void);
std::string kraken_startup_file_init(void);
std::string kraken_ocio_file_init(void);

typedef robin_hood::unordered_map<TfToken, void *, TfHash> RHash;

void *KKE_rhash_lookup(RHash *rh, const TfToken &key);
void KKE_rhash_insert(RHash *rh, const TfToken &key, void *value);

WABI_NAMESPACE_END