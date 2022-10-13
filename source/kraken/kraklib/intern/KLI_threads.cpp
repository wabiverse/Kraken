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

/**
 * @file
 * KRAKEN Library.
 * Gadget Vault.
 */

#include <cerrno>
#include <cstdlib>
#include <cstring>

#include "MEM_guardedalloc.h"

#include "KLI_gsqueue.h"
#include "KLI_listbase.h"
#include "KLI_system.h"
#include "KLI_task.h"
#include "KLI_threads.h"

#include "KLI_time.h"

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

#include "atomic_ops.h"

/**
 * Basic Thread Control API
 * ========================
 *
 * Many thread cases have an X amount of jobs, and only an Y amount of
 * threads are useful (typically amount of CPU's)
 *
 * This code can be used to start a maximum amount of 'thread slots', which
 * then can be filled in a loop with an idle timer.
 *
 * A sample loop can look like this (pseudo c);
 *
 * @code{.c}
 *
 *   ListBase lb;
 *   int max_threads = 2;
 *   int cont = 1;
 *
 *   KLI_threadpool_init(&lb, do_something_func, max_threads);
 *
 *   while (cont) {
 *     if (KLI_available_threads(&lb) && !(escape loop event)) {
 *       // get new job (data pointer)
 *       // tag job 'processed
 *       KLI_threadpool_insert(&lb, job);
 *     }
 *     else PIL_sleep_ms(50);
 *
 *     // Find if a job is ready, this the do_something_func() should write in job somewhere.
 *     cont = 0;
 *     for (go over all jobs)
 *       if (job is ready) {
 *         if (job was not removed) {
 *           KLI_threadpool_remove(&lb, job);
 *         }
 *       }
 *       else cont = 1;
 *     }
 *     // Conditions to exit loop.
 *     if (if escape loop event) {
 *       if (KLI_available_threadslots(&lb) == max_threads) {
 *         break;
 *       }
 *     }
 *   }
 *
 *   KLI_threadpool_end(&lb);
 *
 * @endcode
 */
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
static uint thread_levels = 0; /* threads can be invoked inside threads */
static int threads_override_num = 0;

/* just a max for security reasons */
#define RE_MAX_THREAD KRAKEN_MAX_THREADS

struct ThreadSlot
{
  struct ThreadSlot *next, *prev;
  void *(*do_thread)(void *);
  void *callerdata;
  pthread_t pthread;
  int avail;
};

void KLI_threadapi_init()
{
  mainid = pthread_self();
}

void KLI_threadapi_exit() {}

void KLI_threadpool_init(ListBase *threadbase, void *(*do_thread)(void *), int tot)
{
  int a;

  if (threadbase != nullptr && tot > 0) {
    KLI_listbase_clear(threadbase);

    if (tot > RE_MAX_THREAD) {
      tot = RE_MAX_THREAD;
    } else if (tot < 1) {
      tot = 1;
    }

    for (a = 0; a < tot; a++) {
      ThreadSlot *tslot = static_cast<ThreadSlot *>(MEM_callocN(sizeof(ThreadSlot), "threadslot"));
      KLI_addtail(threadbase, tslot);
      tslot->do_thread = do_thread;
      tslot->avail = 1;
    }
  }

  atomic_fetch_and_add_u(&thread_levels, 1);
}

int KLI_available_threads(ListBase *threadbase)
{
  int counter = 0;

  LISTBASE_FOREACH(ThreadSlot *, tslot, threadbase)
  {
    if (tslot->avail) {
      counter++;
    }
  }

  return counter;
}

int KLI_threadpool_available_thread_index(ListBase *threadbase)
{
  int counter = 0;

  LISTBASE_FOREACH(ThreadSlot *, tslot, threadbase)
  {
    if (tslot->avail) {
      return counter;
    }
    ++counter;
  }

  return 0;
}

static void *tslot_thread_start(void *tslot_p)
{
  ThreadSlot *tslot = (ThreadSlot *)tslot_p;
  return tslot->do_thread(tslot->callerdata);
}

int KLI_thread_is_main()
{
  return pthread_equal(pthread_self(), mainid);
}

