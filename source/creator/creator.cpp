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
 * Creator.
 * Creating Chaos.
 */

#ifdef _WIN32
#  include "pch.h"
#  include "winrt/Kraken.h"

#  include "ChaosEngine/Kraken.Foundation.App.h"
#  ifdef WITH_WINUI3
#    include "ChaosEngine/Kraken.UIKit.UIScreen.h"
#  endif /* WITH_WINUI3 */
#endif   /* _WIN32 */

#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#  include "utfconv.h"
#  include <windows.h>
#endif

#if defined(WITH_TBB_MALLOC) && defined(_MSC_VER) && defined(NDEBUG)
#  pragma comment(lib, "tbbmalloc_proxy.lib")
#  pragma comment(linker, "/include:__TBB_malloc_proxy")
#endif

#include "MEM_guardedalloc.h"

#include "CLG_log.h"

#include "KLI_string.h"
#include "KLI_system.h"
#include "KLI_task.h"
#include "KLI_threads.h"
#include "KLI_utildefines.h"

#include "KKE_appdir.h"
#include "KKE_context.h"
#include "KKE_global.h"
#include "KKE_main.h"

#include "ED_datafiles.h"

#include "USD_ID.h"
#include "USD_pixar_utils.h"

#include "WM_api.h"
#include "WM_init_exit.h"
#include "WM_window.hh"

#include "IMB_imbuf.h"

#include "LUXO_define.h"
#include "LUXO_access.h"

#include <signal.h>

#ifdef __FreeBSD__
#  include <floatingpoint.h>
#endif

#include "creator.h" /* own include. */

#if defined(ARCH_OS_WINDOWS)
using namespace winrt;
using namespace winrt::Windows::ApplicationModel;
using namespace winrt::Windows::ApplicationModel::Activation;
using namespace winrt::Windows::Foundation;
using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Microsoft::UI::Xaml::Controls;

#  ifdef WITH_WINUI3
using namespace winrt::Microsoft::UI::Xaml::Navigation;
#  endif /* WITH_WINUI3 */

using namespace Kraken;
using namespace Kraken::implementation;
#endif /* defined(ARCH_OS_WINDOWS) */

KRAKEN_NAMESPACE_USING

/* -------------------------------------------------------------------- */
/** @name Local Defines
 * @{ */

/* When building as a Python module, don't use special argument handling
 * so the module loading logic can control the `argv` & `argc`. */
#if defined(WIN32) && !defined(WITH_PYTHON_MODULE)
#  define USE_WIN32_UNICODE_ARGS
#endif

/** @} */

/* -------------------------------------------------------------------- */
/** @name Local Application State
 * @{ */

/* written to by 'creator_args.cpp' */

/* clang-format off */
struct ApplicationState app_state = {
  .signal = {
    .use_crash_handler = true,
    .use_abort_handler = true,
  },
  .exit_code_on_error =
  {
    .python = 0,
  },
};
/* clang-format on */

/** @} */

/* -------------------------------------------------------------------- */
/** @name Application Level Callbacks
 *
 * Initialize callbacks for the modules that need them.
 * @{ */

static void callback_mem_error(const char *errorStr)
{
  fputs(errorStr, stderr);
  fflush(stderr);
}

static void main_callback_setup(void)
{
  /* Error output from the guarded allocation routines. */
  MEM_set_error_callback(callback_mem_error);
}

/* free data on early exit (if Python calls 'sys.exit()' while parsing args for eg). */
struct CreatorAtExitData
{
#ifndef WITH_PYTHON_MODULE
  kArgs *ka;
#endif

#ifdef USE_WIN32_UNICODE_ARGS
  const char **argv;
  int argv_num;
#endif

#if defined(WITH_PYTHON_MODULE) && !defined(USE_WIN32_UNICODE_ARGS)
  void *_empty; /* Prevent empty struct error with MSVC. */
#endif
};

