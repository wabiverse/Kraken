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

struct Main : public KrakenPrim
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

  std::vector<struct wmWindowManager *> wm;
  std::vector<struct WorkSpace *> workspaces;
  std::vector<struct kScreen *> screens;
  std::vector<struct Scene *> scenes;
};

/** #Global.debug */
enum
{
  /* general debug flag, print more info in unexpected cases */
  G_DEBUG = (1 << 0),
  /* debug messages for ffmpeg */
  G_DEBUG_FFMPEG = (1 << 1),
  /* extra python info */
  G_DEBUG_PYTHON = (1 << 2),
  /* input/window/screen events */
  G_DEBUG_EVENTS = (1 << 3),
  /* events handling */
  G_DEBUG_HANDLERS = (1 << 4),
  /* operator, undo */
  G_DEBUG_WM = (1 << 5),
  /* jobs time profiling */
  G_DEBUG_JOBS = (1 << 6),
  /* freestyle messages */
  G_DEBUG_FREESTYLE = (1 << 7),
  /* UsdStage construction messages */
  G_DEBUG_STAGE_BUILD = (1 << 8),
  /* UsdStage evaluation messages */
  G_DEBUG_STAGE_EVAL = (1 << 9),
  /* UsdStage tagging messages */
  G_DEBUG_STAGE_TAG = (1 << 10),
  /* UsdStage timing statistics and messages */
  G_DEBUG_STAGE_TIME = (1 << 11),
  /* single threaded UsdStage */
  G_DEBUG_STAGE_NO_THREADS = (1 << 12),
  /* use pretty colors in UsdStage debug messages */
  G_DEBUG_STAGE_PRETTY = (1 << 13),
  /* UsdStage SdfPath and Asset Resolution messages */
  G_DEBUG_STAGE_PATHS = (1 << 14),
  G_DEBUG_STAGE = (G_DEBUG_STAGE_BUILD | G_DEBUG_STAGE_EVAL | G_DEBUG_STAGE_TAG |
                   G_DEBUG_STAGE_TIME | G_DEBUG_STAGE_PATHS),
  /* sim debug data display */
  G_DEBUG_SIMDATA = (1 << 15),
  /* gpu debug */
  G_DEBUG_GPU = (1 << 16),
  /* IO Debugging. */
  G_DEBUG_IO = (1 << 17),
  /* force gpu workarounds bypassing detections. */
  G_DEBUG_GPU_FORCE_WORKAROUNDS = (1 << 18),
  /* XR/OpenXR messages */
  G_DEBUG_XR = (1 << 19),
  /* XR/OpenXR timing messages */
  G_DEBUG_XR_TIME = (1 << 20),
  /* Debug ANCHOR module. */
  G_DEBUG_ANCHOR = (1 << 21),
};

#define G_DEBUG_ALL \
  (G_DEBUG | G_DEBUG_FFMPEG | G_DEBUG_PYTHON | G_DEBUG_EVENTS | G_DEBUG_WM | G_DEBUG_JOBS | \
   G_DEBUG_FREESTYLE | G_DEBUG_STAGE | G_DEBUG_IO | G_DEBUG_ANCHOR)

enum eGlobalFileFlags
{
  G_FILE_AUTOPACK = (1 << 0),
  G_FILE_COMPRESS = (1 << 1),
  G_FILE_NO_UI = (1 << 2),
  G_FILE_RECOVER_READ = (1 << 3),
  G_FILE_RECOVER_WRITE = (1 << 4),
};

struct Global
{
  Main *main;

  bool background;
  bool factory_startup;

  /**
   * Has escape been pressed or Ctrl+C pressed in background mode, used for render quit. */
  bool is_break;

  bool is_rendering;

  short debug_value;

  int f;

  /** 
   *   Debug Flag
   * 
   * - #G_DEBUG,
   * - #G_DEBUG_PYTHON & friends,
   * - set python or command line args */
  int debug;

  /** #eGlobalFileFlags */
  int fileflags;

  /** Message to use when auto execution fails. */
  char autoexec_fail[200];
};

enum eGlobalFlag
{
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
void KKE_main_free(Main *mainvar);
void KKE_kraken_free(void);

void KKE_kraken_atexit(void);
void KKE_kraken_atexit_register(void (*func)(void *user_data), void *user_data);
void KKE_kraken_atexit_unregister(void (*func)(void *user_data), const void *user_data);

void KKE_kraken_main_init(kContext *C, int argc, const char **argv);
void KKE_kraken_globals_init();
void KKE_kraken_plugins_init(void);
void KKE_kraken_python_init(kContext *C);
ckeStatusCode KKE_main_runtime(int backend);
void KKE_kraken_enable_debug_codes(void);

const char *KKE_kraken_version_string(void);

/* ------ */


/* Setup in KKE_kraken. */
extern Global G;

/**
 * Setup in KKE_kraken.
 * TODO: Move these into
 * UserDef. */
extern float UI_DPI_FAC;
extern float UI_PRESSURE_SOFTNESS;
extern float UI_PRESSURE_THRESHOLD_MAX;
extern int UI_FLAG;
extern bool UI_MOUSE_EMULATE_3BUTTON_MODIFIER;
extern int UI_DRAG_THRESHOLD_MOUSE;
extern int UI_DRAG_THRESHOLD_TABLET;
extern int UI_DRAG_THRESHOLD;
extern short UI_DOUBLE_CLICK_TIME;

WABI_NAMESPACE_END

#endif /* KRAKEN_KERNEL_MAIN_H */