void KLI_threadpool_insert(ListBase *threadbase, void *callerdata)
{
  LISTBASE_FOREACH(ThreadSlot *, tslot, threadbase)
  {
    if (tslot->avail) {
      tslot->avail = 0;
      tslot->callerdata = callerdata;
      pthread_create(&tslot->pthread, nullptr, tslot_thread_start, tslot);
      return;
    }
  }
  printf("ERROR: could not insert thread slot\n");
}

void KLI_threadpool_remove(ListBase *threadbase, void *callerdata)
{
  LISTBASE_FOREACH(ThreadSlot *, tslot, threadbase)
  {
    if (tslot->callerdata == callerdata) {
      pthread_join(tslot->pthread, nullptr);
      tslot->callerdata = nullptr;
      tslot->avail = 1;
    }
  }
}

void KLI_threadpool_remove_index(ListBase *threadbase, int index)
{
  int counter = 0;

  LISTBASE_FOREACH(ThreadSlot *, tslot, threadbase)
  {
    if (counter == index && tslot->avail == 0) {
      pthread_join(tslot->pthread, nullptr);
      tslot->callerdata = nullptr;
      tslot->avail = 1;
      break;
    }
    ++counter;
  }
}

void KLI_threadpool_clear(ListBase *threadbase)
{
  LISTBASE_FOREACH(ThreadSlot *, tslot, threadbase)
  {
    if (tslot->avail == 0) {
      pthread_join(tslot->pthread, nullptr);
      tslot->callerdata = nullptr;
      tslot->avail = 1;
    }
  }
}

void KLI_threadpool_end(ListBase *threadbase)
{

  /* Only needed if there's actually some stuff to end
   * this way we don't end up decrementing thread_levels on an empty `threadbase`. */
  if (threadbase == nullptr || KLI_listbase_is_empty(threadbase)) {
    return;
  }

  LISTBASE_FOREACH(ThreadSlot *, tslot, threadbase)
  {
    if (tslot->avail == 0) {
      pthread_join(tslot->pthread, nullptr);
    }
  }
  KLI_freelistN(threadbase);
}

/* System Information */

int KLI_system_thread_count()
{
  static int t = -1;

  if (threads_override_num != 0) {
    return threads_override_num;
  }
  if (LIKELY(t != -1)) {
    return t;
  }

  {
#ifdef WIN32
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    t = int(info.dwNumberOfProcessors);
#else
#  ifdef __APPLE__
    int mib[2];
    size_t len;

    mib[0] = CTL_HW;
    mib[1] = HW_NCPU;
    len = sizeof(t);
    sysctl(mib, 2, &t, &len, nullptr, 0);
#  else
    t = int(sysconf(_SC_NPROCESSORS_ONLN));
#  endif
#endif
  }

  CLAMP(t, 1, RE_MAX_THREAD);

  return t;
}

void KLI_system_num_threads_override_set(int num)
{
  threads_override_num = num;
}

int KLI_system_num_threads_override_get()
{
  return threads_override_num;
}

/* Global Mutex Locks */

static ThreadMutex *global_mutex_from_type(const int type)
{
  switch (type) {
    case LOCK_IMAGE:
      return &_image_lock;
    case LOCK_DRAW_IMAGE:
      return &_image_draw_lock;
    case LOCK_VIEWER:
      return &_viewer_lock;
    case LOCK_CUSTOM1:
      return &_custom1_lock;
    case LOCK_NODES:
      return &_nodes_lock;
    case LOCK_MOVIECLIP:
      return &_movieclip_lock;
    case LOCK_COLORMANAGE:
      return &_colormanage_lock;
    case LOCK_FFTW:
      return &_fftw_lock;
    case LOCK_VIEW3D:
      return &_view3d_lock;
    default:
      KLI_assert_unreachable();
      return nullptr;
  }
}

void KLI_thread_lock(int type)
{
  pthread_mutex_lock(global_mutex_from_type(type));
}

void KLI_thread_unlock(int type)
{
  pthread_mutex_unlock(global_mutex_from_type(type));
}

