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
 * KRAKEN Python.
 * It Bites.
 */

#include <Python.h>
#include <frameobject.h>

#include "MEM_guardedalloc.h"

#include "CLG_log.h"

#include "KLI_fileops.h"
#include "KLI_listbase.h"
#include "KLI_path_utils.h"
#include "KLI_string.h"
#include "KLI_string_utf8.h"
#include "KLI_threads.h"
#include "KLI_utildefines.h"

#include "LUXO_types.h"

#include "kpy.h"
#include "kpy_capi_utils.h"
#include "kpy_interface.h"
#include "kpy_intern_string.h"
#include "kpy_path.h"
#include "kpy_stage.h"

#include "KKE_appdir.h"
#include "KKE_appdir.hh"
#include "KKE_context.h"
#include "KKE_global.h"
#include "KKE_main.h"

#include "KPY_extern_python.h"
#include "KPY_extern_run.h"

#include "../generic/py_capi_utils.h"
#include "../generic/python_utildefines.h"
#include "../gpu/gpu_py_api.h"

#include <boost/python.hpp>
#include <boost/python/overloads.hpp>

/* Logging types to use anywhere in the Python modules. */

CLG_LOGREF_DECLARE_GLOBAL(KPY_LOG_CONTEXT, "kpy.context");
CLG_LOGREF_DECLARE_GLOBAL(KPY_LOG_INTERFACE, "kpy.interface");
CLG_LOGREF_DECLARE_GLOBAL(KPY_LOG_PRIM, "kpy.prim");

using namespace boost::python;

KRAKEN_NAMESPACE_USING

/* In case a python script triggers another python
 * call, stop kpy_context_clear from invalidating. */
static int py_call_level = 0;

/* Set by command line arguments before Python starts. */
static bool py_use_system_env = false;

#ifdef TIME_PY_RUN
#  include "KLI_time.h"
static int kpy_timer_count = 0;
static double kpy_timer;         /* time since python starts */
static double kpy_timer_run;     /* time for each python script run */
static double kpy_timer_run_tot; /* accumulate python runs */
#endif

/* use for updating while a python script runs - in case of file load */
void KPY_context_update(kContext *C)
{
  if (!KLI_thread_is_main()) {
    return;
  }

  KPY_context_set(C);
  KPY_modules_update();
}

void kpy_context_set(kContext *C, PyGILState_STATE *gilstate)
{
  py_call_level++;

  if (gilstate) {
    *gilstate = PyGILState_Ensure();
  }

  if (py_call_level == 1) {
    KPY_context_update(C);

#ifdef TIME_PY_RUN
    if (kpy_timer_count == 0) {
      /* record time from the beginning */
      kpy_timer = PIL_check_seconds_timer();
      kpy_timer_run = kpy_timer_run_tot = 0.0;
    }
    kpy_timer_run = PIL_check_seconds_timer();

    kpy_timer_count++;
#endif
  }
}

void kpy_context_clear(kContext *UNUSED(C), const PyGILState_STATE *gilstate)
{
  py_call_level--;

  if (gilstate) {
    PyGILState_Release(*gilstate);
  }

  if (py_call_level < 0) {
    fprintf(stderr, "ERROR: Python context internal state bug. this should not happen!\n");
  } else if (py_call_level == 0) {
    /* XXX: Calling classes currently won't store the context :\,
     * can't set NULL because of this. but this is very flaky still. */
#if 0
    KPY_context_set(NULL);
#endif

#ifdef TIME_PY_RUN
    kpy_timer_run_tot += PIL_check_seconds_timer() - kpy_timer_run;
    kpy_timer_count++;
#endif
  }
}

static void kpy_context_end(kContext *C)
{
  if (UNLIKELY(C == NULL)) {
    return;
  }
  CTX_wm_operator_poll_msg_clear(C);
}

/**
 * Needed so the #Main pointer in `kpy.data` doesn't become out of date */
