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

#include "KLI_threads.h"

#include "KKE_appdir.h"
#include "KKE_context.h"
#include "KKE_main.h"

#include "UNI_pixar_utils.h"

#include "WM_api.h"
#include "WM_init_exit.h"
#include "WM_window.h"

#include "creator.h"

WABI_NAMESPACE_USING

int main(int argc, const char **argv)
{
  kContext *C;

  /* Environment variables. */
  CREATOR_kraken_env_init();

  /* Create Context C. */
  C = CTX_create();

  /* Initialize path to executable. */
  KKE_appdir_program_path_init();

  /* Initialize Threads. */
  KLI_threadapi_init();

  /* Initialize Globals (paths, sys). */
  KKE_kraken_globals_init();

  /* Init plugins. */
  KKE_kraken_plugins_init();

  /* Init & parse args. */
  CREATOR_setup_args(argc, (const char **)argv);
  if (CREATOR_parse_args(argc, (const char **)argv) != 0)
  {
    return 0;
  }

  KKE_appdir_init();

  /* Determining Stage Configuration and Loadup. */
  KKE_kraken_main_init(C, argc, (const char **)argv);

  /* Runtime. */
  WM_init(C, argc, (const char **)argv);
  WM_main(C);

  return KRAKEN_SUCCESS;
}
