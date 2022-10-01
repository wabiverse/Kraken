# SPDX-License-Identifier: GPL-2.0-or-later

set(INC
  .
  ../atomic
  ../guardedalloc
)

set(INC_SYS
)

set(SRC
  clog.c

  CLG_log.h
)

set(LIB
)

# Disabled for universe/luxo.
add_definitions(-DWITH_CLOG_PTHREADS)

kraken_add_lib(kraken_intern_clog "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")