void KPY_modules_update(void)
{
  /* refreshes the main struct */
  KPY_update_stage_module();
}

static struct _inittab kpy_internal_modules[] = {
  {"_kpy_path", KPyInit__kpy_path},
  {"gpu",       KPyInit_gpu      },
  {NULL,        NULL             },
};

#ifndef WITH_PYTHON_MODULE
/**
 * Convenience function for #KPY_python_start.
 *
 * These should happen so rarely that having comprehensive errors isn't needed.
 * For example if `sys.argv` fails to allocate memory.
 *
 * Show an error just to avoid silent failure in the unlikely event something goes wrong,
 * in this case a developer will need to track down the root cause.
 */
static void pystatus_exit_on_error(PyStatus status)
{
  if (UNLIKELY(PyStatus_Exception(status))) {
    fputs("Internal error initializing Python!\n", stderr);
    /* This calls `exit`. */
    Py_ExitStatusException(status);
  }
}
#endif

void KPY_context_set(kContext *C)
{
  kpy_context_module->ptr.data = (void *)C;
}

kContext *KPY_context_get(void)
{
  return (kContext *)kpy_context_module->ptr.data;
}

/* call KPY_context_set first */
void KPY_python_start(kContext *C, int argc, const char **argv)
{
#ifndef WITH_PYTHON_MODULE
  /* #PyPreConfig (early-configuration). */
  {
    PyPreConfig preconfig;
    PyStatus status;

    if (py_use_system_env) {
      PyPreConfig_InitPythonConfig(&preconfig);
    } else {
      /**
       * Only use the systems environment variables and site when explicitly requested. */
      PyPreConfig_InitIsolatedConfig(&preconfig);
    }

    preconfig.utf8_mode = true;

    status = Py_PreInitialize(&preconfig);
    pystatus_exit_on_error(status);
  }

  /* Must run before python initializes, but after #PyPreConfig. */
  PyImport_ExtendInittab(kpy_internal_modules);

  /* #PyConfig (initialize Python). */
  {
    PyConfig config;
    PyStatus status;
    bool has_python_executable = false;

    PyConfig_InitPythonConfig(&config);

    /* Suppress error messages when calculating the module search path.
     * While harmless, it's noisy. */
    config.pathconfig_warnings = 0;

    /* When using the system's Python, allow the site-directory as well. */
    config.user_site_directory = py_use_system_env;

    /* While `sys.argv` is set, we don't want Python to interpret it. */
    config.parse_argv = 0;
    status = PyConfig_SetBytesArgv(&config, argc, (char *const *)argv);
    pystatus_exit_on_error(status);

    /* Needed for Python's initialization for portable Python installations.
     * We could use #Py_SetPath, but this overrides Python's internal logic
     * for calculating its own module search paths.
     *
     * `sys.executable` is overwritten after initialization to the Python binary. */
    {
      const char *program_path = KKE_appdir_program_path();
      status = PyConfig_SetBytesString(&config, &config.program_name, program_path);
      pystatus_exit_on_error(status);
    }

    /* Setting the program name is important so the 'multiprocessing' module
     * can launch new Python instances. */
    {
      char program_path[FILE_MAX];
      if (KKE_appdir_program_python_search(program_path,
                                           sizeof(program_path),
                                           PY_MAJOR_VERSION,
                                           PY_MINOR_VERSION)) {
        status = PyConfig_SetBytesString(&config, &config.executable, program_path);
        pystatus_exit_on_error(status);
        has_python_executable = true;
      } else {
        /* Set to `sys.executable = None` below (we can't do before Python is initialized). */
        fprintf(stderr,
                "Unable to find the python binary, "
                "the multiprocessing module may not be functional!\n");
      }
    }

    /* Allow to use our own included Python. `py_path_bundle` may be NULL. */
    {
      const char *py_path_bundle = KKE_appdir_folder_id(KRAKEN_SYSTEM_PYTHON, NULL);
      if (py_path_bundle != NULL) {

#  ifdef __APPLE__
        /* Mac-OS allows file/directory names to contain `:` character
         * (represented as `/` in the Finder) but current Python lib (as of release 3.1.1)
         * doesn't handle these correctly. */
        if (strchr(py_path_bundle, ':')) {
          fprintf(stderr,
                  "Warning! Kraken application is located in a path containing ':' or '/' chars\n"
                  "This may make python import function fail\n");
        }
#  endif /* __APPLE__ */

        status = PyConfig_SetBytesString(&config, &config.home, py_path_bundle);
        pystatus_exit_on_error(status);
      } else {
        /* Common enough to use the system Python on Linux/Unix, warn on other systems. */
#  if defined(__APPLE__) || defined(_WIN32)
        fprintf(stderr,
                "Bundled Python not found and is expected on this platform "
                "(the 'install' target may have not been built)\n");
#  endif
      }
    }
    /* Initialize Python (also acquires lock). */
    status = Py_InitializeFromConfig(&config);
    pystatus_exit_on_error(status);

    if (!has_python_executable) {
      PySys_SetObject("executable", Py_None);
    }
  }

#else
  (void)argc;
  (void)argv;

  /* must run before python initializes */
  /* broken in py3.3, load explicitly below */
  // PyImport_ExtendInittab(kpy_internal_modules);
#endif

  kpy_intern_string_init();

#ifdef WITH_PYTHON_MODULE
  {
    /* Manually load all modules */
    struct _inittab *inittab_item;
    PyObject *sys_modules = PyImport_GetModuleDict();

    for (inittab_item = kpy_internal_modules; inittab_item->name; inittab_item++) {
      PyObject *mod = inittab_item->initfunc();
      if (mod) {
        PyDict_SetItemString(sys_modules, inittab_item->name, mod);
      } else {
        PyErr_Print();
        PyErr_Clear();
      }
      // Py_DECREF(mod); /* ideally would decref, but in this case we never want to free */
    }
  }
#endif

  /* Run first, initializes PRIM types. */
  KPY_uni_init();

  /* Defines kpy.* and lets us import it */
  KPy_init_modules(C);

  pyprim_alloc_types();

#ifndef WITH_PYTHON_MODULE
  /* py module runs atexit when kpy is freed */
  KPY_atexit_register(); /* this can init any time */

  /* Free the lock acquired (implicitly) when Python is initialized. */
  PyEval_ReleaseThread(PyGILState_GetThisThreadState());

#endif

#ifdef WITH_PYTHON_MODULE
  /* Disable all add-ons at exit, not essential, it just avoids resource leaks. */
  const char *imports[] = {"atexit", "addon_utils", NULL};
  KPY_run_string_eval(C, imports, "atexit.register(addon_utils.disable_all)");
#endif
}