/* Mutex Locks */

void KLI_mutex_init(ThreadMutex *mutex)
{
  pthread_mutex_init(mutex, nullptr);
}

void KLI_mutex_lock(ThreadMutex *mutex)
{
  pthread_mutex_lock(mutex);
}

void KLI_mutex_unlock(ThreadMutex *mutex)
{
  pthread_mutex_unlock(mutex);
}

bool KLI_mutex_trylock(ThreadMutex *mutex)
{
  return (pthread_mutex_trylock(mutex) == 0);
}

void KLI_mutex_end(ThreadMutex *mutex)
{
  pthread_mutex_destroy(mutex);
}

ThreadMutex *KLI_mutex_alloc()
{
  ThreadMutex *mutex = static_cast<ThreadMutex *>(MEM_callocN(sizeof(ThreadMutex), "ThreadMutex"));
  KLI_mutex_init(mutex);
  return mutex;
}

void KLI_mutex_free(ThreadMutex *mutex)
{
  KLI_mutex_end(mutex);
  MEM_freeN(mutex);
}

/* Spin Locks */

#ifdef WITH_TBB
static tbb::spin_mutex *tbb_spin_mutex_cast(SpinLock *spin)
{
  static_assert(sizeof(SpinLock) >= sizeof(tbb::spin_mutex),
                "SpinLock must match tbb::spin_mutex");
  static_assert(alignof(SpinLock) % alignof(tbb::spin_mutex) == 0,
                "SpinLock must be aligned same as tbb::spin_mutex");
  return reinterpret_cast<tbb::spin_mutex *>(spin);
}
#endif

void KLI_spin_init(SpinLock *spin)
{
#ifdef WITH_TBB
  tbb::spin_mutex *spin_mutex = tbb_spin_mutex_cast(spin);
  new (spin_mutex) tbb::spin_mutex();
#elif defined(__APPLE__)
  KLI_mutex_init(spin);
#elif defined(_MSC_VER)
  *spin = 0;
#else
  pthread_spin_init(spin, 0);
#endif
}

void KLI_spin_lock(SpinLock *spin)
{
#ifdef WITH_TBB
  tbb::spin_mutex *spin_mutex = tbb_spin_mutex_cast(spin);
  spin_mutex->lock();
#elif defined(__APPLE__)
  KLI_mutex_lock(spin);
#elif defined(_MSC_VER)
  while (InterlockedExchangeAcquire(spin, 1)) {
    while (*spin) {
      /* Spin-lock hint for processors with hyper-threading. */
      YieldProcessor();
    }
  }
#else
  pthread_spin_lock(spin);
#endif
}

void KLI_spin_unlock(SpinLock *spin)
{
#ifdef WITH_TBB
  tbb::spin_mutex *spin_mutex = tbb_spin_mutex_cast(spin);
  spin_mutex->unlock();
#elif defined(__APPLE__)
  KLI_mutex_unlock(spin);
#elif defined(_MSC_VER)
  _ReadWriteBarrier();
  *spin = 0;
#else
  pthread_spin_unlock(spin);
#endif
}

void KLI_spin_end(SpinLock *spin)
{
#ifdef WITH_TBB
  tbb::spin_mutex *spin_mutex = tbb_spin_mutex_cast(spin);
  spin_mutex->~spin_mutex();
#elif defined(__APPLE__)
  KLI_mutex_end(spin);
#elif defined(_MSC_VER)
  /* Nothing to do, spin is a simple integer type. */
#else
  pthread_spin_destroy(spin);
#endif
}

/* Read/Write Mutex Lock */

void KLI_rw_mutex_init(ThreadRWMutex *mutex)
{
  pthread_rwlock_init(mutex, nullptr);
}

void KLI_rw_mutex_lock(ThreadRWMutex *mutex, int mode)
{
  if (mode == THREAD_LOCK_READ) {
    pthread_rwlock_rdlock(mutex);
  } else {
    pthread_rwlock_wrlock(mutex);
  }
}

void KLI_rw_mutex_unlock(ThreadRWMutex *mutex)
{
  pthread_rwlock_unlock(mutex);
}

