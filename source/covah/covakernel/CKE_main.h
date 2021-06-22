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

#ifndef COVAH_KERNEL_MAIN_H
#define COVAH_KERNEL_MAIN_H

#include "CKE_api.h"
#include "CKE_context.h"
#include "CKE_robinhood.h"

#include "UNI_object.h"

WABI_NAMESPACE_BEGIN

struct Main : public CovahObject
{
  uint64_t build_commit_timestamp;
  std::string build_hash;

  std::string exe_path;
  std::string temp_dir;
  std::string styles_path;
  std::string icons_path;
  std::string python_path;
  std::string datafiles_path;

  std::filesystem::path stage_id;

  std::string covah_version_decimal;
};

struct Global
{
  Main *main;

  bool server;
  bool factory_startup;
  bool custom_startup;

  bool is_rendering;
};

enum ckeStatusCode
{
  COVAH_SUCCESS = 0,
  COVAH_ERROR,
};

enum ckeErrorType
{
  COVAH_ERROR_VERSION,
  COVAH_ERROR_IO,
  COVAH_ERROR_GL,
  COVAH_ERROR_HYDRA
};

COVAH_KERNEL_API
Main CKE_main_init(void);

COVAH_KERNEL_API
void CKE_covah_main_init(cContext *C, int argc, const char **argv);

COVAH_KERNEL_API
void CKE_covah_globals_init();

COVAH_KERNEL_API
void CKE_covah_plugins_init(void);

COVAH_KERNEL_API
void CKE_covah_python_init(cContext *C);

COVAH_KERNEL_API
ckeStatusCode CKE_main_runtime(int backend);

COVAH_KERNEL_API
void CKE_covah_enable_debug_codes(void);


/* ------ */


/* Setup in CKE_covah. */
COVAH_KERNEL_API
extern Global G;

WABI_NAMESPACE_END

#endif /* COVAH_KERNEL_MAIN_H */