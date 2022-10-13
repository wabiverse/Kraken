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

#if defined(__linux__) && defined(__GNUC__)
#  define _GNU_SOURCE
#  include <fenv.h>
#endif

#if (defined(__APPLE__) && (defined(__i386__) || defined(__x86_64__)))
#  define OSX_SSE_FPE
#  include <xmmintrin.h>
#endif

#ifdef WIN32
#  include <float.h>
#  include <windows.h>
#endif

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "KLI_sys_types.h"

#ifdef WIN32
#  include "KLI_winstuff.h"
#endif

#include "MEM_guardedalloc.h"

#include "CLG_log.h"

#include "KLI_fileops.h"
#include "KLI_fileops.hh"
#include "KLI_kraklib.h"
#include "KLI_listbase.h"
#include "KLI_mempool.h"
#include "KLI_path_utils.h"
#include "KLI_rhash.h"
#include "KLI_system.h"
#include "KLI_threads.h"
#include "KLI_utildefines.h"
#include KLI_SYSTEM_PID_H

#include "KKE_appdir.h"
#include "KKE_context.h"
#include "KKE_main.h"
#include "KKE_report.h"
#include "KKE_global.h"

#include <signal.h>

#include "WM_api.h"
#include "WM_files.h"
#include "WM_init_exit.h"
#include "WM_window.h"

#include "USD_api.h"
#include "USD_pixar_utils.h"

#include "creator.h"

/* KRAKEN PYTHON */
#include "KPY_extern_run.h"
#include "KPY_extern_python.h"

#include "IMB_imbuf.h"

#include "DRW_engine.h"

/* PIXAR */
#include <wabi/base/tf/callContext.h>
#include <wabi/base/tf/diagnostic.h>
#include <wabi/base/tf/iterator.h>
#include <wabi/base/plug/registry.h>

#include <wabi/usd/usd/stage.h>

#include <wabi/imaging/hio/imageRegistry.h>

#include <wabi/usdImaging/usdImagingGL/engine.h>

#include <map>
#include <string>
#include <vector>
#include <filesystem>

KRAKEN_NAMESPACE_USING

struct KrakenPyContextStore
{
  wmWindowManager *wm;
  kScene *scene;
  wmWindow *win;
  bool has_win;
};

static void arg_py_context_backup(kContext *C,
                                  struct KrakenPyContextStore *c_py,
                                  const char *script_id)
{
  c_py->wm = CTX_wm_manager(C);
  c_py->scene = CTX_data_scene(C);
  c_py->has_win = (VALUE_PTR(c_py->wm->windows.begin()) != nullptr);
  if (c_py->has_win) {
    c_py->win = CTX_wm_window(C);
    CTX_wm_window_set(C, VALUE_PTR(c_py->wm->windows.begin()));
  } else {
    c_py->win = NULL;
    fprintf(stderr,
            "Python script \"%s\" "
            "running with missing context data.\n",
            script_id);
  }
}

static void arg_py_context_restore(kContext *C, struct KrakenPyContextStore *c_py)
{
  /* script may load a file, check old data is valid before using */
  if (c_py->has_win) {
    if ((c_py->win == NULL) || ((KLI_findindex(&G_MAIN->wm, c_py->wm) != -1))) {
      CTX_wm_window_set(C, c_py->win);
    }
  }

  if ((c_py->scene == NULL) || KLI_findindex(&G_MAIN->scenes, c_py->scene) != -1) {
    CTX_data_scene_set(C, c_py->scene);
  }
}

/* macro for context setup/reset */
#define KPY_CTX_SETUP(_cmd)                   \
  {                                           \
    struct KrakenPyContextStore py_c;         \
    arg_py_context_backup(C, &py_c, argv[1]); \
    {                                         \
      _cmd;                                   \
    }                                         \
    arg_py_context_restore(C, &py_c);         \
  }                                           \
  ((void)0)

/* -------------------------------------------------------------------- */
/** \name Utility String Parsing
 * \{ */

static bool parse_int_relative(const char *str,
                               const char *str_end_test,
                               int pos,
                               int neg,
                               int *r_value,
                               const char **r_err_msg)
{
  char *str_end = NULL;
  long value;

  errno = 0;

  switch (*str) {
    case '+':
      value = pos + strtol(str + 1, &str_end, 10);
      break;
    case '-':
      value = (neg - strtol(str + 1, &str_end, 10)) + 1;
      break;
    default:
      value = strtol(str, &str_end, 10);
      break;
  }

  if (*str_end != '\0' && (str_end != str_end_test)) {
    static const char *msg = "not a number";
    *r_err_msg = msg;
    return false;
  }
  if ((errno == ERANGE) || ((value < INT_MIN) || (value > INT_MAX))) {
    static const char *msg = "exceeds range";
    *r_err_msg = msg;
    return false;
  }
  *r_value = (int)value;
  return true;
}

static const char *parse_int_range_sep_search(const char *str, const char *str_end_test)
{
  const char *str_end_range = NULL;
  if (str_end_test) {
    str_end_range = static_cast<const char *>(memchr(str, '.', (str_end_test - str) - 1));
    if (str_end_range && (str_end_range[1] != '.')) {
      str_end_range = NULL;
    }
  } else {
    str_end_range = strstr(str, "..");
    if (str_end_range && (str_end_range[2] == '\0')) {
      str_end_range = NULL;
    }
  }
  return str_end_range;
}

/**
 * Parse a number as a range, eg: `1..4`.
 *
 * The \a str_end_range argument is a result of #parse_int_range_sep_search.
 */
static bool parse_int_range_relative(const char *str,
                                     const char *str_end_range,
                                     const char *str_end_test,
                                     int pos,
                                     int neg,
                                     int r_value_range[2],
                                     const char **r_err_msg)
{
  if (parse_int_relative(str, str_end_range, pos, neg, &r_value_range[0], r_err_msg) &&
      parse_int_relative(str_end_range + 2,
                         str_end_test,
                         pos,
                         neg,
                         &r_value_range[1],
                         r_err_msg)) {
    return true;
  }
  return false;
}

static bool parse_int_relative_clamp(const char *str,
                                     const char *str_end_test,
                                     int pos,
                                     int neg,
                                     int min,
                                     int max,
                                     int *r_value,
                                     const char **r_err_msg)
{
  if (parse_int_relative(str, str_end_test, pos, neg, r_value, r_err_msg)) {
    CLAMP(*r_value, min, max);
    return true;
  }
  return false;
}

static bool parse_int_range_relative_clamp(const char *str,
                                           const char *str_end_range,
                                           const char *str_end_test,
                                           int pos,
                                           int neg,
                                           int min,
                                           int max,
                                           int r_value_range[2],
                                           const char **r_err_msg)
{
  if (parse_int_range_relative(str,
                               str_end_range,
                               str_end_test,
                               pos,
                               neg,
                               r_value_range,
                               r_err_msg)) {
    CLAMP(r_value_range[0], min, max);
    CLAMP(r_value_range[1], min, max);
    return true;
  }
  return false;
}

/**
 * No clamping, fails with any number outside the range.
 */
static bool parse_int_strict_range(const char *str,
                                   const char *str_end_test,
                                   const int min,
                                   const int max,
                                   int *r_value,
                                   const char **r_err_msg)
{
  char *str_end = NULL;
  long value;

  errno = 0;
  value = strtol(str, &str_end, 10);

  if (*str_end != '\0' && (str_end != str_end_test)) {
    static const char *msg = "not a number";
    *r_err_msg = msg;
    return false;
  }
  if ((errno == ERANGE) || ((value < min) || (value > max))) {
    static const char *msg = "exceeds range";
    *r_err_msg = msg;
    return false;
  }
  *r_value = (int)value;
  return true;
}

static bool parse_int(const char *str,
                      const char *str_end_test,
                      int *r_value,
                      const char **r_err_msg)
{
  return parse_int_strict_range(str, str_end_test, INT_MIN, INT_MAX, r_value, r_err_msg);
}

static bool parse_int_clamp(const char *str,
                            const char *str_end_test,
                            int min,
                            int max,
                            int *r_value,
                            const char **r_err_msg)
{
  if (parse_int(str, str_end_test, r_value, r_err_msg)) {
    CLAMP(*r_value, min, max);
    return true;
  }
  return false;
}

/**
 * Version of #parse_int_relative_clamp & #parse_int_range_relative_clamp
 * that parses a comma separated list of numbers.
 *
 * \note single values are evaluated as a range with matching start/end.
 */
static int (*parse_int_range_relative_clamp_n(const char *str,
                                              int pos,
                                              int neg,
                                              int min,
                                              int max,
                                              int *r_value_len,
                                              const char **r_err_msg))[2]
{
  const char sep = ',';
  int len = 1;
  for (int i = 0; str[i]; i++) {
    if (str[i] == sep) {
      len++;
    }
  }

  int(*values)[2] = static_cast<int(*)[2]>(MEM_mallocN(sizeof(*values) * len, __func__));
  int i = 0;
  while (true) {
    const char *str_end_range;
    const char *str_end = strchr(str, sep);
    if (ELEM(*str, sep, '\0')) {
      static const char *msg = "incorrect comma use";
      *r_err_msg = msg;
      goto fail;
    } else if ((str_end_range = parse_int_range_sep_search(str, str_end)) ?
                 parse_int_range_relative_clamp(str,
                                                str_end_range,
                                                str_end,
                                                pos,
                                                neg,
                                                min,
                                                max,
                                                values[i],
                                                r_err_msg) :
                 parse_int_relative_clamp(str,
                                          str_end,
                                          pos,
                                          neg,
                                          min,
                                          max,
                                          &values[i][0],
                                          r_err_msg)) {
      if (str_end_range == NULL) {
        values[i][1] = values[i][0];
      }
      i++;
    } else {
      goto fail; /* error message already set */
    }

    if (str_end) { /* next */
      str = str_end + 1;
    } else { /* finished */
      break;
    }
  }

  *r_value_len = i;
  return values;

fail:
  MEM_freeN(values);
  return NULL;
}

static char NO_DOCS[] = "NO DOCUMENTATION SPECIFIED";

struct kArgDoc;
typedef struct kArgDoc
{
  struct kArgDoc *next, *prev;
  const char *short_arg;
  const char *long_arg;
  const char *documentation;
  bool done;
} kArgDoc;

typedef struct kAKey
{
  const char *arg;
  uintptr_t pass; /* cast easier */
  int case_str;   /* case specific or not */
} kAKey;

typedef struct kArgument
{
  kAKey *key;
  KA_ArgCallback func;
  void *data;
  kArgDoc *doc;
} kArgument;

struct kArgs
{
  ListBase docs;
  RHash *items;
  int argc;
  const char **argv;
  int *passes;

  /* Only use when initializing arguments. */
  int current_pass;
};

static uint case_strhash(const void *ptr)
{
  const char *s = static_cast<const char *>(ptr);
  uint i = 0;
  uchar c;

  while ((c = tolower(*s++))) {
    i = i * 37 + c;
  }

  return i;
}

static uint keyhash(const void *ptr)
{
  const kAKey *k = static_cast<const kAKey *>(ptr);
  return case_strhash(k->arg); /* ^ KLI_rhashutil_inthash((void *)k->pass); */
}

static bool keycmp(const void *a, const void *b)
{
  const kAKey *ka = static_cast<const kAKey *>(a);
  const kAKey *kb = static_cast<const kAKey *>(b);
  /* -1 is wildcard for pass */
  if (ka->pass == kb->pass || ka->pass == -1 || kb->pass == -1) {
    if (ka->case_str == 1 || kb->case_str == 1) {
      return (KLI_strcasecmp(ka->arg, kb->arg) != 0);
    }
    return !STREQ(ka->arg, kb->arg);
  }
  return KLI_rhashutil_intcmp((const void *)ka->pass, (const void *)kb->pass);
}

static kArgument *lookUp(struct kArgs *ka, const char *arg, int pass, int case_str)
{
  kAKey key;

  key.case_str = case_str;
  key.pass = pass;
  key.arg = arg;

  return static_cast<kArgument *>(KLI_rhash_lookup(ka->items, &key));
}

