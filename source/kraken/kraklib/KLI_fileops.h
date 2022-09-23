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

#include "KLI_api.h"
#include "KLI_compiler_attrs.h"

#if defined(WIN32)
typedef unsigned int mode_t;
#endif

#define FILELIST_DIRENTRY_SIZE_LEN 16
#define FILELIST_DIRENTRY_MODE_LEN 4
#define FILELIST_DIRENTRY_OWNER_LEN 16
#define FILELIST_DIRENTRY_TIME_LEN 8
#define FILELIST_DIRENTRY_DATE_LEN 16

#ifndef PATH_MAX
#  define PATH_MAX 1024
#endif


KRAKEN_NAMESPACE_BEGIN


FILE *KLI_fopen(const char *filepath, const char *mode);

void *KLI_gzopen(const char *filepath, const char *mode);

int KLI_open(const char *filepath, int oflag, int pmode);

int KLI_access(const char *filepath, int mode);

int KLI_delete(const char *file, bool dir, bool recursive);


/* Cross Platform Implementations (Seperate by preprocessors) */
bool KLI_dir_create_recursive(const char *dirname);

bool KLI_exists(const fs::path &path);
fs::file_status KLI_type(const fs::path &path);

#ifdef WIN32
#  if defined(_MSC_VER)
typedef struct _stat64 KLI_stat_t;
#  else
typedef struct _stat KLI_stat_t;
#  endif
#else
typedef struct stat KLI_stat_t;
#endif

int KLI_fstat(int fd, KLI_stat_t *buffer) ATTR_WARN_UNUSED_RESULT ATTR_NONNULL();
int KLI_stat(const char *path, KLI_stat_t *buffer) ATTR_WARN_UNUSED_RESULT ATTR_NONNULL();
int64_t KLI_ftell(FILE *stream) ATTR_WARN_UNUSED_RESULT ATTR_NONNULL();
int KLI_fseek(FILE *stream, int64_t offset, int whence);
int64_t KLI_lseek(int fd, int64_t offset, int whence);

#ifdef WIN32
int KLI_wstat(const wchar_t *path, KLI_stat_t *buffer);
#endif

void KLI_filelist_entry_free(struct direntry *entry);
void KLI_filelist_free(struct direntry *filelist, const unsigned int nrentries);
unsigned int KLI_filelist_dir_contents(const char *dirname, struct direntry **r_filelist);

bool KLI_is_dir(const char *file);
bool KLI_is_file(const char *path);

struct direntry
{
  mode_t type;
  const char *relname;
  const char *path;
#ifdef WIN32 /* keep in sync with the definition of KLI_stat_t in KLI_fileops.h */
#  if defined(_MSC_VER)
  struct _stat64 s;
#  else
  struct _stat s;
#  endif
#else
  // struct KLI_stat_t s;
#endif
};

struct dirlink
{
  struct dirlink *next, *prev;
  char *name;
};

#ifdef __APPLE__
/**
 * Expand the leading `~` in the given path to `/Users/$USER`.
 * This doesn't preserve the trailing path separator.
 * Giving a path without leading `~` is not an error.
 */
const char *KLI_expand_tilde(const char *path_with_tilde);
#endif
/* This weirdo pops up in two places. */
#if !defined(WIN32)
#  ifndef O_BINARY
#    define O_BINARY 0
#  endif
#else
void KLI_get_short_name(char short_name[256], const char *filepath);
#endif

KRAKEN_NAMESPACE_END