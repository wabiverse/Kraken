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

#include "KKE_api.h"
#include "KKE_context.h"
#include "KKE_robinhood.h"

#include "UNI_object.h"

WABI_NAMESPACE_BEGIN

struct Main : public ObjectUNI
{
  uint64_t build_commit_timestamp;
  std::string build_hash;

  std::string exe_path;
  std::string fonts_path;
  std::string temp_dir;
  std::string icons_path;
  std::string python_path;
  std::string datafiles_path;

  std::filesystem::path stage_id;

  std::string kraken_version_decimal;

  char launch_time[UNI_MAX_TIME];

  std::vector<struct WorkSpace *> workspaces;
  std::vector<struct kScreen *> screens;
};

struct Global
{
  Main *main;

  bool server;
  bool factory_startup;
  bool custom_startup;

  bool is_rendering;

  int f;

  /** Message to use when auto execution fails. */
  char autoexec_fail[200];
};

enum eGlobalFlag {
  G_FLAG_RENDER_VIEWPORT = (1 << 0),
  G_FLAG_PICKSEL = (1 << 2),
  /** Support simulating events (for testing). */
  G_FLAG_EVENT_SIMULATE = (1 << 3),
  G_FLAG_USERPREF_NO_SAVE_ON_EXIT = (1 << 4),

  G_FLAG_SCRIPT_AUTOEXEC = (1 << 13),
  /** When this flag is set ignore the prefs #USER_SCRIPT_AUTOEXEC_DISABLE. */
  G_FLAG_SCRIPT_OVERRIDE_PREF = (1 << 14),
  G_FLAG_SCRIPT_AUTOEXEC_FAIL = (1 << 15),
  G_FLAG_SCRIPT_AUTOEXEC_FAIL_QUIET = (1 << 16),
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

Main KKE_main_init(void);
void KKE_kraken_main_init(kContext *C, int argc, const char **argv);
void KKE_kraken_globals_init();
void KKE_kraken_plugins_init(void);
void KKE_kraken_python_init(kContext *C);
ckeStatusCode KKE_main_runtime(int backend);
void KKE_kraken_enable_debug_codes(void);


/* ------ */


/* Setup in KKE_kraken. */
extern Global G;

/* Setup in KKE_kraken. */
extern float UI_DPI_FAC;

WABI_NAMESPACE_END

#endif /* KRAKEN_KERNEL_MAIN_H */