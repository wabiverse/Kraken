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

#ifndef KRAKEN_KERNEL_MAIN_H
#define KRAKEN_KERNEL_MAIN_H

#include "USD_listBase.h"

#include "KLI_compiler_attrs.h"
#include "KLI_sys_types.h"

#define FILE_MAXDIR 768
#define FILE_MAXFILE 256
#define FILE_MAX 1024

#ifdef __cplusplus
extern "C" {
#endif

struct KLI_mempool;
struct KrakenThumbnail;
struct RHash;
struct RSet;
struct IDNameLib_Map;
struct ImBuf;
struct Library;
struct MainLock;
struct UniqueName_Map;

typedef struct Main
{
  uint64_t build_commit_timestamp;
  char build_hash[FILE_MAX];

  char exe_path[FILE_MAX];
  char fonts_path[FILE_MAX];
  char temp_dir[FILE_MAX];
  char icons_path[FILE_MAX];
  char python_path[FILE_MAX];
  char datafiles_path[FILE_MAX];

  char stage_id[FILE_MAX];
  char ocio_cfg[FILE_MAX];

  char kraken_version_decimal[32];

  char launch_time[80];

  ListBase materials; /* for shaders. */
  ListBase objects;   /* for shaders. */
  ListBase screens;
  ListBase scenes;
  ListBase wm;
  ListBase workspaces;
} Main;

struct Main *KKE_main_new(void);
const char *KKE_main_usdfile_path(const struct Main *kmain);

enum kkeStatusCode
{
  KRAKEN_SUCCESS = 0,
  KRAKEN_ERROR,
};

struct Main KKE_main_init(void);
void KKE_main_free(struct Main *mainvar);
void KKE_kraken_free(void);

void KKE_kraken_atexit(void);
void KKE_kraken_atexit_register(void (*func)(void *user_data), void *user_data);
void KKE_kraken_atexit_unregister(void (*func)(void *user_data), const void *user_data);

void KKE_kraken_main_init(struct kContext *C);
void KKE_kraken_plugins_init(void);
void KKE_kraken_enable_debug_codes(void);

const char *KKE_kraken_version_string(void);

#ifdef __cplusplus
}
#endif

#endif /* KRAKEN_KERNEL_MAIN_H */