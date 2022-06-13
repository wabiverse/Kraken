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

#include "UNI_api.h"

#include "KLI_time.h"
#include "KLI_string_utils.h"

#ifdef __linux__
#  include <sys/time.h>
#  include <unistd.h>

double PIL_check_seconds_timer(void)
{
  struct timeval tv;
  struct timezone tz;

  gettimeofday(&tv, &tz);

  return ((double)tv.tv_sec + tv.tv_usec / 1000000.0);
}

#elif _WIN32
#  include "stdlib.h"
#  include <windows.h>
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
  char buffer[UNI_MAX_TIME];

  std::tm *ptm = std::localtime(&t);

  std::strftime(buffer, UNI_MAX_TIME, "%A, %B %d, %Y %I:%M:%S %p", ptm);

  KLI_strncpy(r_time, buffer, UNI_MAX_TIME);
}