kArgs *CREATOR_args_create(int argc, const char **argv)
{
  kArgs *ka = static_cast<kArgs *>(MEM_callocN(sizeof(kArgs), "kArgs"));
  ka->passes = static_cast<int *>(MEM_callocN(sizeof(int) * argc, "kArgs passes"));
  ka->items = KLI_rhash_new(keyhash, keycmp, "kArgs passes rh");
  KLI_listbase_clear(&ka->docs);
  ka->argc = argc;
  ka->argv = argv;

  /* Must be initialized by #CREATOR_args_pass_set. */
  ka->current_pass = 0;

  return ka;
}

void CREATOR_args_pass_set(struct kArgs *ka, int current_pass)
{
  KLI_assert((current_pass != 0) && (current_pass >= -1));
  ka->current_pass = current_pass;
}

static kArgDoc *internalDocs(struct kArgs *ka,
                             const char *short_arg,
                             const char *long_arg,
                             const char *doc)
{
  kArgDoc *d;

  d = static_cast<kArgDoc *>(MEM_callocN(sizeof(kArgDoc), "kArgDoc"));

  if (doc == NULL) {
    doc = NO_DOCS;
  }

  d->short_arg = short_arg;
  d->long_arg = long_arg;
  d->documentation = doc;

  KLI_addtail(&ka->docs, d);

  return d;
}

static void internalAdd(struct kArgs *ka,
                        const char *arg,
                        int case_str,
                        KA_ArgCallback cb,
                        void *data,
                        kArgDoc *d)
{
  const int pass = ka->current_pass;
  kArgument *a;
  kAKey *key;

  a = lookUp(ka, arg, pass, case_str);

  if (a) {
    printf("WARNING: conflicting argument\n");
    printf("\ttrying to add '%s' on pass %i, %scase sensitive\n",
           arg,
           pass,
           case_str == 1 ? "not " : "");
    printf("\tconflict with '%s' on pass %i, %scase sensitive\n\n",
           a->key->arg,
           (int)a->key->pass,
           a->key->case_str == 1 ? "not " : "");
  }

  a = static_cast<kArgument *>(MEM_callocN(sizeof(kArgument), "kArgument"));
  key = static_cast<kAKey *>(MEM_callocN(sizeof(kAKey), "kAKey"));

  key->arg = arg;
  key->pass = pass;
  key->case_str = case_str;

  a->key = key;
  a->func = cb;
  a->data = data;
  a->doc = d;

  KLI_rhash_insert(ka->items, key, a);
}

/* -------------------------------------------------------------------- */
/** \name Handle Argument Callbacks
 *
 * \note Doc strings here are used in differently:
 *
 * - The `--help` message.
 * - The man page (for Unix systems),
 *   see: `doc/manpage/kraken.1.py`
 * - Parsed and extracted for the manual,
 *   which converts our ad-hoc formatting to reStructuredText.
 *   see: https://docs.wabi.foundation/manual/en/dev/advanced/command_line.html
 *
 * \{ */

static void print_version_full(void)
{
  printf("Kraken %s\n", KKE_kraken_version_string());
#ifdef BUILD_DATE
  printf("\tbuild date: %s\n", build_date);
  printf("\tbuild time: %s\n", build_time);
  printf("\tbuild commit date: %s\n", build_commit_date);
  printf("\tbuild commit time: %s\n", build_commit_time);
  printf("\tbuild hash: %s\n", build_hash);
  printf("\tbuild platform: %s\n", build_platform);
  printf("\tbuild type: %s\n", build_type);
  printf("\tbuild c flags: %s\n", build_cflags);
  printf("\tbuild c++ flags: %s\n", build_cxxflags);
  printf("\tbuild link flags: %s\n", build_linkflags);
  printf("\tbuild system: %s\n", build_system);
#endif
}

static void print_version_short(void)
{
#ifdef BUILD_DATE
  /* NOTE: We include built time since sometimes we need to tell broken from
   * working built of the same hash. */
  printf("Kraken %s (hash %s built %s %s)\n",
         KKE_kraken_version_string(),
         build_hash,
         build_date,
         build_time);
#else
  printf("Kraken %s\n", KKE_kraken_version_string());
#endif
}

static const char arg_handle_print_version_doc[] =
  "\n\t"
  "Print Kraken version and exit.";
static int arg_handle_print_version(int UNUSED(argc),
                                    const char **UNUSED(argv),
                                    void *UNUSED(data))
{
  print_version_full();
  exit(0);
  return 0;
}

static const char arg_handle_print_help_doc[] =
  "\n\t"
  "Print this help text and exit.";
static const char arg_handle_print_help_doc_win32[] =
  "\n\t"
  "Print this help text and exit (Windows only).";
static int arg_handle_print_help(int UNUSED(argc), const char **UNUSED(argv), void *data)
{
  kArgs *ka = (kArgs *)data;

  printf("Kraken %s\n", KKE_kraken_version_string());
  printf("Usage: kraken [args ...] [file] [args ...]\n\n");

  printf("Render Options:\n");
  CREATOR_args_print_arg_doc(ka, "--background");
  CREATOR_args_print_arg_doc(ka, "--render-anim");
  CREATOR_args_print_arg_doc(ka, "--scene");
  CREATOR_args_print_arg_doc(ka, "--render-frame");
  CREATOR_args_print_arg_doc(ka, "--frame-start");
  CREATOR_args_print_arg_doc(ka, "--frame-end");
  CREATOR_args_print_arg_doc(ka, "--frame-jump");
  CREATOR_args_print_arg_doc(ka, "--render-output");
  CREATOR_args_print_arg_doc(ka, "--engine");
  CREATOR_args_print_arg_doc(ka, "--threads");

  printf("\n");
  printf("Format Options:\n");
  CREATOR_args_print_arg_doc(ka, "--render-format");
  CREATOR_args_print_arg_doc(ka, "--use-extension");

  printf("\n");
  printf("Animation Playback Options:\n");
  CREATOR_args_print_arg_doc(ka, "-a");

  printf("\n");
  printf("Window Options:\n");
  CREATOR_args_print_arg_doc(ka, "--window-border");
  CREATOR_args_print_arg_doc(ka, "--window-fullscreen");
  CREATOR_args_print_arg_doc(ka, "--window-geometry");
  CREATOR_args_print_arg_doc(ka, "--window-maximized");
  CREATOR_args_print_arg_doc(ka, "--start-console");
  CREATOR_args_print_arg_doc(ka, "--no-native-pixels");
  CREATOR_args_print_arg_doc(ka, "--no-window-focus");

  printf("\n");
  printf("Python Options:\n");
  CREATOR_args_print_arg_doc(ka, "--enable-autoexec");
  CREATOR_args_print_arg_doc(ka, "--disable-autoexec");

  printf("\n");

  CREATOR_args_print_arg_doc(ka, "--python");
  CREATOR_args_print_arg_doc(ka, "--python-text");
  CREATOR_args_print_arg_doc(ka, "--python-expr");
  CREATOR_args_print_arg_doc(ka, "--python-console");
  CREATOR_args_print_arg_doc(ka, "--python-exit-code");
  CREATOR_args_print_arg_doc(ka, "--python-use-system-env");
  CREATOR_args_print_arg_doc(ka, "--addons");

  printf("\n");
  printf("Logging Options:\n");
  CREATOR_args_print_arg_doc(ka, "--log");
  CREATOR_args_print_arg_doc(ka, "--log-level");
  CREATOR_args_print_arg_doc(ka, "--log-show-basename");
  CREATOR_args_print_arg_doc(ka, "--log-show-backtrace");
  CREATOR_args_print_arg_doc(ka, "--log-show-timestamp");
  CREATOR_args_print_arg_doc(ka, "--log-file");

  printf("\n");
  printf("Debug Options:\n");
  CREATOR_args_print_arg_doc(ka, "--debug");
  CREATOR_args_print_arg_doc(ka, "--debug-value");

  printf("\n");
  CREATOR_args_print_arg_doc(ka, "--debug-events");
#ifdef WITH_FFMPEG
  CREATOR_args_print_arg_doc(ka, "--debug-ffmpeg");
#endif
  CREATOR_args_print_arg_doc(ka, "--debug-handlers");
#ifdef WITH_LIBMV
  CREATOR_args_print_arg_doc(ka, "--debug-libmv");
#endif
#ifdef WITH_CYCLES_LOGGING
  CREATOR_args_print_arg_doc(ka, "--debug-cycles");
#endif
  CREATOR_args_print_arg_doc(ka, "--debug-memory");
  CREATOR_args_print_arg_doc(ka, "--debug-jobs");
  CREATOR_args_print_arg_doc(ka, "--debug-python");
  CREATOR_args_print_arg_doc(ka, "--debug-depsgraph");
  CREATOR_args_print_arg_doc(ka, "--debug-depsgraph-eval");
  CREATOR_args_print_arg_doc(ka, "--debug-depsgraph-build");
  CREATOR_args_print_arg_doc(ka, "--debug-depsgraph-tag");
  CREATOR_args_print_arg_doc(ka, "--debug-depsgraph-no-threads");
  CREATOR_args_print_arg_doc(ka, "--debug-depsgraph-time");
  CREATOR_args_print_arg_doc(ka, "--debug-depsgraph-pretty");
  CREATOR_args_print_arg_doc(ka, "--debug-depsgraph-uuid");
  CREATOR_args_print_arg_doc(ka, "--debug-ghost");
  CREATOR_args_print_arg_doc(ka, "--debug-wintab");
  CREATOR_args_print_arg_doc(ka, "--debug-gpu");
  CREATOR_args_print_arg_doc(ka, "--debug-gpu-force-workarounds");
  CREATOR_args_print_arg_doc(ka, "--debug-gpu-disable-ssbo");
  CREATOR_args_print_arg_doc(ka, "--debug-wm");
#ifdef WITH_XR_OPENXR
  CREATOR_args_print_arg_doc(ka, "--debug-xr");
  CREATOR_args_print_arg_doc(ka, "--debug-xr-time");
#endif
  CREATOR_args_print_arg_doc(ka, "--debug-all");
  CREATOR_args_print_arg_doc(ka, "--debug-io");

  printf("\n");
  CREATOR_args_print_arg_doc(ka, "--debug-fpe");
  CREATOR_args_print_arg_doc(ka, "--debug-exit-on-error");
  CREATOR_args_print_arg_doc(ka, "--disable-crash-handler");
  CREATOR_args_print_arg_doc(ka, "--disable-abort-handler");

  CREATOR_args_print_arg_doc(ka, "--verbose");

  printf("\n");
  printf("Misc Options:\n");
  CREATOR_args_print_arg_doc(ka, "--open-last");
  CREATOR_args_print_arg_doc(ka, "--app-template");
  CREATOR_args_print_arg_doc(ka, "--factory-startup");
  CREATOR_args_print_arg_doc(ka, "--enable-event-simulate");
  printf("\n");
  CREATOR_args_print_arg_doc(ka, "--env-system-datafiles");
  CREATOR_args_print_arg_doc(ka, "--env-system-scripts");
  CREATOR_args_print_arg_doc(ka, "--env-system-python");
  printf("\n");
  CREATOR_args_print_arg_doc(ka, "-noaudio");
  CREATOR_args_print_arg_doc(ka, "-setaudio");

  printf("\n");

  CREATOR_args_print_arg_doc(ka, "--help");
  CREATOR_args_print_arg_doc(ka, "/?");

  /* WIN32 only (ignored for non-win32) */
  CREATOR_args_print_arg_doc(ka, "-R");
  CREATOR_args_print_arg_doc(ka, "-r");

  CREATOR_args_print_arg_doc(ka, "--version");

  CREATOR_args_print_arg_doc(ka, "--");

  // printf("\n");
  // printf("Experimental Features:\n");

  /* Other options _must_ be last (anything not handled will show here).
   *
   * Note that it's good practice for this to remain empty,
   * nevertheless print if any exist. */
  if (CREATOR_args_has_other_doc(ka)) {
    printf("\n");
    printf("Other Options:\n");
    CREATOR_args_print_other_doc(ka);
  }

  printf("\n");
  printf("Argument Parsing:\n");
  printf("\tArguments must be separated by white space, eg:\n");
  printf("\t# kraken -ka test.usd\n");
  printf("\t...will exit since '-ka' is an unknown argument.\n");

  printf("Argument Order:\n");
  printf("\tArguments are executed in the order they are given. eg:\n");
  printf("\t# kraken --background test.usd --render-frame 1 --render-output '/tmp'\n");
  printf(
    "\t...will not render to '/tmp' because '--render-frame 1' renders before the output path "
    "is set.\n");
  printf("\t# kraken --background --render-output /tmp test.usd --render-frame 1\n");
  printf(
    "\t...will not render to '/tmp' because loading the usd-file overwrites the render output "
    "that was set.\n");
  printf("\t# kraken --background test.usd --render-output /tmp --render-frame 1\n");
  printf("\t...works as expected.\n\n");

  printf("Environment Variables:\n");
  printf("  $KRAKEN_USER_RESOURCES  Top level directory for user files.\n");
  printf("                           (other 'KRAKEN_USER_*' variables override when set).\n");
  printf("  $KRAKEN_USER_CONFIG     Directory for user configuration files.\n");
  printf("  $KRAKEN_USER_SCRIPTS    Directory for user scripts.\n");
  printf("  $KRAKEN_USER_DATAFILES  Directory for user data files (icons, translations, ..).\n");
  printf("\n");
  printf("  $KRAKEN_SYSTEM_RESOURCES  Top level directory for system files.\n");
  printf("                             (other 'KRAKEN_SYSTEM_*' variables override when set).\n");
  printf("  $KRAKEN_SYSTEM_SCRIPTS    Directory for system wide scripts.\n");
  printf("  $KRAKEN_SYSTEM_DATAFILES  Directory for system wide data files.\n");
  printf("  $KRAKEN_SYSTEM_PYTHON     Directory for system Python libraries.\n");

#ifdef WITH_OCIO
  printf("  $OCIO                     Path to override the OpenColorIO config file.\n");
#endif
#ifdef WIN32
  printf("  $TEMP                     Store temporary files here.\n");
#else
  printf("  $TMP or $TMPDIR           Store temporary files here.\n");
#endif

  exit(0);

  return 0;
}

