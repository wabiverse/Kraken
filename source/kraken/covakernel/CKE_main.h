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
 * KRAKEN Kernel.
 * Purple Underground.
 */

#ifndef KRAKEN_KERNEL_MAIN_H
#define KRAKEN_KERNEL_MAIN_H

#include "CKE_api.h"
#include "CKE_context.h"
#include "CKE_robinhood.h"

#include "UNI_object.h"

WABI_NAMESPACE_BEGIN

struct Main : public UniverseObject
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

  std::string kraken_version_decimal;

  std::vector<struct WorkSpace *> workspaces;
  std::vector<struct cScreen *> screens;
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
  KRAKEN_SUCCESS = 0,
  KRAKEN_ERROR,
};

enum ckeErrorType
{
  KRAKEN_ERROR_VERSION,
  KRAKEN_ERROR_IO,
  KRAKEN_ERROR_GL,
  KRAKEN_ERROR_HYDRA
};

KRAKEN_KERNEL_API
Main CKE_main_init(void);

KRAKEN_KERNEL_API
void CKE_kraken_main_init(cContext *C, int argc, const char **argv);

KRAKEN_KERNEL_API
void CKE_kraken_globals_init();

KRAKEN_KERNEL_API
void CKE_kraken_plugins_init(void);

KRAKEN_KERNEL_API
void CKE_kraken_python_init(cContext *C);

KRAKEN_KERNEL_API
ckeStatusCode CKE_main_runtime(int backend);

KRAKEN_KERNEL_API
void CKE_kraken_enable_debug_codes(void);


/* ------ */


/* Setup in CKE_kraken. */
KRAKEN_KERNEL_API
extern Global G;

KRAKEN_KERNEL_API
/* Setup in CKE_kraken. */
extern float UI_DPI_FAC;

WABI_NAMESPACE_END

#endif /* KRAKEN_KERNEL_MAIN_H */