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


#pragma once

/**
 * @file
 * KRAKEN Library.
 * Gadget Vault.
 */

#include <pthread.h>

#include "KLI_utildefines.h"

/* for tables, button in UI, etc */
#define KRAKEN_MAX_THREADS 1024

/** This is run once during startup. */
void KLI_threadapi_init(void);

int KLI_thread_is_main(void);

enum
{
  LOCK_IMAGE = 0,
  LOCK_DRAW_IMAGE,
  LOCK_VIEWER,
  LOCK_CUSTOM1,
  LOCK_NODES,
  LOCK_MOVIECLIP,
  LOCK_COLORMANAGE,
  LOCK_FFTW,
  LOCK_VIEW3D,
};

typedef pthread_mutex_t ThreadMutex;
#define KLI_MUTEX_INITIALIZER PTHREAD_MUTEX_INITIALIZER


#ifdef WITH_TBB
typedef uint32_t SpinLock;
#elif defined(__APPLE__)
typedef ThreadMutex SpinLock;
#elif defined(_MSC_VER)
typedef volatile unsigned int SpinLock;
#else
typedef pthread_spinlock_t SpinLock;
#endif


#if defined(__APPLE__)
#  define ThreadLocal(type) pthread_key_t
#  define KLI_thread_local_create(name) pthread_key_create(&name, NULL)
#  define KLI_thread_local_delete(name) pthread_key_delete(name)
#  define KLI_thread_local_get(name) pthread_getspecific(name)
#  define KLI_thread_local_set(name, value) pthread_setspecific(name, value)
#else /* defined(__APPLE__) */
#  ifdef _MSC_VER
#    define ThreadLocal(type) __declspec(thread) type
#  else
#    define ThreadLocal(type) __thread type
#  endif
#  define KLI_thread_local_create(name)
#  define KLI_thread_local_delete(name)
#  define KLI_thread_local_get(name) name
#  define KLI_thread_local_set(name, value) name = value
#endif /* defined(_MSC_VER) */