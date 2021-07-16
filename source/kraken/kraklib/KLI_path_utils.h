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
 * KRAKEN Library.
 * Gadget Vault.
 */

#pragma once

#include "KLI_api.h"
#include "KLI_assert.h"
#include "KLI_string_utils.h"

#include <cstring>
#include <string>

#include <wabi/base/arch/attributes.h>

WABI_NAMESPACE_BEGIN

#ifdef _WIN32
#define MAXPATHLEN MAX_PATH
#endif /* _WIN32 */

const char *KLI_getenv(const char *env);

size_t KLI_path_join(char *__restrict dst, const size_t dst_len, const char *path, ...);

void KLI_join_dirfile(char *__restrict dst,
                      const size_t maxlen,
                      const char *__restrict dir,
                      const char *__restrict file);

bool KLI_path_extension_check(const char *str, const char *ext);
bool KLI_path_extension_check_n(const char *str, ...);
bool KLI_path_extension_check_array(const std::string &str, const char **ext_array);

void KLI_path_append(char *__restrict dst, const size_t maxlen, const char *__restrict file);

bool KLI_path_program_search(char *fullname, const size_t maxlen, const char *name);

const char *KLI_path_slash_rfind(const char *string);
void KLI_path_slash_rstrip(char *string);

bool KLI_has_pixar_extension(const std::string &str);

#ifdef _WIN32
bool KLI_path_program_extensions_add_win32(char *name, const size_t maxlen);
#endif /* _WIN32 */

/* path string comparisons: case-insensitive for Windows, case-sensitive otherwise */
#if defined(WIN32)
#  define KLI_path_cmp BLI_strcasecmp
#  define KLI_path_ncmp BLI_strncasecmp
#else
#  define KLI_path_cmp strcmp
#  define KLI_path_ncmp strncmp
#endif

/* also defined in UNI_space_types.h */
#ifndef FILE_MAXDIR
#  define FILE_MAXDIR 768
#  define FILE_MAXFILE 256
#  define FILE_MAX 1024
#endif

#ifdef WIN32
#  define SEP '\\'
#  define ALTSEP '/'
#  define SEP_STR "\\"
#  define ALTSEP_STR "/"
#else
#  define SEP '/'
#  define ALTSEP '\\'
#  define SEP_STR "/"
#  define ALTSEP_STR "\\"
#endif

/* Parent and current dir helpers. */
#define FILENAME_PARENT ".."
#define FILENAME_CURRENT "."

/* Avoid calling strcmp on one or two chars! */
#define FILENAME_IS_PARENT(_n) (((_n)[0] == '.') && ((_n)[1] == '.') && ((_n)[2] == '\0'))
#define FILENAME_IS_CURRENT(_n) (((_n)[0] == '.') && ((_n)[1] == '\0'))
#define FILENAME_IS_CURRPAR(_n) \
  (((_n)[0] == '.') && (((_n)[1] == '\0') || (((_n)[1] == '.') && ((_n)[2] == '\0'))))

#ifndef KLI_ISREG
#  define KLI_ISREG(x) fs::is_regular_file(x)
#endif
#ifndef KLI_ISDIR
#  define KLI_ISDIR(x) fs::is_directory(x)
#endif

WABI_NAMESPACE_END