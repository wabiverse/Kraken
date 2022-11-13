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

#include "KLI_time.h"
#include "KLI_listbase.h"

#include "MEM_guardedalloc.h"

#include "USD_api.h"
#include "USD_listBase.h"

#include "KLI_string.h"

#if defined(ARCH_OS_LINUX) || defined(ARCH_OS_DARWIN)
double PIL_check_seconds_timer(void)
{
  struct timeval tv;
  struct timezone tz;

  gettimeofday(&tv, &tz);

  return ((double)tv.tv_sec + tv.tv_usec / 1000000.0);
}
#elif defined(ARCH_OS_WINDOWS)
#  define sleep(x) Sleep(x)

void usleep(__int64 usec)
{
  HANDLE timer;
  LARGE_INTEGER ft;

  ft.QuadPart = -(10 * usec);

  timer = CreateWaitableTimer(NULL, TRUE, NULL);
  SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
  WaitForSingleObject(timer, INFINITE);
  CloseHandle(timer);
}

double PIL_check_seconds_timer(void)
{
  static int hasperfcounter = -1; /* (-1 == unknown) */
  static double perffreq;

  if (hasperfcounter == -1) {
    __int64 ifreq;
    hasperfcounter = QueryPerformanceFrequency((LARGE_INTEGER *)&ifreq);
    perffreq = (double)ifreq;
  }

  if (hasperfcounter) {
    __int64 count;

    QueryPerformanceCounter((LARGE_INTEGER *)&count);

    return count / perffreq;
  } else {
    static double accum = 0.0;
    static int ltick = 0;
    int ntick = GetTickCount();

    if (ntick < ltick) {
      accum += (0xFFFFFFFF - ltick + ntick) / 1000.0;
    } else {
      accum += (ntick - ltick) / 1000.0;
    }

    ltick = ntick;
    return accum;
  }
}
#endif /* WIN32 */

void PIL_sleep_ms(int ms)
{
  if (ms >= 1000) {
    sleep(ms / 1000);
    ms = (ms % 1000);
  }

  usleep(ms * 1000);
}


void KLI_pretty_time(time_t t, char *r_time)
{
  char buffer[USD_MAX_TIME];

  std::tm *ptm = std::localtime(&t);

  std::strftime(buffer, USD_MAX_TIME, "%A, %B %d, %Y %I:%M:%S %p", ptm);

  KLI_strncpy(r_time, buffer, USD_MAX_TIME);
}


#define GET_TIME() PIL_check_seconds_timer()

typedef struct TimedFunction
{
  struct TimedFunction *next, *prev;
  KLI_timer_func func;
  KLI_timer_data_free user_data_free;
  void *user_data;
  double next_time;
  uintptr_t uuid;
  bool tag_removal;
  bool persistent;
} TimedFunction;

typedef struct TimerContainer
{
  ListBase funcs;
} TimerContainer;

static TimerContainer GlobalTimer = {{0}};

void KLI_timer_register(uintptr_t uuid,
                        KLI_timer_func func,
                        void *user_data,
                        KLI_timer_data_free user_data_free,
                        double first_interval,
                        bool persistent)
{
  TimedFunction *timed_func = static_cast<TimedFunction *>(
    MEM_callocN(sizeof(TimedFunction), __func__));
  timed_func->func = func;
  timed_func->user_data_free = user_data_free;
  timed_func->user_data = user_data;
  timed_func->next_time = GET_TIME() + first_interval;
  timed_func->tag_removal = false;
  timed_func->persistent = persistent;
  timed_func->uuid = uuid;

  KLI_addtail(&GlobalTimer.funcs, timed_func);
}

static void clear_user_data(TimedFunction *timed_func)
{
  if (timed_func->user_data_free) {
    timed_func->user_data_free(timed_func->uuid, timed_func->user_data);
    timed_func->user_data_free = NULL;
  }
}

bool KLI_timer_unregister(uintptr_t uuid)
{
  LISTBASE_FOREACH(TimedFunction *, timed_func, &GlobalTimer.funcs)
  {
    if (timed_func->uuid == uuid && !timed_func->tag_removal) {
      timed_func->tag_removal = true;
      clear_user_data(timed_func);
      return true;
    }
  }
  return false;
}

bool KLI_timer_is_registered(uintptr_t uuid)
{
  LISTBASE_FOREACH(TimedFunction *, timed_func, &GlobalTimer.funcs)
  {
    if (timed_func->uuid == uuid && !timed_func->tag_removal) {
      return true;
    }
  }
  return false;
}

static void execute_functions_if_necessary(void)
{
  double current_time = GET_TIME();

  LISTBASE_FOREACH(TimedFunction *, timed_func, &GlobalTimer.funcs)
  {
    if (timed_func->tag_removal) {
      continue;
    }
    if (timed_func->next_time > current_time) {
      continue;
    }

    double ret = timed_func->func(timed_func->uuid, timed_func->user_data);

    if (ret < 0) {
      timed_func->tag_removal = true;
    } else {
      timed_func->next_time = current_time + ret;
    }
  }
}

static void remove_tagged_functions(void)
{
  for (TimedFunction *timed_func = (TimedFunction *)GlobalTimer.funcs.first; timed_func;) {
    TimedFunction *next = timed_func->next;
    if (timed_func->tag_removal) {
      clear_user_data(timed_func);
      KLI_freelinkN(&GlobalTimer.funcs, timed_func);
    }
    timed_func = next;
  }
}

void KLI_timer_execute()
{
  execute_functions_if_necessary();
  remove_tagged_functions();
}

void KLI_timer_free()
{
  LISTBASE_FOREACH(TimedFunction *, timed_func, &GlobalTimer.funcs)
  {
    timed_func->tag_removal = true;
  }

  remove_tagged_functions();
}

static void remove_non_persistent_functions(void)
{
  LISTBASE_FOREACH(TimedFunction *, timed_func, &GlobalTimer.funcs)
  {
    if (!timed_func->persistent) {
      timed_func->tag_removal = true;
    }
  }
}

void KLI_timer_on_file_load(void)
{
  remove_non_persistent_functions();
}
