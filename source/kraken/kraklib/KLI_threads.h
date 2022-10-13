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

#include <pthread.h>

#include "KLI_sys_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** For tables, button in UI, etc. */
#define KRAKEN_MAX_THREADS 1024

struct ListBase;

/* Threading API */

/**
 * This is run once at startup.
 */
void KLI_threadapi_init(void);
void KLI_threadapi_exit(void);

/**
 * \param tot: When 0 only initializes malloc mutex in a safe way (see sequence.c)
 * problem otherwise: scene render will kill of the mutex!
 */
void KLI_threadpool_init(struct ListBase *threadbase, void *(*do_thread)(void *), int tot);
/**
 * Amount of available threads.
 */
int KLI_available_threads(struct ListBase *threadbase);
/**
 * Returns thread number, for sample patterns or threadsafe tables.
 */
int KLI_threadpool_available_thread_index(struct ListBase *threadbase);
void KLI_threadpool_insert(struct ListBase *threadbase, void *callerdata);
void KLI_threadpool_remove(struct ListBase *threadbase, void *callerdata);
void KLI_threadpool_remove_index(struct ListBase *threadbase, int index);
void KLI_threadpool_clear(struct ListBase *threadbase);
void KLI_threadpool_end(struct ListBase *threadbase);
int KLI_thread_is_main(void);

/* System Information */

/**
 * \return the number of threads the system can make use of.
 */
int KLI_system_thread_count(void);
void KLI_system_num_threads_override_set(int num);
int KLI_system_num_threads_override_get(void);

/**
 * Global Mutex Locks
 *
 * One custom lock available now. can be extended.
 */
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

void KLI_thread_lock(int type);
void KLI_thread_unlock(int type);

/* Mutex Lock */

typedef pthread_mutex_t ThreadMutex;
#define KLI_MUTEX_INITIALIZER PTHREAD_MUTEX_INITIALIZER

void KLI_mutex_init(ThreadMutex *mutex);
void KLI_mutex_end(ThreadMutex *mutex);

ThreadMutex *KLI_mutex_alloc(void);
void KLI_mutex_free(ThreadMutex *mutex);

void KLI_mutex_lock(ThreadMutex *mutex);
bool KLI_mutex_trylock(ThreadMutex *mutex);
void KLI_mutex_unlock(ThreadMutex *mutex);

/* Spin Lock */

/* By default we use TBB for spin lock on all platforms. When building without
 * TBB fall-back to spin lock implementation which is native to the platform.
 *
 * On macOS we use mutex lock instead of spin since the spin lock has been
 * deprecated in SDK 10.12 and is discouraged from use. */

#ifdef WITH_TBB
typedef uint32_t SpinLock;
#elif defined(__APPLE__)
typedef ThreadMutex SpinLock;
#elif defined(_MSC_VER)
typedef volatile unsigned int SpinLock;
#else
typedef pthread_spinlock_t SpinLock;
#endif

void KLI_spin_init(SpinLock *spin);
void KLI_spin_lock(SpinLock *spin);
void KLI_spin_unlock(SpinLock *spin);
void KLI_spin_end(SpinLock *spin);

/* Read/Write Mutex Lock */

#define THREAD_LOCK_READ 1
#define THREAD_LOCK_WRITE 2

#define KLI_RWLOCK_INITIALIZER PTHREAD_RWLOCK_INITIALIZER

typedef pthread_rwlock_t ThreadRWMutex;

void KLI_rw_mutex_init(ThreadRWMutex *mutex);
void KLI_rw_mutex_end(ThreadRWMutex *mutex);

ThreadRWMutex *KLI_rw_mutex_alloc(void);
void KLI_rw_mutex_free(ThreadRWMutex *mutex);

void KLI_rw_mutex_lock(ThreadRWMutex *mutex, int mode);
void KLI_rw_mutex_unlock(ThreadRWMutex *mutex);

/* Ticket Mutex Lock
 *
 * This is a 'fair' mutex in that it will grant the lock to the first thread
 * that requests it. */

typedef struct TicketMutex TicketMutex;

TicketMutex *KLI_ticket_mutex_alloc(void);
void KLI_ticket_mutex_free(TicketMutex *ticket);
void KLI_ticket_mutex_lock(TicketMutex *ticket);
void KLI_ticket_mutex_unlock(TicketMutex *ticket);

/* Condition */

typedef pthread_cond_t ThreadCondition;

void KLI_condition_init(ThreadCondition *cond);
void KLI_condition_wait(ThreadCondition *cond, ThreadMutex *mutex);
void KLI_condition_wait_global_mutex(ThreadCondition *cond, int type);
void KLI_condition_notify_one(ThreadCondition *cond);
void KLI_condition_notify_all(ThreadCondition *cond);
void KLI_condition_end(ThreadCondition *cond);

/* ThreadWorkQueue
 *
 * Thread-safe work queue to push work/pointers between threads. */

typedef struct ThreadQueue ThreadQueue;

ThreadQueue *KLI_thread_queue_init(void);
void KLI_thread_queue_free(ThreadQueue *queue);

void KLI_thread_queue_push(ThreadQueue *queue, void *work);
void *KLI_thread_queue_pop(ThreadQueue *queue);
void *KLI_thread_queue_pop_timeout(ThreadQueue *queue, int ms);
int KLI_thread_queue_len(ThreadQueue *queue);
bool KLI_thread_queue_is_empty(ThreadQueue *queue);

void KLI_thread_queue_wait_finish(ThreadQueue *queue);
void KLI_thread_queue_nowait(ThreadQueue *queue);

/* Thread local storage */

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
#endif /* defined(__APPLE__) */

#ifdef __cplusplus
}
#endif