void KLI_rw_mutex_end(ThreadRWMutex *mutex)
{
  pthread_rwlock_destroy(mutex);
}

ThreadRWMutex *KLI_rw_mutex_alloc()
{
  ThreadRWMutex *mutex = static_cast<ThreadRWMutex *>(
    MEM_callocN(sizeof(ThreadRWMutex), "ThreadRWMutex"));
  KLI_rw_mutex_init(mutex);
  return mutex;
}

void KLI_rw_mutex_free(ThreadRWMutex *mutex)
{
  KLI_rw_mutex_end(mutex);
  MEM_freeN(mutex);
}

/* Ticket Mutex Lock */

struct TicketMutex
{
  pthread_cond_t cond;
  pthread_mutex_t mutex;
  uint queue_head, queue_tail;
};

TicketMutex *KLI_ticket_mutex_alloc()
{
  TicketMutex *ticket = static_cast<TicketMutex *>(
    MEM_callocN(sizeof(TicketMutex), "TicketMutex"));

  pthread_cond_init(&ticket->cond, nullptr);
  pthread_mutex_init(&ticket->mutex, nullptr);

  return ticket;
}

void KLI_ticket_mutex_free(TicketMutex *ticket)
{
  pthread_mutex_destroy(&ticket->mutex);
  pthread_cond_destroy(&ticket->cond);
  MEM_freeN(ticket);
}

void KLI_ticket_mutex_lock(TicketMutex *ticket)
{
  uint queue_me;

  pthread_mutex_lock(&ticket->mutex);
  queue_me = ticket->queue_tail++;

  while (queue_me != ticket->queue_head) {
    pthread_cond_wait(&ticket->cond, &ticket->mutex);
  }

  pthread_mutex_unlock(&ticket->mutex);
}

void KLI_ticket_mutex_unlock(TicketMutex *ticket)
{
  pthread_mutex_lock(&ticket->mutex);
  ticket->queue_head++;
  pthread_cond_broadcast(&ticket->cond);
  pthread_mutex_unlock(&ticket->mutex);
}

/* ************************************************ */

/* Condition */

void KLI_condition_init(ThreadCondition *cond)
{
  pthread_cond_init(cond, nullptr);
}

void KLI_condition_wait(ThreadCondition *cond, ThreadMutex *mutex)
{
  pthread_cond_wait(cond, mutex);
}

void KLI_condition_wait_global_mutex(ThreadCondition *cond, const int type)
{
  pthread_cond_wait(cond, global_mutex_from_type(type));
}

void KLI_condition_notify_one(ThreadCondition *cond)
{
  pthread_cond_signal(cond);
}

void KLI_condition_notify_all(ThreadCondition *cond)
{
  pthread_cond_broadcast(cond);
}

void KLI_condition_end(ThreadCondition *cond)
{
  pthread_cond_destroy(cond);
}

/* ************************************************ */

struct ThreadQueue
{
  GSQueue *queue;
  pthread_mutex_t mutex;
  pthread_cond_t push_cond;
  pthread_cond_t finish_cond;
  volatile int nowait;
  volatile int canceled;
};

ThreadQueue *KLI_thread_queue_init()
{
  ThreadQueue *queue;

  queue = static_cast<ThreadQueue *>(MEM_callocN(sizeof(ThreadQueue), "ThreadQueue"));
  queue->queue = KLI_gsqueue_new(sizeof(void *));

  pthread_mutex_init(&queue->mutex, nullptr);
  pthread_cond_init(&queue->push_cond, nullptr);
  pthread_cond_init(&queue->finish_cond, nullptr);

  return queue;
}

void KLI_thread_queue_free(ThreadQueue *queue)
{
  /* destroy everything, assumes no one is using queue anymore */
  pthread_cond_destroy(&queue->finish_cond);
  pthread_cond_destroy(&queue->push_cond);
  pthread_mutex_destroy(&queue->mutex);

  KLI_gsqueue_free(queue->queue);

  MEM_freeN(queue);
}

