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
 * Creator.
 * Creating Chaos.
 */

#include "KLI_utildefines.h"

#ifdef __cplusplus
#  include <wabi/base/arch/defines.h>
#endif /* __cplusplus */

/* --------- c compatible here. -------- */

struct kArgs;
typedef struct kArgs kArgs;

/**
 * Passes for use by #CREATOR_args_setup.
 * Keep in order of execution.
 */
enum
{
  /** Run before sub-system initialization. */
  ARG_PASS_ENVIRONMENT = 1,
  /** General settings parsing, also animation player. */
  ARG_PASS_SETTINGS = 2,
  /** Windowing & graphical settings (ignored in background mode). */
  ARG_PASS_SETTINGS_GUI = 3,
  /** Currently use for audio devices. */
  ARG_PASS_SETTINGS_FORCE = 4,

  /** Actions & fall back to loading usd file. */
  ARG_PASS_FINAL = 5,
};

/* for the callbacks: */
#ifndef WITH_PYTHON_MODULE
#  define USD_VERSION_FMT "Kraken %d.%d.%d"
#  define USD_VERSION_ARG (KRAKEN_VERSION / 100), (KRAKEN_VERSION % 100), KRAKEN_VERSION_PATCH
#endif

#ifdef WITH_BUILDINFO_HEADER
#  define BUILD_DATE
#endif

/* From `buildinfo.cpp`. */
#ifdef BUILD_DATE
extern char build_date[];
extern char build_time[];
extern char build_hash[];
extern unsigned long build_commit_timestamp;

extern char build_commit_date[16];
extern char build_commit_time[16];

extern char build_branch[];
extern char build_platform[];
extern char build_type[];
extern char build_cflags[];
extern char build_cxxflags[];
extern char build_linkflags[];
extern char build_system[];
#endif /* BUILD_DATE */

/** Shared data for argument handlers to store state in. */
struct ApplicationState
{
  struct
  {
    bool use_crash_handler;
    bool use_abort_handler;
  } signal;

  /* we may want to set different exit codes for other kinds of errors */
  struct
  {
    unsigned char python;
  } exit_code_on_error;
};
extern struct ApplicationState app_state; /* creator.cpp */

/**
 * Returns the number of extra arguments consumed by the function.
 * -  0 is normal value,
 * - -1 stops parsing arguments, other negative indicates skip
 */
typedef int (*KA_ArgCallback)(int argc, const char **argv, void *data);

struct kArgs *CREATOR_args_create(int argc, const char **argv);
void CREATOR_args_destroy(struct kArgs *ka);
void CREATOR_args_print(struct kArgs *ka);
void CREATOR_args_add(struct kArgs *ka,
                      const char *short_arg,
                      const char *long_arg,
                      const char *doc,
                      KA_ArgCallback cb,
                      void *data);
void CREATOR_args_add_case(struct kArgs *ka,
                           const char *short_arg,
                           int short_case,
                           const char *long_arg,
                           int long_case,
                           const char *doc,
                           KA_ArgCallback cb,
                           void *data);
void CREATOR_args_setup(struct kContext *C, struct kArgs *ka);
void CREATOR_args_setup_post(struct kContext *C, struct kArgs *ka);

void CREATOR_main_signal_setup(void);
void CREATOR_main_signal_setup_background(void);
void CREATOR_main_signal_setup_fpe(void);
void CREATOR_args_parse(struct kArgs *ka, int pass, KA_ArgCallback default_cb, void *default_data);
void CREATOR_args_print_arg_doc(struct kArgs *ka, const char *arg);
void CREATOR_args_print_other_doc(struct kArgs *ka);
bool CREATOR_args_has_other_doc(const struct kArgs *ka);
void CREATOR_args_pass_set(struct kArgs *ka, int current_pass);

/* --------- cxx only here. -------- */

#ifdef __cplusplus
int CREATOR_kraken_main(int argc = 0, const char **argv = NULL);
#endif /* __cplusplus */
