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

struct kContext;
struct KLI_mempool;
struct KrakenThumbnail;
struct RHash;
struct RSet;
struct IDNameLib_Map;
struct ImBuf;
struct Library;
struct MainLock;
struct UniqueName_Map;

/* Kraken thumbnail, as written on file (width, height, and data as char RGBA). */
/* We pack pixel data after that struct. */
typedef struct KrakenThumbnail {
  int width, height;
  char rect[0];
} KrakenThumbnail;

typedef struct Main
{
  char stage_id[1024];
  short versionfile, subversionfile;
  short minversionfile, minsubversionfile;

  uint64_t build_commit_timestamp;
  char build_hash[16];

  bool recovered;
  bool is_memfile_undo_written;
  bool is_memfile_undo_flush_needed;
  bool use_memfile_full_barrier;
  bool is_locked_for_linking;

  /**
   * True if this main is the 'GMAIN' of current Kraken.
   *
   * @note There should always be only one global main, all others generated temporarily for
   * various data management process must have this property set to false..
   */
  bool is_global_main;

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

/* because it ends cool. */
#define KRAKEN_GODSPEED 0
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

void KKE_kraken_plugins_init(void);
void KKE_kraken_enable_debug_codes(void);

const char *KKE_kraken_version_string(void);

#ifdef __cplusplus
}
#endif

#endif /* KRAKEN_KERNEL_MAIN_H */