void KPY_python_use_system_env(void)
{
  KLI_assert(!Py_IsInitialized());
  py_use_system_env = true;
}

void KPY_python_backtrace(FILE *fp)
{
  fputs("\n# Python backtrace\n", fp);

  /* Can happen in rare cases. */
  if (!_PyThreadState_UncheckedGet()) {
    return;
  }
  PyFrameObject *frame;
  if (!(frame = PyEval_GetFrame())) {
    return;
  }
  do {
    PyCodeObject *code = PyFrame_GetCode(frame);
    const int line = PyFrame_GetLineNumber(frame);
    const char *filepath = PyUnicode_AsUTF8(code->co_filename);
    const char *funcname = PyUnicode_AsUTF8(code->co_name);
    fprintf(fp, "  File \"%s\", line %d in %s\n", filepath, line, funcname);
  } while ((frame = PyFrame_GetBack(frame)));
}

void KPY_python_end(void)
{
  PyGILState_STATE gilstate;

  /* finalizing, no need to grab the state, except when we are a module */
  gilstate = PyGILState_Ensure();

  /* Clear Python values in the context so freeing the context after Python exits doesn't crash. */
  kpy_context_end(KPY_context_get());

  /* Decrement user counts of all callback functions. */
  // KPY_uni_props_clear_all();

  /* free other python data. */
  // pyprim_free_types();

  /* clear all python data from structs */

  kpy_intern_string_exit();

#ifndef WITH_PYTHON_MODULE
  KPY_atexit_unregister(); /* without this we get recursive calls to WM_exit */

  Py_Finalize();

  (void)gilstate;
#else
  PyGILState_Release(gilstate);
#endif
}