static const char arg_handle_arguments_end_doc[] =
  "\n\t"
  "End option processing, following arguments passed unchanged. Access via Python's "
  "'sys.argv'.";
static int arg_handle_arguments_end(int UNUSED(argc),
                                    const char **UNUSED(argv),
                                    void *UNUSED(data))
{
  return -1;
}

/* only to give help message */
#ifndef WITH_PYTHON_SECURITY /* default */
#  define PY_ENABLE_AUTO ", (default)"
#  define PY_DISABLE_AUTO ""
#else
#  define PY_ENABLE_AUTO ""
#  define PY_DISABLE_AUTO ", (compiled as non-standard default)"
#endif

static const char arg_handle_python_set_doc_enable[] =
  "\n\t"
  "Enable automatic Python script execution" PY_ENABLE_AUTO ".";
static const char arg_handle_python_set_doc_disable[] =
  "\n\t"
  "Disable automatic Python script execution (pydrivers & startup scripts)" PY_DISABLE_AUTO ".";
#undef PY_ENABLE_AUTO
#undef PY_DISABLE_AUTO

static int arg_handle_python_set(int UNUSED(argc), const char **UNUSED(argv), void *data)
{
  if ((bool)data) {
    G.f |= G_FLAG_SCRIPT_AUTOEXEC;
  } else {
    G.f &= ~G_FLAG_SCRIPT_AUTOEXEC;
  }
  G.f |= G_FLAG_SCRIPT_OVERRIDE_PREF;
  return 0;
}

static const char arg_handle_crash_handler_disable_doc[] =
  "\n\t"
  "Disable the crash handler.";
static int arg_handle_crash_handler_disable(int UNUSED(argc),
                                            const char **UNUSED(argv),
                                            void *UNUSED(data))
{
  app_state.signal.use_crash_handler = false;
  return 0;
}

static const char arg_handle_abort_handler_disable_doc[] =
  "\n\t"
  "Disable the abort handler.";
static int arg_handle_abort_handler_disable(int UNUSED(argc),
                                            const char **UNUSED(argv),
                                            void *UNUSED(data))
{
  app_state.signal.use_abort_handler = false;
  return 0;
}

static void clog_abort_on_error_callback(void *fp)
{
  KLI_system_backtrace(static_cast<FILE *>(fp));
  fflush(static_cast<FILE *>(fp));
  abort();
}

static const char arg_handle_debug_exit_on_error_doc[] =
  "\n\t"
  "Immediately exit when internal errors are detected.";
static int arg_handle_debug_exit_on_error(int UNUSED(argc),
                                          const char **UNUSED(argv),
                                          void *UNUSED(data))
{
  MEM_enable_fail_on_memleak();
  CLG_error_fn_set(clog_abort_on_error_callback);
  return 0;
}

static const char arg_handle_background_mode_set_doc[] =
  "\n\t"
  "Run in background (often used for UI-less rendering).";
static int arg_handle_background_mode_set(int UNUSED(argc),
                                          const char **UNUSED(argv),
                                          void *UNUSED(data))
{
  print_version_short();
  G.background = 1;
  return 0;
}

static const char arg_handle_log_level_set_doc[] =
  "<level>\n"
  "\tSet the logging verbosity level (higher for more details) defaults to 1,\n"
  "\tuse -1 to log all levels.";
static int arg_handle_log_level_set(int argc, const char **argv, void *UNUSED(data))
{
  const char *arg_id = "--log-level";
  if (argc > 1) {
    const char *err_msg = NULL;
    if (!parse_int_clamp(argv[1], NULL, -1, INT_MAX, &G.log.level, &err_msg)) {
      printf("\nError: %s '%s %s'.\n", err_msg, arg_id, argv[1]);
    } else {
      if (G.log.level == -1) {
        G.log.level = INT_MAX;
      }
      CLG_level_set(G.log.level);
    }
    return 1;
  }
  printf("\nError: '%s' no args given.\n", arg_id);
  return 0;
}

static const char arg_handle_log_show_basename_set_doc[] =
  "\n\t"
  "Only show file name in output (not the leading path).";
static int arg_handle_log_show_basename_set(int UNUSED(argc),
                                            const char **UNUSED(argv),
                                            void *UNUSED(data))
{
  CLG_output_use_basename_set(true);
  return 0;
}

static const char arg_handle_log_show_backtrace_set_doc[] =
  "\n\t"
  "Show a back trace for each log message (debug builds only).";
static int arg_handle_log_show_backtrace_set(int UNUSED(argc),
                                             const char **UNUSED(argv),
                                             void *UNUSED(data))
{
  /* Ensure types don't become incompatible. */
  void (*fn)(FILE * fp) = KLI_system_backtrace;
  CLG_backtrace_fn_set((void (*)(void *))fn);
  return 0;
}

static const char arg_handle_log_show_timestamp_set_doc[] =
  "\n\t"
  "Show a timestamp for each log message in seconds since start.";
static int arg_handle_log_show_timestamp_set(int UNUSED(argc),
                                             const char **UNUSED(argv),
                                             void *UNUSED(data))
{
  CLG_output_use_timestamp_set(true);
  return 0;
}

static const char arg_handle_log_file_set_doc[] =
  "<filepath>\n"
  "\tSet a file to output the log to.";
static int arg_handle_log_file_set(int argc, const char **argv, void *UNUSED(data))
{
  const char *arg_id = "--log-file";
  if (argc > 1) {
    errno = 0;
    FILE *fp = KLI_fopen(argv[1], "w");
    if (fp == NULL) {
      const char *err_msg = errno ? strerror(errno) : "unknown";
      printf("\nError: %s '%s %s'.\n", err_msg, arg_id, argv[1]);
    } else {
      if (UNLIKELY(G.log.file != NULL)) {
        fclose(static_cast<FILE *>(G.log.file));
      }
      G.log.file = fp;
      CLG_output_set(G.log.file);
    }
    return 1;
  }
  printf("\nError: '%s' no args given.\n", arg_id);
  return 0;
}

static const char arg_handle_log_set_doc[] =
  "<match>\n"
  "\tEnable logging categories, taking a single comma separated argument.\n"
  "\tMultiple categories can be matched using a '.*' suffix,\n"
  "\tso '--log \"wm.*\"' logs every kind of window-manager message.\n"
  "\tSub-string can be matched using a '*' prefix and suffix,\n"
  "\tso '--log \"*undo*\"' logs every kind of undo-related message.\n"
  "\tUse \"^\" prefix to ignore, so '--log \"*,^wm.operator.*\"' logs all except for "
  "'wm.operators.*'\n"
  "\tUse \"*\" to log everything.";
static int arg_handle_log_set(int argc, const char **argv, void *UNUSED(data))
{
  const char *arg_id = "--log";
  if (argc > 1) {
    const char *str_step = argv[1];
    while (*str_step) {
      const char *str_step_end = strchr(str_step, ',');
      int str_step_len = str_step_end ? (str_step_end - str_step) : strlen(str_step);

      if (str_step[0] == '^') {
        CLG_type_filter_exclude(str_step + 1, str_step_len - 1);
      } else {
        CLG_type_filter_include(str_step, str_step_len);
      }

      if (str_step_end) {
        /* Typically only be one, but don't fail on multiple. */
        while (*str_step_end == ',') {
          str_step_end++;
        }
        str_step = str_step_end;
      } else {
        break;
      }
    }
    return 1;
  }
  printf("\nError: '%s' no args given.\n", arg_id);
  return 0;
}

static const char arg_handle_debug_mode_set_doc[] =
  "\n"
  "\tTurn debugging on.\n"
  "\n"
  "\t* Enables memory error detection\n"
  "\t* Disables mouse grab (to interact with a debugger in some cases)\n"
  "\t* Keeps Python's 'sys.stdin' rather than setting it to None";
static int arg_handle_debug_mode_set(int UNUSED(argc), const char **UNUSED(argv), void *data)
{
  G.debug |= G_DEBUG; /* std output printf's */
  printf("Kraken %s\n", KKE_kraken_version_string());
  MEM_set_memory_debug();
#ifndef NDEBUG
  KLI_mempool_set_memory_debug();
#endif

#ifdef WITH_BUILDINFO
  printf("Build: %s %s %s %s\n", build_date, build_time, build_platform, build_type);
#endif

  CREATOR_args_print(static_cast<kArgs *>(data));
  return 0;
}

#ifdef WITH_FFMPEG
static const char arg_handle_debug_mode_generic_set_doc_ffmpeg[] =
  "\n\t"
  "Enable debug messages from FFmpeg library.";
#endif
#ifdef WITH_FREESTYLE
static const char arg_handle_debug_mode_generic_set_doc_freestyle[] =
  "\n\t"
  "Enable debug messages for Freestyle.";
#endif
static const char arg_handle_debug_mode_generic_set_doc_python[] =
  "\n\t"
  "Enable debug messages for Python.";
static const char arg_handle_debug_mode_generic_set_doc_events[] =
  "\n\t"
  "Enable debug messages for the event system.";
static const char arg_handle_debug_mode_generic_set_doc_handlers[] =
  "\n\t"
  "Enable debug messages for event handling.";
static const char arg_handle_debug_mode_generic_set_doc_wm[] =
  "\n\t"
  "Enable debug messages for the window manager, shows all operators in search, shows "
  "keymap errors.";
static const char arg_handle_debug_mode_generic_set_doc_ghost[] =
  "\n\t"
  "Enable debug messages for Ghost (Linux only).";
static const char arg_handle_debug_mode_generic_set_doc_wintab[] =
  "\n\t"
  "Enable debug messages for Wintab.";
#ifdef WITH_XR_OPENXR
static const char arg_handle_debug_mode_generic_set_doc_xr[] =
  "\n\t"
  "Enable debug messages for virtual reality contexts.\n"
  "\tEnables the OpenXR API validation layer, (OpenXR) debug messages and general information "
  "prints.";
static const char arg_handle_debug_mode_generic_set_doc_xr_time[] =
  "\n\t"
  "Enable debug messages for virtual reality frame rendering times.";
#endif
static const char arg_handle_debug_mode_generic_set_doc_jobs[] =
  "\n\t"
  "Enable time profiling for background jobs.";
static const char arg_handle_debug_mode_generic_set_doc_depsgraph[] =
  "\n\t"
  "Enable all debug messages from dependency graph.";
