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
 * KRAKEN Library.
 * Gadget Vault.
 *
 * Task scheduler initialization.
 */

#include "MEM_guardedalloc.h"

#include "KLI_task.h"
#include "KLI_threads.h"

#ifdef WITH_TBB
/* Need to include at least one header to get the version define. */
#  include <tbb/blocked_range.h>
#  include <tbb/task_arena.h>
#  if TBB_INTERFACE_VERSION_MAJOR >= 10
#    include <tbb/global_control.h>
#    define WITH_TBB_GLOBAL_CONTROL
#  endif
#endif

/* Task Scheduler */

static int task_scheduler_num_threads = 1;
#ifdef WITH_TBB_GLOBAL_CONTROL
static tbb::global_control *task_scheduler_global_control = nullptr;
#endif

void KLI_task_scheduler_init()
{
#ifdef WITH_TBB_GLOBAL_CONTROL
  const int threads_override_num = KLI_system_num_threads_override_get();

  if (threads_override_num > 0) {
    /* Override number of threads. This settings is used within the lifetime
     * of tbb::global_control, so we allocate it on the heap. */
    task_scheduler_global_control = MEM_new<tbb::global_control>(
      __func__,
      tbb::global_control::max_allowed_parallelism,
      threads_override_num);
    task_scheduler_num_threads = threads_override_num;
  } else {
    /* Let TBB choose the number of threads. For (legacy) code that calls
     * KLI_task_scheduler_num_threads() we provide the system thread count.
     * Ideally such code should be rewritten not to use the number of threads
     * at all. */
    task_scheduler_num_threads = KLI_system_thread_count();
  }
#else
  task_scheduler_num_threads = KLI_system_thread_count();
#endif
}

void KLI_task_scheduler_exit()
{
#ifdef WITH_TBB_GLOBAL_CONTROL
  MEM_delete(task_scheduler_global_control);
#endif
}

int KLI_task_scheduler_num_threads()
{
  return task_scheduler_num_threads;
}

void KLI_task_isolate(void (*func)(void *userdata), void *userdata)
{
#ifdef WITH_TBB
  tbb::this_task_arena::isolate([&] {
    func(userdata);
  });
#else
  func(userdata);
#endif
}