int KPY_context_member_get(kContext *C, const char *member, kContextDataResult *result)
{
  PyGILState_STATE gilstate;
  const bool use_gil = !PyC_IsInterpreterActive();

  PyObject *pyctx;
  PyObject *item;
  KrakenPRIM *ptr = NULL;
  bool done = false;

  if (use_gil) {
    gilstate = PyGILState_Ensure();
  }

  pyctx = (PyObject *)CTX_py_dict_get(C);
  item = PyDict_GetItemString(pyctx, member);

  if (item == NULL) {
    /* pass */

  } else if (item == Py_None) {
    done = true;

  } else if (KPy_StagePRIM_Check(item)) {
    ptr = &(((KPy_StagePRIM *)item)->ptr);

    // result->ptr = ((KPy_KrakenSTAGE *)item)->ptr;
    CTX_data_pointer_set_ptr(result, ptr);
    CTX_data_type_set(result, CTX_DATA_TYPE_POINTER);
    done = true;

  } else if (PySequence_Check(item)) {
    PyObject *seq_fast = PySequence_Fast(item, "kpy_context_get sequence conversion");
    if (seq_fast == NULL) {
      PyErr_Print();
      PyErr_Clear();

    } else {
      const int len = PySequence_Fast_GET_SIZE(seq_fast);
      PyObject **seq_fast_items = PySequence_Fast_ITEMS(seq_fast);
      int i;

      for (i = 0; i < len; i++) {
        PyObject *list_item = seq_fast_items[i];

        if (KPy_StagePRIM_Check(list_item)) {
#if 0
          CollectionPointerLink *link = MEM_callocN(sizeof(CollectionPointerLink),
                                                    "kpy_context_get");
          link->ptr = ((KPy_KrakenSTAGE *)item)->ptr;
          KLI_addtail(&result->list, link);
#endif
          ptr = &(((KPy_StagePRIM *)list_item)->ptr);
          CTX_data_list_add_ptr(result, ptr);
        } else {
          CLOG_INFO(KPY_LOG_CONTEXT,
                    1,
                    "'%s' list item not a valid type in sequence type '%s'",
                    member,
                    Py_TYPE(item)->tp_name);
        }
      }
      Py_DECREF(seq_fast);
      CTX_data_type_set(result, CTX_DATA_TYPE_COLLECTION);
      done = true;
    }
  }

  if (done == false) {
    if (item) {
      CLOG_INFO(KPY_LOG_CONTEXT, 1, "'%s' not a valid type", member);
    } else {
      CLOG_INFO(KPY_LOG_CONTEXT, 1, "'%s' not found", member);
    }
  } else {
    CLOG_INFO(KPY_LOG_CONTEXT, 2, "'%s' found", member);
  }

  if (use_gil) {
    PyGILState_Release(gilstate);
  }

  return done;
}

void KPY_python_reset(kContext *C)
{
  /* unrelated security stuff */
  G.f &= ~(G_FLAG_SCRIPT_AUTOEXEC_FAIL | G_FLAG_SCRIPT_AUTOEXEC_FAIL_QUIET);
  G.autoexec_fail[0] = '\0';

  // KPY_driver_reset();
  // KPY_app_handlers_reset(false);
  // KPY_modules_load_user(C);
}