static const char arg_handle_debug_mode_generic_set_doc_depsgraph_build[] =
  "\n\t"
  "Enable debug messages from dependency graph related on graph construction.";
static const char arg_handle_debug_mode_generic_set_doc_depsgraph_tag[] =
  "\n\t"
  "Enable debug messages from dependency graph related on tagging.";
static const char arg_handle_debug_mode_generic_set_doc_depsgraph_time[] =
  "\n\t"
  "Enable debug messages from dependency graph related on timing.";
static const char arg_handle_debug_mode_generic_set_doc_depsgraph_eval[] =
  "\n\t"
  "Enable debug messages from dependency graph related on evaluation.";
static const char arg_handle_debug_mode_generic_set_doc_depsgraph_no_threads[] =
  "\n\t"
  "Switch dependency graph to a single threaded evaluation.";
static const char arg_handle_debug_mode_generic_set_doc_depsgraph_pretty[] =
  "\n\t"
  "Enable colors for dependency graph debug messages.";
static const char arg_handle_debug_mode_generic_set_doc_depsgraph_uuid[] =
  "\n\t"
  "Verify validness of session-wide identifiers assigned to ID datablocks.";
static const char arg_handle_debug_mode_generic_set_doc_gpu_force_workarounds[] =
  "\n\t"
  "Enable workarounds for typical GPU issues and disable all GPU extensions.";
static const char arg_handle_debug_mode_generic_set_doc_gpu_disable_ssbo[] =
  "\n\t"
  "Disable usage of shader storage buffer objects.";

static int arg_handle_debug_mode_generic_set(int UNUSED(argc),
                                             const char **UNUSED(argv),
                                             void *data)
{
  G.debug |= POINTER_AS_INT(data);
  return 0;
}

static const char arg_handle_debug_mode_io_doc[] =
  "\n\t"
  "Enable debug messages for I/O (Collada, ...).";
static int arg_handle_debug_mode_io(int UNUSED(argc),
                                    const char **UNUSED(argv),
                                    void *UNUSED(data))
{
  G.debug |= G_DEBUG_IO;
  return 0;
}

static const char arg_handle_debug_mode_all_doc[] =
  "\n\t"
  "Enable all debug messages.";
static int arg_handle_debug_mode_all(int UNUSED(argc),
                                     const char **UNUSED(argv),
                                     void *UNUSED(data))
{
  G.debug |= G_DEBUG_ALL;
#ifdef WITH_LIBMV
  libmv_startDebugLogging();
#endif
#ifdef WITH_CYCLES_LOGGING
  CCL_start_debug_logging();
#endif
  return 0;
}

#ifdef WITH_LIBMV
static const char arg_handle_debug_mode_libmv_doc[] =
  "\n\t"
  "Enable debug messages from libmv library.";
static int arg_handle_debug_mode_libmv(int UNUSED(argc),
                                       const char **UNUSED(argv),
                                       void *UNUSED(data))
{
  libmv_startDebugLogging();

  return 0;
}
#endif

#ifdef WITH_CYCLES_LOGGING
static const char arg_handle_debug_mode_cycles_doc[] =
  "\n\t"
  "Enable debug messages from Cycles.";
static int arg_handle_debug_mode_cycles(int UNUSED(argc),
                                        const char **UNUSED(argv),
                                        void *UNUSED(data))
{
  CCL_start_debug_logging();
  return 0;
}
#endif

static const char arg_handle_debug_mode_memory_set_doc[] =
  "\n\t"
  "Enable fully guarded memory allocation and debugging.";
static int arg_handle_debug_mode_memory_set(int UNUSED(argc),
                                            const char **UNUSED(argv),
                                            void *UNUSED(data))
{
  MEM_set_memory_debug();
  return 0;
}

static const char arg_handle_debug_value_set_doc[] =
  "<value>\n"
  "\tSet debug value of <value> on startup.";
static int arg_handle_debug_value_set(int argc, const char **argv, void *UNUSED(data))
{
  const char *arg_id = "--debug-value";
  if (argc > 1) {
    const char *err_msg = NULL;
    int value;
    if (!parse_int(argv[1], NULL, &value, &err_msg)) {
      printf("\nError: %s '%s %s'.\n", err_msg, arg_id, argv[1]);
      return 1;
    }

    G.debug_value = value;

    return 1;
  }
  printf("\nError: you must specify debug value to set.\n");
  return 0;
}

static const char arg_handle_debug_gpu_set_doc[] =
  "\n"
  "\tEnable GPU debug context and information for OpenGL 4.3+.";
static int arg_handle_debug_gpu_set(int UNUSED(argc),
                                    const char **UNUSED(argv),
                                    void *UNUSED(data))
{
  /* Also enable logging because that how gl errors are reported. */
  const char *gpu_filter = "gpu.*";
  CLG_type_filter_include(gpu_filter, strlen(gpu_filter));
  G.debug |= G_DEBUG_GPU;
  return 0;
}

/* Handling `Ctrl-C` event in the console. */
static void sig_handle_kraken_esc(int sig)
{
  G.is_break = true; /* forces render loop to read queue, not sure if its needed */

  if (sig == 2) {
    static int count = 0;
    if (count) {
      printf("\Kraken killed\n");
      exit(2);
    }
    printf("\nSent an internal break event. Press ^C again to kill Kraken\n");
    count++;
  }
}

void CREATOR_main_signal_setup_background(void)
{
  /* for all platforms, even windows has it! */
  KLI_assert(G.background);

  /* Support pressing `Ctrl-C` to close Kraken in background-mode.
   * Useful to be able to cancel a render operation. */
  signal(SIGINT, sig_handle_kraken_esc);
}

void CREATOR_main_signal_setup_fpe(void)
{
#if defined(__linux__) || defined(_WIN32) || defined(OSX_SSE_FPE)
  /* zealous but makes float issues a heck of a lot easier to find!
   * set breakpoints on sig_handle_fpe */
  signal(SIGFPE, sig_handle_fpe);

#  if defined(__linux__) && defined(__GNUC__) && defined(HAVE_FEENABLEEXCEPT)
  feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
#  endif /* defined(__linux__) && defined(__GNUC__) */
#  if defined(OSX_SSE_FPE)
  /* OSX uses SSE for floating point by default, so here
   * use SSE instructions to throw floating point exceptions */
  _MM_SET_EXCEPTION_MASK(_MM_MASK_MASK &
                         ~(_MM_MASK_OVERFLOW | _MM_MASK_INVALID | _MM_MASK_DIV_ZERO));
#  endif /* OSX_SSE_FPE */
#  if defined(_WIN32) && defined(_MSC_VER)
  /* enables all fp exceptions */
  _controlfp_s(NULL, 0, _MCW_EM);
  /* hide the ones we don't care about */
  _controlfp_s(NULL, _EM_DENORMAL | _EM_UNDERFLOW | _EM_INEXACT, _MCW_EM);
#  endif /* _WIN32 && _MSC_VER */
#endif
}

static const char arg_handle_debug_fpe_set_doc[] =
  "\n\t"
  "Enable floating-point exceptions.";
static int arg_handle_debug_fpe_set(int UNUSED(argc),
                                    const char **UNUSED(argv),
                                    void *UNUSED(data))
{
  CREATOR_main_signal_setup_fpe();
  return 0;
}

static const char arg_handle_app_template_doc[] =
  "<template>\n"
  "\tSet the application template (matching the directory name), use 'default' for none.";
static int arg_handle_app_template(int argc, const char **argv, void *UNUSED(data))
{
  if (argc > 1) {
    const char *app_template = STREQ(argv[1], "default") ? "" : argv[1];
    WM_init_state_app_template_set(app_template);
    return 1;
  }
  printf("\nError: App template must follow '--app-template'.\n");
  return 0;
}

static const char arg_handle_factory_startup_set_doc[] =
  "\n\t"
  "Skip reading the " STRINGIFY(KRAKEN_STARTUP_FILE) " in the users home directory.";
static int arg_handle_factory_startup_set(int UNUSED(argc),
                                          const char **UNUSED(argv),
                                          void *UNUSED(data))
{
  G.factory_startup = 1;
  G.f |= G_FLAG_USERPREF_NO_SAVE_ON_EXIT;
  return 0;
}

static const char arg_handle_enable_event_simulate_doc[] =
  "\n\t"
  "Enable event simulation testing feature 'kpy.types.Window.event_simulate'.";
static int arg_handle_enable_event_simulate(int UNUSED(argc),
                                            const char **UNUSED(argv),
                                            void *UNUSED(data))
{
  G.f |= G_FLAG_EVENT_SIMULATE;
  return 0;
}

static const char arg_handle_env_system_set_doc_datafiles[] =
  "\n\t"
  "Set the " STRINGIFY_ARG(KRAKEN_SYSTEM_DATAFILES) " environment variable.";
static const char arg_handle_env_system_set_doc_scripts[] =
  "\n\t"
  "Set the " STRINGIFY_ARG(KRAKEN_SYSTEM_SCRIPTS) " environment variable.";
static const char arg_handle_env_system_set_doc_python[] =
  "\n\t"
  "Set the " STRINGIFY_ARG(KRAKEN_SYSTEM_PYTHON) " environment variable.";

static int arg_handle_env_system_set(int argc, const char **argv, void *UNUSED(data))
{
  /* `--env-system-scripts` -> `KRAKEN_SYSTEM_SCRIPTS` */

  char env[64] = "KRAKEN";
  char *ch_dst = env + 7;           /* skip KRAKEN */
  const char *ch_src = argv[0] + 5; /* skip --env */

  if (argc < 2) {
    printf("%s requires one argument\n", argv[0]);
    exit(1);
  }

  for (; *ch_src; ch_src++, ch_dst++) {
    *ch_dst = (*ch_src == '-') ? '_' : (*ch_src) - 32; /* Inline #toupper() */
  }

  *ch_dst = '\0';
  KLI_setenv(env, argv[1]);
  return 1;
}

static const char arg_handle_playback_mode_doc[] =
  "<options> <file(s)>\n"
  "\tInstead of showing Kraken's user interface, this runs Kraken as an animation player,\n"
  "\tto view movies and image sequences rendered in Kraken (ignored if '-b' is set).\n"
  "\n"
  "\tPlayback Arguments:\n"
  "\n"
  "\t-p <sx> <sy>\n"
  "\t\tOpen with lower left corner at <sx>, <sy>.\n"
  "\t-m\n"
  "\t\tRead from disk (Do not buffer).\n"
  "\t-f <fps> <fps-base>\n"
  "\t\tSpecify FPS to start with.\n"
  "\t-j <frame>\n"
  "\t\tSet frame step to <frame>.\n"
  "\t-s <frame>\n"
  "\t\tPlay from <frame>.\n"
  "\t-e <frame>\n"
  "\t\tPlay until <frame>.\n"
  "\t-c <cache_memory>\n"
  "\t\tAmount of memory in megabytes to allow for caching images during playback.\n"
  "\t\tZero disables (clamping to a fixed number of frames instead).";
static int arg_handle_playback_mode(int argc, const char **argv, void *UNUSED(data))
{
  /* Ignore the animation player if `-b` was given first. */
  if (G.background == 0) {
#ifdef WITH_FFMPEG
    /* Setup FFmpeg with current debug flags. */
    IMB_ffmpeg_init();
#endif

    /* This function knows to skip this argument ('-a'). */
    // WM_main_playanim(argc, argv);

    exit(0);
  }

  return -2;
}

static const char arg_handle_window_geometry_doc[] =
  "<sx> <sy> <w> <h>\n"
  "\tOpen with lower left corner at <sx>, <sy> and width and height as <w>, <h>.";
static int arg_handle_window_geometry(int argc, const char **argv, void *UNUSED(data))
{
  const char *arg_id = "-p / --window-geometry";
  int params[4], i;

  if (argc < 5) {
    fprintf(stderr, "Error: requires four arguments '%s'\n", arg_id);
    exit(1);
  }

  for (i = 0; i < 4; i++) {
    const char *err_msg = NULL;
    if (!parse_int(argv[i + 1], NULL, &params[i], &err_msg)) {
      printf("\nError: %s '%s %s'.\n", err_msg, arg_id, argv[1]);
      exit(1);
    }
  }

  // WM_init_state_size_set(UNPACK4(params));

  return 4;
}