static void callback_main_atexit(void *user_data)
{
  struct CreatorAtExitData *app_init_data = static_cast<CreatorAtExitData *>(user_data);

#ifndef WITH_PYTHON_MODULE
  if (app_init_data->ka) {
    CREATOR_args_destroy(app_init_data->ka);
    app_init_data->ka = nullptr;
  }
#else
  UNUSED_VARS(app_init_data); /* May be unused. */
#endif

#ifdef USE_WIN32_UNICODE_ARGS
  if (app_init_data->argv) {
    while (app_init_data->argv_num) {
      free((void *)app_init_data->argv[--app_init_data->argv_num]);
    }
    free((void *)app_init_data->argv);
    app_init_data->argv = NULL;
  }
#else
  UNUSED_VARS(app_init_data); /* May be unused. */
#endif
}

static void callback_clg_fatal(void *fp)
{
  KLI_system_backtrace(static_cast<FILE *>(fp));
}

/** @} */

/* -------------------------------------------------------------------- */
/** @name Kraken as a Stand-Alone Python Module (kpy)
 *
 * While not officially supported, this can be useful for Python developers.
 * See: https://wiki.blender.org/wiki/Building_Blender/Other/BlenderAsPyModule
 * @{ */

#ifdef WITH_PYTHON_MODULE

/* Called in `kpy_interface.cpp` when building as a Python module. */
int main_python_enter(int argc, const char **argv);
void main_python_exit(void);

/* Rename the 'main' function, allowing Python initialization to call it. */
#  define main main_python_enter
static void *evil_C = NULL;

#  ifdef __APPLE__
/* Environment is not available in macOS shared libraries. */
#    include <crt_externs.h>
char **environ = NULL;
#  endif /* __APPLE__ */

#endif /* WITH_PYTHON_MODULE */

/** @} */

/* -------------------------------------------------------------------- */
/** @name GMP Allocator Workaround
 * @{ */

#if (defined(WITH_TBB_MALLOC) && defined(_MSC_VER) && defined(NDEBUG) && defined(WITH_GMP)) || \
  defined(DOXYGEN)
#  include "gmp.h"
#  include "tbb/scalable_allocator.h"

void *gmp_alloc(size_t size)
{
  return scalable_malloc(size);
}
void *gmp_realloc(void *ptr, size_t old_size, size_t new_size)
{
  return scalable_realloc(ptr, new_size);
}

void gmp_free(void *ptr, size_t size)
{
  scalable_free(ptr);
}
/**
 * Use TBB's scalable_allocator on Windows.
 * `TBBmalloc` correctly captures all allocations already,
 * however, GMP is built with MINGW since it doesn't build with MSVC,
 * which TBB has issues hooking into automatically.
 */
void gmp_kraken_init_allocator()
{
  mp_set_memory_functions(gmp_alloc, gmp_realloc, gmp_free);
}
#endif

/** @} */

/* -------------------------------------------------------------------- */
/** @name Main Function
 * @{ */

/**
 * Kraken's main function responsibilities are:
 * - setup subsystems.
 * - handle arguments.
 * - run #WM_main() event loop,
 *   or exit immediately when running in background-mode.
 */
