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
 * Derived from original work by Copyright 2022, Blender Foundation.
 * From the Blender Library. (source/blender/blenlib).
 *
 * With any additions or modifications specific to Kraken.
 *
 * Modifications Copyright 2022, Wabi Animation Studios, Ltd. Co.
 */

#pragma once

/**
 * @file
 * KRAKEN Library.
 * Gadget Vault.
 */

#include "KLI_sys_types.h"

/* platform specific includes. */
#if defined(__linux__) || defined(__APPLE__)
#  include <sys/time.h>
#  include <unistd.h>
#elif defined(ARCH_OS_WINDOWS)
#  include "stdlib.h"
#  include <windows.h>
#endif /* ARCH_OS_WINDOWS */

/* ---------------------------------------------- */

/** -----
 * @CXX: Time Utils ----- */

void KLI_pretty_time(time_t timer, char *r_time);

/* ---------------------------------------------- */

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------- */

/** -----
 * @PIL: Time API ----- */

double PIL_check_seconds_timer(void);
void PIL_sleep_ms(int ms);

/* ---------------------------------------------- */

/** -----
 * @KRAKEN: Timer API ----- */

/**
 * @return A value of:
 * - <  0: the timer will be removed.
 * - >= 0: the timer will be called again in this number of seconds. */
typedef double (*KLI_timer_func)(uintptr_t uuid, void *user_data);
typedef void (*KLI_timer_data_free)(uintptr_t uuid, void *user_data);

/**
 * @FUNC:(...)  < 0: The timer will be removed.
 * @FUNC:(...) >= 0: The function will be called again in that many seconds. */
void KLI_timer_register(uintptr_t uuid,
                        KLI_timer_func func,
                        void *user_data,
                        KLI_timer_data_free user_data_free,
                        double first_interval,
                        bool persistent);

bool KLI_timer_is_registered(uintptr_t uuid);

/** Returns False when the timer does not exist (anymore). */
bool KLI_timer_unregister(uintptr_t uuid);

/** Execute all registered functions that are due. */
void KLI_timer_execute(void);

void KLI_timer_free(void);

/* This function is to be called next to BKE_CB_EVT_LOAD_PRE, to make sure the module
 * is properly configured for the new file. */
void KLI_timer_on_file_load(void);

/* ---------------------------------------------- */

#ifdef __cplusplus
}
#endif

/* ---------------------------------------------- */