static const char arg_handle_native_pixels_set_doc[] =
  "\n\t"
  "Do not use native pixel size, for high resolution displays (MacBook 'Retina').";
static int arg_handle_native_pixels_set(int UNUSED(argc),
                                        const char **UNUSED(argv),
                                        void *UNUSED(data))
{
  WM_init_native_pixels(false);
  return 0;
}

static const char arg_handle_with_borders_doc[] =
  "\n\t"
  "Force opening with borders.";
static int arg_handle_with_borders(int UNUSED(argc), const char **UNUSED(argv), void *UNUSED(data))
{
  WM_init_state_normal_set();
  return 0;
}

static const char arg_handle_without_borders_doc[] =
  "\n\t"
  "Force opening in fullscreen mode.";
static int arg_handle_without_borders(int UNUSED(argc),
                                      const char **UNUSED(argv),
                                      void *UNUSED(data))
{
  WM_init_state_fullscreen_set();
  return 0;
}

static const char arg_handle_window_maximized_doc[] =
  "\n\t"
  "Force opening maximized.";
static int arg_handle_window_maximized(int UNUSED(argc),
                                       const char **UNUSED(argv),
                                       void *UNUSED(data))
{
  WM_init_state_maximized_set();
  return 0;
}

static const char arg_handle_no_window_focus_doc[] =
  "\n\t"
  "Open behind other windows and without taking focus.";
static int arg_handle_no_window_focus(int UNUSED(argc),
                                      const char **UNUSED(argv),
                                      void *UNUSED(data))
{
  WM_init_window_focus_set(false);
  return 0;
}

static const char arg_handle_start_with_console_doc[] =
  "\n\t"
  "Start with the console window open (ignored if '-b' is set), (Windows only).";
static int arg_handle_start_with_console(int UNUSED(argc),
                                         const char **UNUSED(argv),
                                         void *UNUSED(data))
{
  WM_init_state_start_with_console_set(true);
  return 0;
}

static const char arg_handle_register_extension_doc[] =
  "\n\t"
  "Register usd-file extension, then exit (Windows only).";
static const char arg_handle_register_extension_doc_silent[] =
  "\n\t"
  "Silently register usd-file extension, then exit (Windows only).";
static int arg_handle_register_extension(int UNUSED(argc), const char **UNUSED(argv), void *data)
{
#ifdef WIN32
  if (data) {
    G.background = 1;
  }
  KLI_windows_register_usd_extension(G.background);
  TerminateProcess(GetCurrentProcess(), 0);
#else
  (void)data; /* unused */
#endif
  return 0;
}

static const char arg_handle_audio_disable_doc[] =
  "\n\t"
  "Force sound system to None.";
static int arg_handle_audio_disable(int UNUSED(argc),
                                    const char **UNUSED(argv),
                                    void *UNUSED(data))
{
  // KKE_sound_force_device("None");
  return 0;
}

static const char arg_handle_audio_set_doc[] =
  "\n\t"
  "Force sound system to a specific device."
  "\n\t"
  "'None' 'SDL' 'OpenAL' 'CoreAudio' 'JACK' 'PulseAudio' 'WASAPI'.";
static int arg_handle_audio_set(int argc, const char **argv, void *UNUSED(data))
{
  if (argc < 1) {
    fprintf(stderr, "-setaudio require one argument\n");
    exit(1);
  }

  // KKE_sound_force_device(argv[1]);
  return 1;
}

static const char arg_handle_output_set_doc[] =
  "<path>\n"
  "\tSet the render path and file name.\n"
  "\tUse '//' at the start of the path to render relative to the usd-file.\n"
  "\n"
  "\tThe '#' characters are replaced by the frame number, and used to define zero padding.\n"
  "\n"
  "\t* 'animation_##_test.png' becomes 'animation_01_test.png'\n"
  "\t* 'test-######.png' becomes 'test-000001.png'\n"
  "\n"
  "\tWhen the filename does not contain '#', The suffix '####' is added to the filename.\n"
  "\n"
  "\tThe frame number will be added at the end of the filename, eg:\n"
  "\t# kraken -b animation.usd -o //render_ -F PNG -x 1 -a\n"
  "\t'//render_' becomes '//render_####', writing frames as '//render_0001.png'";
static int arg_handle_output_set(int argc, const char **argv, void *data)
{
  kContext *C = static_cast<kContext *>(data);
  if (argc > 1) {
    Scene *scene = CTX_data_scene(C);
    if (scene) {
      KLI_strncpy(scene->r.pic, argv[1], sizeof(scene->r.pic));
      // DEG_id_tag_update(&scene->id, ID_RECALC_COPY_ON_WRITE);
    } else {
      printf("\nError: no usd loaded. cannot use '-o / --render-output'.\n");
    }
    return 1;
  }
  printf("\nError: you must specify a path after '-o  / --render-output'.\n");
  return 0;
}

static const char arg_handle_engine_set_doc[] =
  "<engine>\n"
  "\tSpecify the render engine.\n"
  "\tUse '-E help' to list available engines.";
static int arg_handle_engine_set(int argc, const char **argv, void *data)
{
  kContext *C = static_cast<kContext *>(data);
  if (argc >= 2) {
    if (STREQ(argv[1], "help")) {
      RenderEngineType *type = NULL;
      printf("Kraken Engine Listing:\n");
      /**
       * @TODO: Just stub this out to use GL engine to list the engines,
       * till we have our engine sorted out. */
      for (const auto &engine : UsdImagingGLEngine::GetRendererPlugins()) {
        printf("\t%s\n", UsdImagingGLEngine::GetRendererDisplayName(engine).c_str());
      }
      exit(0);
    } else {
      Scene *scene = CTX_data_scene(C);
      if (scene) {
        /**
         * Users can match either the plugin name token or the render engine display name. */
        for (const auto &engine : UsdImagingGLEngine::GetRendererPlugins()) {
          if (engine == argv[1] ||
              UsdImagingGLEngine::GetRendererDisplayName(engine).contains(argv[1])) {
            KLI_strncpy_utf8(scene->r.engine, argv[1], sizeof(scene->r.engine));
            // DEG_id_tag_update(&scene->id, ID_RECALC_COPY_ON_WRITE);
            break;
          } else {
            printf("\nError: engine not found '%s'\n", argv[1]);
            exit(1);
          }
        }
      } else {
        printf(
          "\nError: no usd stage loaded. "
          "order the arguments so '-E / --engine' is after a usd stage is loaded.\n");
      }
    }

    return 1;
  }
  printf("\nEngine not specified, give 'help' for a list of available engines.\n");
  return 0;
}

static const char arg_handle_image_type_set_doc[] =
  "<format>\n"
  "\tSet the render format.\n"
  "\tValid options are:\n"
  "\t'TGA' 'RAWTGA' 'JPEG' 'IRIS' 'IRIZ' 'AVIRAW' 'AVIJPEG' 'PNG' 'BMP'\n"
  "\n"
  "\tFormats that can be compiled into Kraken, not available on all systems:\n"
  "\t'HDR' 'TIFF' 'OPEN_EXR' 'OPEN_EXR_MULTILAYER' 'MPEG' 'CINEON' 'DPX' 'DDS' 'JP2' 'WEBP'";
static int arg_handle_image_type_set(int argc, const char **argv, void *data)
{
  kContext *C = static_cast<kContext *>(data);
  if (argc > 1) {
    const char *imtype = argv[1];
    Scene *scene = CTX_data_scene(C);
    if (scene) {
      const TfToken imtype_token(imtype);
      const char imtype_new = IMF_imtype_from_token(imtype_token);
      const std::string imtype_dotext = IMF_imtype_dotext_from_token(imtype_token).GetString();
      bool is_hio_supported;

      // Lookup the plug-in type name based on the format,
      // using a filename of 'fake.(*ext)' for check purposes.
      HioImageRegistry &hio_reg = HioImageRegistry::GetInstance();
      if (hio_reg.CurrentlyExists() && !imtype_dotext.empty()) {
        is_hio_supported = hio_reg.IsSupportedImageFile(TfStringToLower("fake." + imtype_dotext));
      }

      if (imtype_new == R_IMF_IMTYPE_INVALID || !is_hio_supported) {
        printf(
          "\nError: Format from '-F / --render-format' not known or not compiled in this "
          "release, or this file type is not registered with the Hio Image Registry.\n");
      } else {
        scene->r.im_format.imtype = imtype_new;
        // DEG_id_tag_update(&scene->id, ID_RECALC_COPY_ON_WRITE);
      }
    } else {
      printf(
        "\nError: no usd loaded. "
        "order the arguments so '-F  / --render-format' is after the usd is loaded.\n");
    }
    return 1;
  }
  printf("\nError: you must specify a format after '-F  / --render-format'.\n");
  return 0;
}

static const char arg_handle_threads_set_doc[] =
  "<threads>\n"
  "\tUse amount of <threads> for rendering and other operations\n"
  "\t[1-" STRINGIFY(KRAKEN_MAX_THREADS) "], 0 for systems processor count.";
static int arg_handle_threads_set(int argc, const char **argv, void *UNUSED(data))
{
  const char *arg_id = "-t / --threads";
  const int min = 0, max = KRAKEN_MAX_THREADS;
  if (argc > 1) {
    const char *err_msg = NULL;
    int threads;
    if (!parse_int_strict_range(argv[1], NULL, min, max, &threads, &err_msg)) {
      printf("\nError: %s '%s %s', expected number in [%d..%d].\n",
             err_msg,
             arg_id,
             argv[1],
             min,
             max);
      return 1;
    }

    KLI_system_num_threads_override_set(threads);
    return 1;
  }
  printf("\nError: you must specify a number of threads in [%d..%d] '%s'.\n", min, max, arg_id);
  return 0;
}

static const char arg_handle_verbosity_set_doc[] =
  "<verbose>\n"
  "\tSet the logging verbosity level for debug messages that support it.";
static int arg_handle_verbosity_set(int argc, const char **argv, void *UNUSED(data))
{
  const char *arg_id = "--verbose";
  if (argc > 1) {
    const char *err_msg = NULL;
    int level;
    if (!parse_int(argv[1], NULL, &level, &err_msg)) {
      printf("\nError: %s '%s %s'.\n", err_msg, arg_id, argv[1]);
    }

#ifdef WITH_LIBMV
    libmv_setLoggingVerbosity(level);
#elif defined(WITH_CYCLES_LOGGING)
    CCL_logging_verbosity_set(level);
#else
    (void)level;
#endif

    return 1;
  }
  printf("\nError: you must specify a verbosity level.\n");
  return 0;
}

static const char arg_handle_extension_set_doc[] =
  "<bool>\n"
  "\tSet option to add the file extension to the end of the file.";
static int arg_handle_extension_set(int argc, const char **argv, void *data)
{
  kContext *C = static_cast<kContext *>(data);
  if (argc > 1) {
    Scene *scene = CTX_data_scene(C);
    if (scene) {
      if (argv[1][0] == '0') {
        scene->r.scemode &= ~R_EXTENSION;
        // DEG_id_tag_update(&scene->id, ID_RECALC_COPY_ON_WRITE);
      } else if (argv[1][0] == '1') {
        scene->r.scemode |= R_EXTENSION;
        // DEG_id_tag_update(&scene->id, ID_RECALC_COPY_ON_WRITE);
      } else {
        printf("\nError: Use '-x 1 / -x 0' To set the extension option or '--use-extension'\n");
      }
    } else {
      printf(
        "\nError: no usd loaded. "
        "order the arguments so '-o ' is after '-x '.\n");
    }
    return 1;
  }
  printf("\nError: you must specify a path after '- '.\n");
  return 0;
}

static const char arg_handle_render_frame_doc[] =
  "<frame>\n"
  "\tRender frame <frame> and save it.\n"
  "\n"
  "\t* +<frame> start frame relative, -<frame> end frame relative.\n"
  "\t* A comma separated list of frames can also be used (no spaces).\n"
  "\t* A range of frames can be expressed using '..' separator between the first and last "
  "frames (inclusive).\n";