void KLI_thread_queue_push(ThreadQueue *queue, void *work)
{
  pthread_mutex_lock(&queue->mutex);

  KLI_gsqueue_push(queue->queue, &work);

  /* signal threads waiting to pop */
  pthread_cond_signal(&queue->push_cond);
  pthread_mutex_unlock(&queue->mutex);
}

void *KLI_thread_queue_pop(ThreadQueue *queue)
{
  void *work = nullptr;

  /* wait until there is work */
  pthread_mutex_lock(&queue->mutex);
  while (KLI_gsqueue_is_empty(queue->queue) && !queue->nowait) {
    pthread_cond_wait(&queue->push_cond, &queue->mutex);
  }

  /* if we have something, pop it */
  if (!KLI_gsqueue_is_empty(queue->queue)) {
    KLI_gsqueue_pop(queue->queue, &work);

    if (KLI_gsqueue_is_empty(queue->queue)) {
      pthread_cond_broadcast(&queue->finish_cond);
    }
  }

  pthread_mutex_unlock(&queue->mutex);

  return work;
}

static void wait_timeout(struct timespec *timeout, int ms)
{
  ldiv_t div_result;
  long sec, usec, x;

#ifdef WIN32
  {
    struct _timeb now;
    _ftime(&now);
    sec = now.time;
    usec = now.millitm * 1000; /* microsecond precision would be better */
  }
#else
  {
    struct timeval now;
    gettimeofday(&now, nullptr);
    sec = now.tv_sec;
    usec = now.tv_usec;
  }
#endif

  /* add current time + millisecond offset */
  div_result = ldiv(ms, 1000);
  timeout->tv_sec = sec + div_result.quot;

  x = usec + (div_result.rem * 1000);

  if (x >= 1000000) {
    timeout->tv_sec++;
    x -= 1000000;
  }

  timeout->tv_nsec = x * 1000;
}

void *KLI_thread_queue_pop_timeout(ThreadQueue *queue, int ms)
{
  double t;
  void *work = nullptr;
  struct timespec timeout;

  t = PIL_check_seconds_timer();
  wait_timeout(&timeout, ms);

  /* wait until there is work */
  pthread_mutex_lock(&queue->mutex);
  while (KLI_gsqueue_is_empty(queue->queue) && !queue->nowait) {
    if (pthread_cond_timedwait(&queue->push_cond, &queue->mutex, &timeout) == ETIMEDOUT) {
      break;
    }
    if (PIL_check_seconds_timer() - t >= ms * 0.001) {
      break;
    }
  }

  /* if we have something, pop it */
  if (!KLI_gsqueue_is_empty(queue->queue)) {
    KLI_gsqueue_pop(queue->queue, &work);

    if (KLI_gsqueue_is_empty(queue->queue)) {
      pthread_cond_broadcast(&queue->finish_cond);
    }
  }

  pthread_mutex_unlock(&queue->mutex);

  return work;
}

int KLI_thread_queue_len(ThreadQueue *queue)
{
  int size;

  pthread_mutex_lock(&queue->mutex);
  size = KLI_gsqueue_len(queue->queue);
  pthread_mutex_unlock(&queue->mutex);

  return size;
}

bool KLI_thread_queue_is_empty(ThreadQueue *queue)
{
  bool is_empty;

  pthread_mutex_lock(&queue->mutex);
  is_empty = KLI_gsqueue_is_empty(queue->queue);
  pthread_mutex_unlock(&queue->mutex);

  return is_empty;
}

void KLI_thread_queue_nowait(ThreadQueue *queue)
{
  pthread_mutex_lock(&queue->mutex);

  queue->nowait = 1;

  /* signal threads waiting to pop */
  pthread_cond_broadcast(&queue->push_cond);
  pthread_mutex_unlock(&queue->mutex);
}

void KLI_thread_queue_wait_finish(ThreadQueue *queue)
{
  /* wait for finish condition */
  pthread_mutex_lock(&queue->mutex);

  while (!KLI_gsqueue_is_empty(queue->queue)) {
    pthread_cond_wait(&queue->finish_cond, &queue->mutex);
  }

  pthread_mutex_unlock(&queue->mutex);
}