int CREATOR_kraken_main(int argc,
#ifdef USE_WIN32_UNICODE_ARGS
                        const char **UNUSED(argv_c)
#else
                        const char **argv
#endif
)
{
  kContext *C;

#ifndef WITH_PYTHON_MODULE
  kArgs *ka;
#endif

#ifdef USE_WIN32_UNICODE_ARGS
  char **argv;
  int argv_num;
#endif

  /* Ensure we free data on early-exit. */
  struct CreatorAtExitData app_init_data = {NULL};
  KKE_kraken_atexit_register(callback_main_atexit, &app_init_data);

  /* Un-buffered `stdout` makes `stdout` and `stderr` better synchronized, and helps
   * when stepping through code in a debugger (prints are immediately
   * visible). However disabling buffering causes lock contention on windows
   * see T76767 for details, since this is a debugging aid, we do not enable
   * the un-buffered behavior for release builds. */
#ifndef NDEBUG
  setvbuf(stdout, NULL, _IONBF, 0);
#endif

#ifdef WIN32
  /* We delay loading of OPENMP so we can set the policy here. */
#  if defined(_MSC_VER)
  _putenv_s("OMP_WAIT_POLICY", "PASSIVE");
#  endif

#  ifdef USE_WIN32_UNICODE_ARGS
  /* Win32 Unicode Arguments. */
  {
    /* NOTE: Can't use `guardedalloc` allocation here, as it's not yet initialized
     * (it depends on the arguments passed in, which is what we're getting here!) */
    wchar_t **argv_16 = CommandLineToArgvW(GetCommandLineW(), &argc);
    argv = malloc(argc * sizeof(char *));
    for (argv_num = 0; argv_num < argc; argv_num++) {
      argv[argv_num] = alloc_utf_8_from_16(argv_16[argv_num], 0);
    }
    LocalFree(argv_16);

    /* free on early-exit */
    app_init_data.argv = argv;
    app_init_data.argv_num = argv_num;
  }
#  endif /* USE_WIN32_UNICODE_ARGS */
#endif   /* WIN32 */

  /* NOTE: Special exception for guarded allocator type switch:
   *       we need to perform switch from lock-free to fully
   *       guarded allocator before any allocation happened.
   */
  {
    int i;
    for (i = 0; i < argc; i++) {
      if (STR_ELEM(argv[i], "-d", "--debug", "--debug-memory", "--debug-all")) {
        printf("Switching to fully guarded memory allocator.\n");
        MEM_use_guarded_allocator();
        break;
      }
      if (STREQ(argv[i], "--")) {
        break;
      }
    }
    MEM_init_memleak_detection();
  }

#ifdef BUILD_DATE
  {
    time_t temp_time = build_commit_timestamp;
    struct tm *tm = gmtime(&temp_time);
    if (LIKELY(tm)) {
      strftime(build_commit_date, sizeof(build_commit_date), "%Y-%m-%d", tm);
      strftime(build_commit_time, sizeof(build_commit_time), "%H:%M", tm);
    } else {
      const char *unknown = "date-unknown";
      KLI_strncpy(build_commit_date, unknown, sizeof(build_commit_date));
      KLI_strncpy(build_commit_time, unknown, sizeof(build_commit_time));
    }
  }
#endif

  /* Initialize logging. */
  CLG_init();
  CLG_fatal_fn_set(callback_clg_fatal);

  /* Create Context C. */
  C = CTX_create();

#ifdef WITH_PYTHON_MODULE
#  ifdef __APPLE__
  environ = *_NSGetEnviron();
#  endif

#  undef main
  evil_C = C;
#endif

#if defined(WITH_TBB_MALLOC) && defined(_MSC_VER) && defined(NDEBUG) && defined(WITH_GMP)
  gmp_kraken_init_allocator();
#endif

  main_callback_setup();

#if defined(__APPLE__) && !defined(WITH_PYTHON_MODULE) && !defined(WITH_HEADLESS)
  /* Patch to ignore argument finder gives us (PID?) */
  if (argc == 2 && STRPREFIX(argv[1], "-psn_")) {
    extern int ANCHOR_HACK_getFirstFile(char buf[]);
    static char firstfilebuf[512];

    argc = 1;

    if (ANCHOR_HACK_getFirstFile(firstfilebuf)) {
      argc = 2;
      argv[1] = firstfilebuf;
    }
  }
#endif

#ifdef __FreeBSD__
  fpsetmask(0);
#endif

  /* Initialize path to executable. */
  KKE_appdir_program_path_init(argv[0]);

  /* Initialize Threads. */
  KLI_threadapi_init();

  /* Initialize Globals (paths, sys). */
  KKE_kraken_globals_init();

  KKE_callback_global_init();

  /* First test for background-mode (#Global.background) */
#ifndef WITH_PYTHON_MODULE
  /* ------------ skip binary path ------------------ */
  ka = CREATOR_args_create(argc, (const char **)argv);

  /* Ensure we free on early exit. */
  app_init_data.ka = ka;

  CREATOR_args_setup(C, ka);

  /* Begin argument parsing, ignore leaks so arguments that call #exit
   * (such as '--version' & '--help') don't report leaks. */
  MEM_use_memleak_detection(false);

  /* Parse environment handling arguments. */
  CREATOR_args_parse(ka, ARG_PASS_ENVIRONMENT, NULL, NULL);

#else
  /* Using preferences or user startup makes no sense for #WITH_PYTHON_MODULE. */
  G.factory_startup = true;
#endif
  /* After parsing #ARG_PASS_ENVIRONMENT such as `--env-*`,
   * since they impact `KKE_appdir` behavior. */
  KKE_appdir_init();

  /* After parsing number of threads argument. */
  KLI_task_scheduler_init();

  /* Initialize sub-systems that use `KKE_appdir.h`. */
  IMB_init();

#ifndef WITH_PYTHON_MODULE
  /* First test for background-mode (#Global.background) */
  CREATOR_args_parse(ka, ARG_PASS_SETTINGS, NULL, NULL);

  CREATOR_main_signal_setup();
#endif

  /* Init plugins, or you're not going to get very far here. */
  KKE_kraken_plugins_init();

  /* Setup and create all root level prims. */
  LUXO_init();

  /* This sets the context stage. */
  LUXO_set_stage_ctx(C);

  /**
   * @TODO: Render engine init goes here
   *
   * ... # RE_engines_init() */

#if defined(WITH_PYTHON_MODULE) || defined(WITH_HEADLESS)
  /* Python module mode ALWAYS runs in background-mode (for now). */
  G.background = true;
#else
  if (G.background) {
    CREATOR_main_signal_setup_background();
  }
#endif

#ifndef WITH_PYTHON_MODULE
  if (G.background == 0) {
    CREATOR_args_parse(ka, ARG_PASS_SETTINGS_GUI, NULL, NULL);
  }
  CREATOR_args_parse(ka, ARG_PASS_SETTINGS_FORCE, NULL, NULL);
#endif

  /* Initialize main Runtime. */
  WM_init(C, argc, (const char **)argv);

  /* Need to be after WM init so that userpref are loaded. */
  // RE_engines_init_experimental();

#ifndef WITH_PYTHON
  printf(
    "\n* WARNING * - Kraken compiled without Python!\n"
    "this is not intended for typical usage\n\n");
#endif

  /* Initialize kraken python module. */
  CTX_py_init_set(C, true);
  // WM_keyconfig_init(C);

  /* OK we are ready for it */
#ifndef WITH_PYTHON_MODULE
  /* Handles #ARG_PASS_FINAL. */
  CREATOR_args_setup_post(C, ka);
#endif

  /* Explicitly free data allocated for argument parsing:
   * - 'ka'
   * - 'argv' on WIN32.
   */
  callback_main_atexit(&app_init_data);
  KKE_kraken_atexit_unregister(callback_main_atexit, &app_init_data);

  /* End argument parsing, allow memory leaks to be printed. */
  MEM_use_memleak_detection(true);

  /* Paranoid, avoid accidental re-use. */
#ifndef WITH_PYTHON_MODULE
  ka = nullptr;
  (void)ka;
#endif

#ifdef USE_WIN32_UNICODE_ARGS
  argv = NULL;
  (void)argv;
#endif

#ifndef WITH_PYTHON_MODULE
  if (G.background) {
    /* Using window-manager API in background-mode is a bit odd, but works fine. */
    WM_exit(C);
  } else {
    /* When no file is loaded, show the splash screen. */
    const char *stagefile_path = KKE_main_usdfile_path(G_MAIN);
    if (stagefile_path[0] == '\0') {
      /**
       * @TODO: Splash screen.
       *
       * WM_init_splash(C);
       */
    }

    /* Run the main event loop. */
    WM_main(C);
  }
#endif /* WITH_PYTHON_MODULE */

  return KRAKEN_GODSPEED; /* -> the end. */
}

#ifdef WITH_PYTHON_MODULE
void main_python_exit(void)
{
  WM_exit_ex((kContext *)evil_C, true);
  evil_C = NULL;
}
#endif

#if defined(ARCH_OS_WINDOWS)

int __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
  /**
   * 🚀 Kraken Launch with Microsoft WinRT Superpowers. */

  winrt::init_apartment();
  Windows::UI::Xaml::Application::Start([](auto &&) {
    ::winrt::make<::winrt::Kraken::implementation::App>();
  });

  CREATOR_kraken_main();

  return KRAKEN_SUCCESS;
}

#else /* ARCH_OS_WINDOWS */

int main(int argc, const char **argv)
{
  return CREATOR_kraken_main(argc, argv);
}

#endif /* ARCH_OS_LINUX || ARCH_OS_DARWIN */