static int arg_handle_render_frame(int argc, const char **argv, void *data)
{
  const char *arg_id = "-f / --render-frame";
  kContext *C = static_cast<kContext *>(data);
  Scene *scene = CTX_data_scene(C);
  if (scene) {
    Main *bmain = CTX_data_main(C);

    if (argc > 1) {
      const char *err_msg = NULL;
      Render *re;
      ReportList reports;

      int(*frame_range_arr)[2], frames_range_len;
      if ((frame_range_arr = parse_int_range_relative_clamp_n(argv[1],
                                                              scene->r.sfra,
                                                              scene->r.efra,
                                                              MINAFRAME,
                                                              MAXFRAME,
                                                              &frames_range_len,
                                                              &err_msg)) == NULL) {
        printf("\nError: %s '%s %s'.\n", err_msg, arg_id, argv[1]);
        return 1;
      }

      // re = RE_NewSceneRender(scene);
      // KKE_reports_init(&reports, RPT_STORE);
      // RE_SetReports(re, &reports);
      // for (int i = 0; i < frames_range_len; i++) {
      //   /* We could pass in frame ranges,
      //    * but prefer having exact behavior as passing in multiple frames */
      //   if ((frame_range_arr[i][0] <= frame_range_arr[i][1]) == 0) {
      //     printf("\nWarning: negative range ignored '%s %s'.\n", arg_id, argv[1]);
      //   }

      //   for (int frame = frame_range_arr[i][0]; frame <= frame_range_arr[i][1]; frame++) {
      //     RE_RenderAnim(re, bmain, scene, NULL, NULL, frame, frame, scene->r.frame_step);
      //   }
      // }
      // RE_SetReports(re, NULL);
      // KKE_reports_clear(&reports);
      MEM_freeN(frame_range_arr);
      return 1;
    }
    printf("\nError: frame number must follow '%s'.\n", arg_id);
    return 0;
  }
  printf("\nError: no usd loaded. cannot use '%s'.\n", arg_id);
  return 0;
}

static const char arg_handle_render_animation_doc[] =
  "\n\t"
  "Render frames from start to end (inclusive).";
static int arg_handle_render_animation(int UNUSED(argc), const char **UNUSED(argv), void *data)
{
  kContext *C = static_cast<kContext *>(data);
  Scene *scene = CTX_data_scene(C);
  if (scene) {
    // Main *bmain = CTX_data_main(C);
    // Render *re = RE_NewSceneRender(scene);
    // ReportList reports;
    // KKE_reports_init(&reports, RPT_STORE);
    // RE_SetReports(re, &reports);
    // RE_RenderAnim(re, bmain, scene, NULL, NULL, scene->r.sfra, scene->r.efra,
    // scene->r.frame_step); RE_SetReports(re, NULL); KKE_reports_clear(&reports);
  } else {
    printf("\nError: no usd loaded. cannot use '-a'.\n");
  }
  return 0;
}

static const char arg_handle_scene_set_doc[] =
  "<name>\n"
  "\tSet the active scene <name> for rendering.";
static int arg_handle_scene_set(int argc, const char **argv, void *data)
{
  if (argc > 1) {
    kContext *C = static_cast<kContext *>(data);
    kScene *scene = CTX_data_scene(C);
    if (scene) {
      KLI_strncpy(scene->layer_properties->name, argv[1], sizeof(scene->layer_properties->name));
      CTX_data_scene_set(C, scene);

      /* Set the scene of the first window, see: T55991,
       * otherwise scripts that run later won't get this scene back from the context. */
      wmWindow *win = CTX_wm_window(C);
      if (win == nullptr || !win->GetPrim().IsValid()) {
        win = VALUE_PTR(CTX_wm_manager(C)->windows.begin());
      }
      if (win != nullptr && win->GetPrim().IsValid()) {
        // WM_window_set_active_scene(CTX_data_main(C), C, win, scene);
      }
    }
    return 1;
  }
  printf("\nError: Scene name must follow '-S / --scene'.\n");
  return 0;
}

static const char arg_handle_frame_start_set_doc[] =
  "<frame>\n"
  "\tSet start to frame <frame>, supports +/- for relative frames too.";
static int arg_handle_frame_start_set(int argc, const char **argv, void *data)
{
  const char *arg_id = "-s / --frame-start";
  kContext *C = static_cast<kContext *>(data);
  Scene *scene = CTX_data_scene(C);
  if (scene) {
    if (argc > 1) {
      const char *err_msg = NULL;
      if (!parse_int_relative_clamp(argv[1],
                                    NULL,
                                    scene->r.sfra,
                                    scene->r.sfra - 1,
                                    MINAFRAME,
                                    MAXFRAME,
                                    &scene->r.sfra,
                                    &err_msg)) {
        printf("\nError: %s '%s %s'.\n", err_msg, arg_id, argv[1]);
      } else {
        // DEG_id_tag_update(&scene->id, ID_RECALC_COPY_ON_WRITE);
      }
      return 1;
    }
    printf("\nError: frame number must follow '%s'.\n", arg_id);
    return 0;
  }
  printf("\nError: no usd loaded. cannot use '%s'.\n", arg_id);
  return 0;
}

static const char arg_handle_frame_end_set_doc[] =
  "<frame>\n"
  "\tSet end to frame <frame>, supports +/- for relative frames too.";
static int arg_handle_frame_end_set(int argc, const char **argv, void *data)
{
  const char *arg_id = "-e / --frame-end";
  kContext *C = static_cast<kContext *>(data);
  Scene *scene = CTX_data_scene(C);
  if (scene) {
    if (argc > 1) {
      const char *err_msg = NULL;
      if (!parse_int_relative_clamp(argv[1],
                                    NULL,
                                    scene->r.efra,
                                    scene->r.efra - 1,
                                    MINAFRAME,
                                    MAXFRAME,
                                    &scene->r.efra,
                                    &err_msg)) {
        printf("\nError: %s '%s %s'.\n", err_msg, arg_id, argv[1]);
      } else {
        // DEG_id_tag_update(&scene->id, ID_RECALC_COPY_ON_WRITE);
      }
      return 1;
    }
    printf("\nError: frame number must follow '%s'.\n", arg_id);
    return 0;
  }
  printf("\nError: no usd loaded. cannot use '%s'.\n", arg_id);
  return 0;
}

static const char arg_handle_frame_skip_set_doc[] =
  "<frames>\n"
  "\tSet number of frames to step forward after each rendered frame.";
static int arg_handle_frame_skip_set(int argc, const char **argv, void *data)
{
  const char *arg_id = "-j / --frame-jump";
  kContext *C = static_cast<kContext *>(data);
  Scene *scene = CTX_data_scene(C);
  if (scene) {
    if (argc > 1) {
      const char *err_msg = NULL;
      if (!parse_int_clamp(argv[1], NULL, 1, MAXFRAME, &scene->r.frame_step, &err_msg)) {
        printf("\nError: %s '%s %s'.\n", err_msg, arg_id, argv[1]);
      } else {
        // DEG_id_tag_update(&scene->id, ID_RECALC_COPY_ON_WRITE);
      }
      return 1;
    }
    printf("\nError: number of frames to step must follow '%s'.\n", arg_id);
    return 0;
  }
  printf("\nError: no usd loaded. cannot use '%s'.\n", arg_id);
  return 0;
}

static const char arg_handle_python_file_run_doc[] =
  "<filepath>\n"
  "\tRun the given Python script file.";
static int arg_handle_python_file_run(int argc, const char **argv, void *data)
{
#ifdef WITH_PYTHON
  kContext *C = static_cast<kContext *>(data);

  /* workaround for scripts not getting a kpy.context.scene, causes internal errors elsewhere */
  if (argc > 1) {
    /* Make the path absolute because its needed for relative linked usds to be found */
    char filepath[FILE_MAX];
    KLI_strncpy(filepath, argv[1], sizeof(filepath));
    KLI_path_abs_from_cwd(filepath, sizeof(filepath));

    bool ok = false;
    // KPY_CTX_SETUP(ok = KPY_run_filepath(C, filepath, NULL));
    // if (!ok && app_state.exit_code_on_error.python) {
    printf("\nError: script failed, file: '%s', exiting.\n", argv[1]);
    KPY_python_end();
    exit(app_state.exit_code_on_error.python);
    // }
    return 1;
  }
  printf("\nError: you must specify a filepath after '%s'.\n", argv[0]);
  return 0;

#else
  UNUSED_VARS(argc, argv, data);
  printf("This build of Kraken was built without Python support\n");
  return 0;
#endif /* WITH_PYTHON */
}

static const char arg_handle_python_text_run_doc[] =
  "<name>\n"
  "\tRun the given Python script text block.";
static int arg_handle_python_text_run(int argc, const char **argv, void *data)
{
#ifdef WITH_PYTHON
  kContext *C = static_cast<kContext *>(data);

  /* workaround for scripts not getting a kpy.context.scene, causes internal errors elsewhere */
  if (argc > 1) {
    // Main *bmain = CTX_data_main(C);
    /* Make the path absolute because its needed for relative linked usds to be found */
    // struct Text *text = (struct Text *)KKE_libblock_find_name(bmain, ID_TXT, argv[1]);
    bool ok;

    // if (text) {
    //   KPY_CTX_SETUP(ok = KPY_run_text(C, text, NULL, false));
    // } else {
    printf("\nError: text block not found %s.\n", argv[1]);
    ok = false;
    // }

    if (!ok && app_state.exit_code_on_error.python) {
      printf("\nError: script failed, text: '%s', exiting.\n", argv[1]);
      KPY_python_end();
      exit(app_state.exit_code_on_error.python);
    }

    return 1;
  }
  printf("\nError: you must specify a text block after '%s'.\n", argv[0]);
  return 0;

#else
  UNUSED_VARS(argc, argv, data);
  printf("This build of Kraken was built without Python support\n");
  return 0;
#endif /* WITH_PYTHON */
}

static const char arg_handle_python_expr_run_doc[] =
  "<expression>\n"
  "\tRun the given expression as a Python script.";
static int arg_handle_python_expr_run(int argc, const char **argv, void *data)
{
#ifdef WITH_PYTHON
  kContext *C = static_cast<kContext *>(data);

  /* workaround for scripts not getting a kpy.context.scene, causes internal errors elsewhere */
  if (argc > 1) {
    bool ok;
    KPY_CTX_SETUP(ok = KPY_run_string_exec(C, NULL, argv[1]));
    if (!ok && app_state.exit_code_on_error.python) {
      printf("\nError: script failed, expr: '%s', exiting.\n", argv[1]);
      KPY_python_end();
      exit(app_state.exit_code_on_error.python);
    }
    return 1;
  }
  printf("\nError: you must specify a Python expression after '%s'.\n", argv[0]);
  return 0;

#else
  UNUSED_VARS(argc, argv, data);
  printf("This Kraken was built without Python support\n");
  return 0;
#endif /* WITH_PYTHON */
}

static const char arg_handle_python_console_run_doc[] =
  "\n\t"
  "Run Kraken with an interactive console.";
static int arg_handle_python_console_run(int UNUSED(argc), const char **argv, void *data)
{
#ifdef WITH_PYTHON
  kContext *C = static_cast<kContext *>(data);

  KPY_CTX_SETUP(KPY_run_string_eval(C, (const char *[]){"code", NULL}, "code.interact()"));

  return 0;
#else
  UNUSED_VARS(argv, data);
  printf("This Kraken was built without python support\n");
  return 0;
#endif /* WITH_PYTHON */
}

static const char arg_handle_python_exit_code_set_doc[] =
  "<code>\n"
  "\tSet the exit-code in [0..255] to exit if a Python exception is raised\n"
  "\t(only for scripts executed from the command line), zero disables.";
static int arg_handle_python_exit_code_set(int argc, const char **argv, void *UNUSED(data))
{
  const char *arg_id = "--python-exit-code";
  if (argc > 1) {
    const char *err_msg = NULL;
    const int min = 0, max = 255;
    int exit_code;
    if (!parse_int_strict_range(argv[1], NULL, min, max, &exit_code, &err_msg)) {
      printf("\nError: %s '%s %s', expected number in [%d..%d].\n",
             err_msg,
             arg_id,
             argv[1],
             min,
             max);
      return 1;
    }

    app_state.exit_code_on_error.python = (uchar)exit_code;
    return 1;
  }
  printf("\nError: you must specify an exit code number '%s'.\n", arg_id);
  return 0;
}

