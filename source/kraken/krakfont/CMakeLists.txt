# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright 2008 Blender Foundation. All rights reserved.

set(INC
  .
  ../krakernel
  ../kraklib
  ../gpu
  ../imbuf
  ../universe
  ../luxo
  ../../../. # Pixar USD
  ../../../source
  ../../../intern/atomic
  ../../../intern/clog
  ../../../intern/guardedalloc
)

set(INC_SYS
  ${FREETYPE_INCLUDE_DIRS}
)

set(SRC
  intern/krf.c
  intern/krf_default.c
  intern/krf_dir.c
  intern/krf_font.c
  intern/krf_font_default.c
  intern/krf_glyph.c
  intern/krf_thumbs.c
  intern/krf_util.c

  KRF_api.h
  intern/krf_internal.h
  intern/krf_internal_types.h
)

set(LIB
  kraken_gpu
  kraken_intern_guardedalloc

  ${FREETYPE_LIBRARIES} ${BROTLI_LIBRARIES}
)

if(WIN32)
  list(APPEND SRC
    intern/krf_font_win32_compat.c
  )
endif()

if(WITH_PYTHON)
  add_definitions(-DWITH_PYTHON)
  list(APPEND INC
    ../python
  )
endif()

kraken_add_lib(kraken_krakfont "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")
