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

#include <stddef.h>

#include "KLI_compiler_attrs.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ListBase;

/**
 * Sanity check to ensure correct API use in debug mode.
 *
 * Run this once the first level of arguments has been passed so we can be sure
 * `--env-system-datafiles`, and other `--env-*` arguments has been passed.
 *
 * Without this any callers to this module that run early on,
 * will miss out on changes from parsing arguments.
 */
void KKE_appdir_init(void);
void KKE_appdir_exit(void);

/**
 * Get the folder that's the "natural" starting point for browsing files on an OS.
 * - Unix: `$HOME`
 * - Windows: `%userprofile%/Documents`
 *
 * \note On Windows `Users/{MyUserName}/Documents` is used as it's the default location to save
 * documents.
 */
const char *KKE_appdir_folder_default(void) ATTR_WARN_UNUSED_RESULT;
const char *KKE_appdir_folder_root(void) ATTR_WARN_UNUSED_RESULT ATTR_RETURNS_NONNULL;
const char *KKE_appdir_folder_default_or_root(void) ATTR_WARN_UNUSED_RESULT ATTR_RETURNS_NONNULL;
/**
 * Get the user's home directory, i.e.
 * - Unix: `$HOME`
 * - Windows: `%userprofile%`
 */
const char *KKE_appdir_folder_home(void);
/**
 * Get the user's document directory, i.e.
 * - Linux: `$HOME/Documents`
 * - Windows: `%userprofile%/Documents`
 *
 * If this can't be found using OS queries (via Ghost), try manually finding it.
 *
 * \returns True if the path is valid and points to an existing directory.
 */
bool KKE_appdir_folder_documents(char *dir) ATTR_NONNULL(1) ATTR_WARN_UNUSED_RESULT;
/**
 * Get the user's cache directory, i.e.
 * - Linux: `$HOME/.cache/KRAKEN/`
 * - Windows: `%USERPROFILE%\AppData\Local\KRAKEN Foundation\KRAKEN\`
 * - MacOS: `/Library/Caches/KRAKEN`
 *
 * \returns True if the path is valid. It doesn't create or checks format
 * if the `KRAKEN` folder exists. It does check if the parent of the path exists.
 */
bool KKE_appdir_folder_caches(char *r_path, size_t path_len) ATTR_NONNULL(1);
/**
 * Get a folder out of the \a folder_id presets for paths.
 *
 * \param subfolder: The name of a directory to check for,
 * this may contain path separators but must resolve to a directory, checked with #BLI_is_dir.
 * \return The path if found, NULL string if not.
 */
bool KKE_appdir_folder_id_ex(int folder_id, const char *subfolder, char *path, size_t path_len);
const char *KKE_appdir_folder_id(int folder_id, const char *subfolder) ATTR_WARN_UNUSED_RESULT;
/**
 * Returns the path to a folder in the user area, creating it if it doesn't exist.
 */
const char *KKE_appdir_folder_id_create(int folder_id,
                                        const char *subfolder) ATTR_WARN_UNUSED_RESULT;
/**
 * Returns the path to a folder in the user area without checking that it actually exists first.
 */
const char *KKE_appdir_folder_id_user_notest(int folder_id,
                                             const char *subfolder) ATTR_WARN_UNUSED_RESULT;
/**
 * Returns the path of the top-level version-specific local, user or system directory.
 * If check_is_dir, then the result will be NULL if the directory doesn't exist.
 */
const char *KKE_appdir_resource_path_id_with_version(int folder_id,
                                                     bool check_is_dir,
                                                     int version);
const char *KKE_appdir_resource_path_id(int folder_id, bool check_is_dir);

/**
 * Check if this is an install with user files kept together
 * with the KRAKEN executable and its installation files.
 */
bool KKE_appdir_app_is_portable_install(void);
/**
 * Return true if templates exist
 */
bool KKE_appdir_app_template_any(void);
bool KKE_appdir_app_template_id_search(const char *app_template, char *path, size_t path_len)
  ATTR_NONNULL(1);
bool KKE_appdir_app_template_has_userpref(const char *app_template) ATTR_NONNULL(1);
void KKE_appdir_app_templates(struct ListBase *templates) ATTR_NONNULL(1);

/**
 * Initialize path to program executable.
 */
void KKE_appdir_program_path_init(const char *argv0) ATTR_NONNULL(1);

/**
 * Path to executable
 */
const char *KKE_appdir_program_path(void) ATTR_WARN_UNUSED_RESULT ATTR_RETURNS_NONNULL;
/**
 * Path to directory of executable
 */
const char *KKE_appdir_program_dir(void) ATTR_WARN_UNUSED_RESULT ATTR_RETURNS_NONNULL;

/**
 * Gets a good default directory for fonts.
 */
bool KKE_appdir_font_folder_default(char *dir);

/**
 * Find Python executable.
 */
bool KKE_appdir_program_python_search(char *fullpath,
                                      size_t fullpath_len,
                                      int version_major,
                                      int version_minor) ATTR_NONNULL(1);

/**
 * Initialize path to temporary directory.
 */
void KKE_tempdir_init(const char *userdir);

/**
 * Path to persistent temporary directory (with trailing slash)
 */
const char *KKE_tempdir_base(void) ATTR_WARN_UNUSED_RESULT ATTR_RETURNS_NONNULL;
/**
 * Path to temporary directory (with trailing slash)
 */
const char *KKE_tempdir_session(void) ATTR_WARN_UNUSED_RESULT ATTR_RETURNS_NONNULL;
/**
 * Delete content of this instance's temp dir.
 */
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

/* for KKE_appdir_resource_path_id_with_version only */
enum
{
  KRAKEN_RESOURCE_PATH_USER = 0,
  KRAKEN_RESOURCE_PATH_LOCAL = 1,
  KRAKEN_RESOURCE_PATH_SYSTEM = 2,
};

#define KRAKEN_STARTUP_FILE "startup.usda"
#define KRAKEN_USERPREF_FILE "userpref.usda"
#define KRAKEN_QUIT_FILE "quit.usda"
#define KRAKEN_BOOKMARK_FILE "bookmarks.txt"
#define KRAKEN_HISTORY_FILE "recent-files.txt"
#define KRAKEN_PLATFORM_SUPPORT_FILE "platform_support.txt"

#ifdef __cplusplus
}
#endif
