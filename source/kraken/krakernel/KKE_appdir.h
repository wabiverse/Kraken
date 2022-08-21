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
 * KRAKEN Kernel.
 * Purple Underground.
 */

#include "KKE_api.h"

WABI_NAMESPACE_BEGIN

void KKE_appdir_init(void);
void KKE_appdir_exit(void);
void KKE_appdir_program_path_init(const char *argv0);

const char *KKE_appdir_folder_default(void);
const char *KKE_appdir_folder_home(void);
const char *KKE_appdir_program_path(void);
const char *KKE_appdir_program_dir(void);
const char *KKE_appdir_folder_id(const int folder_id, const char *subfolder);
const char *KKE_appdir_folder_id_user_notest(const int folder_id, const char *subfolder);
const char *KKE_appdir_folder_id_create(const int folder_id, const char *subfolder);
const char *KKE_appdir_folder_id_version(const int folder_id,
                                         const int version,
                                         const bool check_is_dir);

bool KKE_appdir_folder_documents(char *dir);
bool KKE_appdir_font_folder_default(char *dir);
bool KKE_appdir_app_is_portable_install(void);
bool KKE_appdir_folder_id_ex(const int folder_id,
                             const char *subfolder,
                             char *path,
                             size_t path_len);
bool KKE_appdir_program_python_search(char *fullpath,
                                      const size_t fullpath_len,
                                      const int version_major,
                                      const int version_minor);

const char *KKE_appdir_copy_recursive(const int src_id, const int target_id) noexcept;

/* Initialize path to temporary directory. */
void KKE_tempdir_init(const char *userdir);

const char *KKE_tempdir_base(void);
const char *KKE_tempdir_session(void);
void KKE_tempdir_session_purge(void);

/* folder_id */
enum
{
  /* general, will find based on user/local/system priority */
  KRAKEN_DATAFILES = 2,

  /* user-specific */
  KRAKEN_USER_CONFIG = 31,
  KRAKEN_USER_DATAFILES = 32,
  KRAKEN_USER_SCRIPTS = 33,
  KRAKEN_USER_AUTOSAVE = 34,

  /* system */
  KRAKEN_SYSTEM_DATAFILES = 52,
  KRAKEN_SYSTEM_SCRIPTS = 53,
  KRAKEN_SYSTEM_PYTHON = 54,
};

/* for KKE_appdir_folder_id_version only */
enum
{
  KRAKEN_RESOURCE_PATH_USER = 0,
  KRAKEN_RESOURCE_PATH_LOCAL = 1,
  KRAKEN_RESOURCE_PATH_SYSTEM = 2,
};

#define KRAKEN_STARTUP_FILE "startup.usd"
#define KRAKEN_USERPREF_FILE "userpref.usd"
#define KRAKEN_QUIT_FILE "quit.usd"
#define KRAKEN_BOOKMARK_FILE "bookmarks.txt"
#define KRAKEN_HISTORY_FILE "recent-files.txt"
#define KRAKEN_PLATFORM_SUPPORT_FILE "platform_support.txt"

WABI_NAMESPACE_END