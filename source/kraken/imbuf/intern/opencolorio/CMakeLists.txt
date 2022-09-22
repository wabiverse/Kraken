# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright 2012 Blender Foundation. All rights reserved.

set(INC
  .
)

set(INC_SYS
  ${OPENCOLORIO_INCLUDE_DIRS}
  ${Epoxy_INCLUDE_DIRS}
)

set(SRC
  ocio_capi.cc
  ocio_impl.cc

  ocio_capi.h
  ocio_impl.h
)

set(LIB
  ${OPENCOLORIO_LIBRARIES}
  ${ALL_IMAGING_LIBRARIES}
)

add_definitions(-DWITH_OCIO)

add_definitions(${OPENCOLORIO_DEFINITIONS})

if(WIN32)
  list(APPEND INC_SYS
    ${BOOST_INCLUDE_DIR}
  )
  list(APPEND LIB
    ${BOOST_LIBRARIES}
  )
endif()

list(APPEND INC ${CMAKE_CURRENT_BINARY_DIR})


kraken_add_lib(kraken_intern_opencolorio "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")