static const char arg_handle_python_use_system_env_set_doc[] =
  "\n\t"
  "Allow Python to use system environment variables such as 'PYTHONPATH' and the user "
  "site-packages directory.";
static int arg_handle_python_use_system_env_set(int UNUSED(argc),
                                                const char **UNUSED(argv),
                                                void *UNUSED(data))
{
#ifdef WITH_PYTHON
  KPY_python_use_system_env();
#endif
  return 0;
}

static const char arg_handle_addons_set_doc[] =
  "<addon(s)>\n"
  "\tComma separated list (no spaces) of add-ons to enable in addition to any default add-ons.";
static int arg_handle_addons_set(int argc, const char **argv, void *data)
{
  /* workaround for scripts not getting a kpy.context.scene, causes internal errors elsewhere */
  if (argc > 1) {
#ifdef WITH_PYTHON
    const char script_str[] =
      "from addon_utils import check, enable\n"
      "for m in '%s'.split(','):\n"
      "    if check(m)[1] is False:\n"
      "        enable(m, persistent=True)";
    const int slen = strlen(argv[1]) + (sizeof(script_str) - 2);
    char *str = (char *)malloc(slen);
    kContext *C = static_cast<kContext *>(data);
    KLI_snprintf(str, slen, script_str, argv[1]);

    KLI_assert(strlen(str) + 1 == slen);
    KPY_CTX_SETUP(KPY_run_string_exec(C, NULL, str));
    free(str);
#else
    UNUSED_VARS(argv, data);
#endif /* WITH_PYTHON */
    return 1;
  }
  printf("\nError: you must specify a comma separated list after '--addons'.\n");
  return 0;
}

static int arg_handle_load_file(int UNUSED(argc), const char **argv, void *data)
{
  kContext *C = static_cast<kContext *>(data);
  ReportList reports;
  bool success;

  /* Make the path absolute because its needed for relative linked usds to be found */
  char filepath[FILE_MAX];

  /* NOTE: we could skip these, but so far we always tried to load these files. */
  if (argv[0][0] == '-') {
    fprintf(stderr, "unknown argument, loading as file: %s\n", argv[0]);
  }

  KLI_strncpy(filepath, argv[0], sizeof(filepath));
  KLI_path_slash_native(filepath);
  KLI_path_abs_from_cwd(filepath, sizeof(filepath));
  KLI_path_normalize(NULL, filepath);

  /* load the file */
  KKE_reports_init(&reports, RPT_PRINT);
  WM_file_autoexec_init(filepath);
  success = /*WM_file_read(C, filepath, &reports)*/ false;
  KKE_reports_clear(&reports);

  if (success) {
    if (G.background) {
      /* Ensure we use 'C->data.scene' for background render. */
      CTX_wm_window_set(C, NULL);
    }
  } else {
    /* failed to load file, stop processing arguments if running in background mode */
    if (G.background) {
      /* Set is_break if running in the background mode so
       * kraken will return non-zero exit code which then
       * could be used in automated script to control how
       * good or bad things are.
       */
      G.is_break = true;
      return -1;
    }

    if (KLI_has_kfile_extension(filepath)) {
      /* Just pretend a file was loaded, so the user can press Save and it'll
       * save at the filepath from the CLI. */
      STRNCPY(G_MAIN->stage_id, filepath);
      printf("... opened default scene instead; saving will write to: %s\n", filepath);
    } else {
      printf(
        "Error: argument has no '(.usd|.usda|.usdc|.usdz)' file extension, not using as new file, "
        "exiting! %s\n",
        filepath);
      G.is_break = true;
      WM_exit(C);
    }
  }

  return 0;
}

static const char arg_handle_load_last_file_doc[] =
  "\n\t"
  "Open the most recently opened usd file, instead of the default startup file.";
static int arg_handle_load_last_file(int UNUSED(argc), const char **UNUSED(argv), void *data)
{
  if (KLI_listbase_is_empty(&G.recent_files)) {
    printf("Warning: no recent files known, opening default startup file instead.\n");
    return -1;
  }

  const RecentFile *recent_file = static_cast<RecentFile *>(G.recent_files.first);
  const char *fake_argv[] = {recent_file->filepath};
  return arg_handle_load_file(ARRAY_SIZE(fake_argv), fake_argv, data);
}

void CREATOR_args_add_case(struct kArgs *ka,
                           const char *short_arg,
                           int short_case,
                           const char *long_arg,
                           int long_case,
                           const char *doc,
                           KA_ArgCallback cb,
                           void *data)
{
  kArgDoc *d = internalDocs(ka, short_arg, long_arg, doc);

  if (short_arg) {
    internalAdd(ka, short_arg, short_case, cb, data, d);
  }

  if (long_arg) {
    internalAdd(ka, long_arg, long_case, cb, data, d);
  }
}

void CREATOR_args_add(struct kArgs *ka,
                      const char *short_arg,
                      const char *long_arg,
                      const char *doc,
                      KA_ArgCallback cb,
                      void *data)
{
  CREATOR_args_add_case(ka, short_arg, 0, long_arg, 0, doc, cb, data);
}

void CREATOR_args_destroy(struct kArgs *ka)
{
  KLI_rhash_free(ka->items, MEM_freeN, MEM_freeN);
  MEM_freeN(ka->passes);
  KLI_freelistN(&ka->docs);
  MEM_freeN(ka);
}

void CREATOR_args_print(struct kArgs *ka)
{
  int i;
  for (i = 0; i < ka->argc; i++) {
    printf("argv[%d] = %s\n", i, ka->argv[i]);
  }
}

static void internalDocPrint(kArgDoc *d)
{
  if (d->short_arg && d->long_arg) {
    printf("%s or %s", d->short_arg, d->long_arg);
  } else if (d->short_arg) {
    printf("%s", d->short_arg);
  } else if (d->long_arg) {
    printf("%s", d->long_arg);
  }

  printf(" %s\n\n", d->documentation);
}

void CREATOR_args_print_arg_doc(struct kArgs *ka, const char *arg)
{
  kArgument *a = lookUp(ka, arg, -1, -1);

  if (a) {
    kArgDoc *d = a->doc;

    internalDocPrint(d);

    d->done = true;
  }
}

void CREATOR_args_print_other_doc(struct kArgs *ka)
{
  kArgDoc *d;

  for (d = static_cast<kArgDoc *>(ka->docs.first); d; d = d->next) {
    if (d->done == 0) {
      internalDocPrint(d);
    }
  }
}

bool CREATOR_args_has_other_doc(const struct kArgs *ka)
{
  for (const kArgDoc *d = static_cast<kArgDoc *>(ka->docs.first); d; d = d->next) {
    if (d->done == 0) {
      return true;
    }
  }
  return false;
}

void CREATOR_args_parse(struct kArgs *ka, int pass, KA_ArgCallback default_cb, void *default_data)
{
  KLI_assert((pass != 0) && (pass >= -1));
  int i = 0;

  for (i = 1; i < ka->argc; i++) { /* skip argv[0] */
    if (ka->passes[i] == 0) {
      /* -1 signal what side of the comparison it is */
      kArgument *a = lookUp(ka, ka->argv[i], pass, -1);
      KA_ArgCallback func = NULL;
      void *data = NULL;

      if (a) {
        func = a->func;
        data = a->data;
      } else {
        func = default_cb;
        data = default_data;
      }

      if (func) {
        int retval = func(ka->argc - i, ka->argv + i, data);

        if (retval >= 0) {
          int j;

          /* use extra arguments */
          for (j = 0; j <= retval; j++) {
            ka->passes[i + j] = pass;
          }
          i += retval;
        } else if (retval == -1) {
          if (a) {
            if (a->key->pass != -1) {
              ka->passes[i] = pass;
            }
          }
          break;
        }
      }
    }
  }
}

static void sig_handle_crash_backtrace(FILE *fp)
{
  fputs("\n# backtrace\n", fp);
  KLI_system_backtrace(fp);
}

static void sig_handle_crash(int signum)
{
  /* Might be called after WM/Main exit, so needs to be careful about NULL-checking before
   * de-referencing. */

  wmWindowManager *wm = G_MAIN ? static_cast<wmWindowManager *>(G_MAIN->wm.first) : NULL;

#ifdef USE_WRITE_CRASH_USD
  if (wm && wm->undo_stack) {
    struct MemFile *memfile = KKE_undosys_stack_memfile_get_active(wm->undo_stack);
    if (memfile) {
      char fname[FILE_MAX];

      if (!(G_MAIN && G_MAIN->filepath[0])) {
        KLI_join_dirfile(fname, sizeof(fname), KKE_tempdir_base(), "crash.usda");
      } else {
        STRNCPY(fname, G_MAIN->filepath);
        KLI_path_extension_replace(fname, sizeof(fname), ".crash.usda");
      }

      printf("Writing: %s\n", fname);
      fflush(stdout);

      KLO_memfile_write_file(memfile, fname);
    }
  }
#endif

  FILE *fp;
  char header[512];

  char fname[FILE_MAX];

  if (!(G_MAIN && G_MAIN->stage_id[0])) {
    KLI_join_dirfile(fname, sizeof(fname), KKE_tempdir_base(), "kraken.crash.txt");
  } else {
    KLI_join_dirfile(fname,
                     sizeof(fname),
                     KKE_tempdir_base(),
                     KLI_path_basename(G_MAIN->stage_id));
    KLI_path_extension_replace(fname, sizeof(fname), ".crash.txt");
  }

  printf("Writing: %s\n", fname);
  fflush(stdout);

#ifndef BUILD_DATE
  KLI_snprintf(header,
               sizeof(header),
               "# " USD_VERSION_FMT ", Unknown revision\n",
               USD_VERSION_ARG);
#else
  KLI_snprintf(header,
               sizeof(header),
               "# " USD_VERSION_FMT ", Commit date: %s %s, Hash %s\n",
               USD_VERSION_ARG,
               build_commit_date,
               build_commit_time,
               build_hash);
#endif

  /* open the crash log */
  errno = 0;
  fp = KLI_fopen(fname, "wb");
  if (fp == NULL) {
    fprintf(stderr,
            "Unable to save '%s': %s\n",
            fname,
            errno ? strerror(errno) : "Unknown error opening file");
  } else {
    if (wm) {
      KKE_report_write_file_fp(fp, &wm->reports, header);
    }

    sig_handle_crash_backtrace(fp);

#ifdef WITH_PYTHON
    /* Generate python back-trace if Python is currently active. */
    KPY_python_backtrace(fp);
#endif

    fclose(fp);
  }

  /* Delete content of temp dir! */
  KKE_tempdir_session_purge();

  /* really crash */
  signal(signum, SIG_DFL);
#ifndef WIN32
  kill(getpid(), signum);
#else
  TerminateProcess(GetCurrentProcess(), signum);
#endif
}

static void sig_handle_abort(int UNUSED(signum))
{
  /* Delete content of temp dir! */
  KKE_tempdir_session_purge();
}

void CREATOR_main_signal_setup(void)
{
  if (app_state.signal.use_crash_handler) {
#ifdef WIN32
    SetUnhandledExceptionFilter(windows_exception_handler);
#else
    /* after parsing args */
    signal(SIGSEGV, sig_handle_crash);
#endif
  }

#ifdef WIN32
  /* Prevent any error mode dialogs from hanging the application. */
  SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOALIGNMENTFAULTEXCEPT | SEM_NOGPFAULTERRORBOX |
               SEM_NOOPENFILEERRORBOX);
#endif

  if (app_state.signal.use_abort_handler) {
    signal(SIGABRT, sig_handle_abort);
  }
}

