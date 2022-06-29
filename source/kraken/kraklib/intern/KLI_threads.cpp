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

#include <pthread.h>

#include <cerrno>
#include <cstdlib>
#include <cstring>

#include "USD_api.h"

#include "KLI_assert.h"
#include "KLI_time.h"
#include "KLI_threads.h"

/* for checking system threads - KLI_system_thread_count */
#ifdef WIN32
#  include <sys/timeb.h>
#  include <windows.h>
#elif defined(__APPLE__)
#  include <sys/sysctl.h>
#  include <sys/types.h>
#else
#  include <sys/time.h>
#  include <unistd.h>
#endif

#ifdef WITH_TBB
#  include <tbb/spin_mutex.h>
#endif

#include "KLI_numaapi.h"

static pthread_mutex_t _image_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t _image_draw_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t _viewer_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t _custom1_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t _nodes_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t _movieclip_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t _colormanage_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t _fftw_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t _view3d_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_t mainid;
static bool is_numa_available = false;
static unsigned int thread_levels = 0; /* threads can be invoked inside threads */
static int num_threads_override = 0;

/* just a max for security reasons */
#define RE_MAX_THREAD KRAKEN_MAX_THREADS

void KLI_threadapi_init(void)
{
#if ARCH_OS_WINDOWS
  mainid = pthread_self();
  if (numaAPI_Initialize() == NUMAAPI_SUCCESS) {
    is_numa_available = true;
  }
#endif /* WIN32 */
}

int KLI_thread_is_main(void)
{
#if !defined(ARCH_OS_WINDOWS)
  return pthread_equal(pthread_self(), mainid);
#else
  /**
   * Checking for main thread is not
   * currently necessary on WinRT,
   * for now, always validate this
   * check. */
  return true;
#endif /* !defined(ARCH_OS_WINDOWS) */
}