void CREATOR_args_setup(kContext *C, kArgs *ka)
{

#define CB(a) a##_doc, a
#define CB_EX(a, b) a##_doc_##b, a

  /* end argument processing after -- */
  CREATOR_args_pass_set(ka, -1);
  CREATOR_args_add(ka, "--", NULL, CB(arg_handle_arguments_end), NULL);

  /* Pass: Environment Setup
   *
   * It's important these run before any initialization is done, since they set up
   * the environment used to access data-files, which are be used when initializing
   * sub-systems such as color management. */
  CREATOR_args_pass_set(ka, ARG_PASS_ENVIRONMENT);
  CREATOR_args_add(ka,
                   NULL,
                   "--python-use-system-env",
                   CB(arg_handle_python_use_system_env_set),
                   NULL);

  /* Note that we could add used environment variables too. */
  CREATOR_args_add(ka,
                   NULL,
                   "--env-system-datafiles",
                   CB_EX(arg_handle_env_system_set, datafiles),
                   NULL);
  CREATOR_args_add(ka,
                   NULL,
                   "--env-system-scripts",
                   CB_EX(arg_handle_env_system_set, scripts),
                   NULL);
  CREATOR_args_add(ka,
                   NULL,
                   "--env-system-python",
                   CB_EX(arg_handle_env_system_set, python),
                   NULL);

  CREATOR_args_add(ka, "-t", "--threads", CB(arg_handle_threads_set), NULL);

  /* Include in the environment pass so it's possible display errors initializing subsystems,
   * especially `kpy.appdir` since it's useful to show errors finding paths on startup. */
  CREATOR_args_add(ka, NULL, "--log", CB(arg_handle_log_set), ka);
  CREATOR_args_add(ka, NULL, "--log-level", CB(arg_handle_log_level_set), ka);
  CREATOR_args_add(ka, NULL, "--log-show-basename", CB(arg_handle_log_show_basename_set), ka);
  CREATOR_args_add(ka, NULL, "--log-show-backtrace", CB(arg_handle_log_show_backtrace_set), ka);
  CREATOR_args_add(ka, NULL, "--log-show-timestamp", CB(arg_handle_log_show_timestamp_set), ka);
  CREATOR_args_add(ka, NULL, "--log-file", CB(arg_handle_log_file_set), ka);

  /* Pass: Background Mode & Settings
   *
   * Also and commands that exit after usage. */
  CREATOR_args_pass_set(ka, ARG_PASS_SETTINGS);
  CREATOR_args_add(ka, "-h", "--help", CB(arg_handle_print_help), ka);
  /* Windows only */
  CREATOR_args_add(ka, "/?", NULL, CB_EX(arg_handle_print_help, win32), ka);

  CREATOR_args_add(ka, "-v", "--version", CB(arg_handle_print_version), NULL);

  CREATOR_args_add(ka,
                   "-y",
                   "--enable-autoexec",
                   CB_EX(arg_handle_python_set, enable),
                   (void *)true);
  CREATOR_args_add(ka,
                   "-Y",
                   "--disable-autoexec",
                   CB_EX(arg_handle_python_set, disable),
                   (void *)false);

  CREATOR_args_add(ka,
                   NULL,
                   "--disable-crash-handler",
                   CB(arg_handle_crash_handler_disable),
                   NULL);
  CREATOR_args_add(ka,
                   NULL,
                   "--disable-abort-handler",
                   CB(arg_handle_abort_handler_disable),
                   NULL);

  CREATOR_args_add(ka, "-b", "--background", CB(arg_handle_background_mode_set), NULL);

  CREATOR_args_add(ka, "-a", NULL, CB(arg_handle_playback_mode), NULL);

  CREATOR_args_add(ka, "-d", "--debug", CB(arg_handle_debug_mode_set), ka);

  CREATOR_args_add(ka,
                   NULL,
                   "--debug-python",
                   CB_EX(arg_handle_debug_mode_generic_set, python),
                   (void *)G_DEBUG_PYTHON);
  CREATOR_args_add(ka,
                   NULL,
                   "--debug-events",
                   CB_EX(arg_handle_debug_mode_generic_set, events),
                   (void *)G_DEBUG_EVENTS);
  CREATOR_args_add(ka,
                   NULL,
                   "--debug-handlers",
                   CB_EX(arg_handle_debug_mode_generic_set, handlers),
                   (void *)G_DEBUG_HANDLERS);
  CREATOR_args_add(ka,
                   NULL,
                   "--debug-wm",
                   CB_EX(arg_handle_debug_mode_generic_set, wm),
                   (void *)G_DEBUG_WM);

  CREATOR_args_add(ka,
                   NULL,
                   "--debug-ghost",
                   CB_EX(arg_handle_debug_mode_generic_set, ghost),
                   (void *)G_DEBUG_ANCHOR);
  CREATOR_args_add(ka,
                   NULL,
                   "--debug-wintab",
                   CB_EX(arg_handle_debug_mode_generic_set, wintab),
                   (void *)G_DEBUG_WINTAB);
  CREATOR_args_add(ka, NULL, "--debug-all", CB(arg_handle_debug_mode_all), NULL);

  CREATOR_args_add(ka, NULL, "--debug-io", CB(arg_handle_debug_mode_io), NULL);

  CREATOR_args_add(ka, NULL, "--debug-fpe", CB(arg_handle_debug_fpe_set), NULL);

  CREATOR_args_add(ka, NULL, "--debug-memory", CB(arg_handle_debug_mode_memory_set), NULL);

  CREATOR_args_add(ka, NULL, "--debug-value", CB(arg_handle_debug_value_set), NULL);
  CREATOR_args_add(ka,
                   NULL,
                   "--debug-jobs",
                   CB_EX(arg_handle_debug_mode_generic_set, jobs),
                   (void *)G_DEBUG_JOBS);
  CREATOR_args_add(ka, NULL, "--debug-gpu", CB(arg_handle_debug_gpu_set), NULL);

  CREATOR_args_add(ka,
                   NULL,
                   "--debug-depsgraph",
                   CB_EX(arg_handle_debug_mode_generic_set, depsgraph),
                   (void *)G_DEBUG_DEPSGRAPH);
  CREATOR_args_add(ka,
                   NULL,
                   "--debug-depsgraph-build",
                   CB_EX(arg_handle_debug_mode_generic_set, depsgraph_build),
                   (void *)G_DEBUG_DEPSGRAPH_BUILD);
  CREATOR_args_add(ka,
                   NULL,
                   "--debug-depsgraph-eval",
                   CB_EX(arg_handle_debug_mode_generic_set, depsgraph_eval),
                   (void *)G_DEBUG_DEPSGRAPH_EVAL);
  CREATOR_args_add(ka,
                   NULL,
                   "--debug-depsgraph-tag",
                   CB_EX(arg_handle_debug_mode_generic_set, depsgraph_tag),
                   (void *)G_DEBUG_DEPSGRAPH_TAG);
  CREATOR_args_add(ka,
                   NULL,
                   "--debug-depsgraph-time",
                   CB_EX(arg_handle_debug_mode_generic_set, depsgraph_time),
                   (void *)G_DEBUG_DEPSGRAPH_TIME);
  CREATOR_args_add(ka,

                   NULL,
                   "--debug-depsgraph-no-threads",
                   CB_EX(arg_handle_debug_mode_generic_set, depsgraph_no_threads),
                   (void *)G_DEBUG_DEPSGRAPH_NO_THREADS);
  CREATOR_args_add(ka,
                   NULL,
                   "--debug-depsgraph-pretty",
                   CB_EX(arg_handle_debug_mode_generic_set, depsgraph_pretty),
                   (void *)G_DEBUG_DEPSGRAPH_PRETTY);
  CREATOR_args_add(ka,
                   NULL,
                   "--debug-depsgraph-uuid",
                   CB_EX(arg_handle_debug_mode_generic_set, depsgraph_uuid),
                   (void *)G_DEBUG_DEPSGRAPH_UUID);
  CREATOR_args_add(ka,
                   NULL,
                   "--debug-gpu-force-workarounds",
                   CB_EX(arg_handle_debug_mode_generic_set, gpu_force_workarounds),
                   (void *)G_DEBUG_GPU_FORCE_WORKAROUNDS);
  CREATOR_args_add(ka,
                   NULL,
                   "--debug-gpu-disable-ssbo",
                   CB_EX(arg_handle_debug_mode_generic_set, gpu_disable_ssbo),
                   (void *)G_DEBUG_GPU_FORCE_DISABLE_SSBO);
  CREATOR_args_add(ka, NULL, "--debug-exit-on-error", CB(arg_handle_debug_exit_on_error), NULL);

  CREATOR_args_add(ka, NULL, "--verbose", CB(arg_handle_verbosity_set), NULL);

  CREATOR_args_add(ka, NULL, "--app-template", CB(arg_handle_app_template), NULL);
  CREATOR_args_add(ka, NULL, "--factory-startup", CB(arg_handle_factory_startup_set), NULL);
  CREATOR_args_add(ka,
                   NULL,
                   "--enable-event-simulate",
                   CB(arg_handle_enable_event_simulate),
                   NULL);

  /* Pass: Custom Window Stuff. */
  CREATOR_args_pass_set(ka, ARG_PASS_SETTINGS_GUI);
  CREATOR_args_add(ka, "-p", "--window-geometry", CB(arg_handle_window_geometry), NULL);
  CREATOR_args_add(ka, "-w", "--window-border", CB(arg_handle_with_borders), NULL);
  CREATOR_args_add(ka, "-W", "--window-fullscreen", CB(arg_handle_without_borders), NULL);
  CREATOR_args_add(ka, "-M", "--window-maximized", CB(arg_handle_window_maximized), NULL);
  CREATOR_args_add(ka, NULL, "--no-window-focus", CB(arg_handle_no_window_focus), NULL);
  CREATOR_args_add(ka, "-con", "--start-console", CB(arg_handle_start_with_console), NULL);
  CREATOR_args_add(ka, "-R", NULL, CB(arg_handle_register_extension), NULL);
  CREATOR_args_add(ka, "-r", NULL, CB_EX(arg_handle_register_extension, silent), ka);
  CREATOR_args_add(ka, NULL, "--no-native-pixels", CB(arg_handle_native_pixels_set), ka);

  /* Pass: Disabling Things & Forcing Settings. */
  CREATOR_args_pass_set(ka, ARG_PASS_SETTINGS_FORCE);
  CREATOR_args_add_case(ka, "-noaudio", 1, NULL, 0, CB(arg_handle_audio_disable), NULL);
  CREATOR_args_add_case(ka, "-setaudio", 1, NULL, 0, CB(arg_handle_audio_set), NULL);

  /* Pass: Processing Arguments. */
  CREATOR_args_pass_set(ka, ARG_PASS_FINAL);
  CREATOR_args_add(ka, "-f", "--render-frame", CB(arg_handle_render_frame), C);
  CREATOR_args_add(ka, "-a", "--render-anim", CB(arg_handle_render_animation), C);
  CREATOR_args_add(ka, "-S", "--scene", CB(arg_handle_scene_set), C);
  CREATOR_args_add(ka, "-s", "--frame-start", CB(arg_handle_frame_start_set), C);
  CREATOR_args_add(ka, "-e", "--frame-end", CB(arg_handle_frame_end_set), C);
  CREATOR_args_add(ka, "-j", "--frame-jump", CB(arg_handle_frame_skip_set), C);
  CREATOR_args_add(ka, "-P", "--python", CB(arg_handle_python_file_run), C);
  CREATOR_args_add(ka, NULL, "--python-text", CB(arg_handle_python_text_run), C);
  CREATOR_args_add(ka, NULL, "--python-expr", CB(arg_handle_python_expr_run), C);
  CREATOR_args_add(ka, NULL, "--python-console", CB(arg_handle_python_console_run), C);
  CREATOR_args_add(ka, NULL, "--python-exit-code", CB(arg_handle_python_exit_code_set), NULL);
  CREATOR_args_add(ka, NULL, "--addons", CB(arg_handle_addons_set), C);

  CREATOR_args_add(ka, "-o", "--render-output", CB(arg_handle_output_set), C);
  CREATOR_args_add(ka, "-E", "--engine", CB(arg_handle_engine_set), C);

  CREATOR_args_add(ka, "-F", "--render-format", CB(arg_handle_image_type_set), C);
  CREATOR_args_add(ka, "-x", "--use-extension", CB(arg_handle_extension_set), C);

  CREATOR_args_add(ka, NULL, "--open-last", CB(arg_handle_load_last_file), C);
#undef CB
#undef CB_EX
}

/**
 * Needs to be added separately.
 */
void CREATOR_args_setup_post(kContext *C, kArgs *ka)
{
  CREATOR_args_parse(ka, ARG_PASS_FINAL, arg_handle_load_file, C);
}