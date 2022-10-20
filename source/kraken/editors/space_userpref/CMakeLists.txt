# SPDX-License-Identifier: GPL-2.0-or-later

set(INC
  ../include
  ../../../../. # Pixar USD
  ../../../../source
  ../../krakernel
  ../../kraklib
  ../../krakloader
  ../../universe
  ../../luxo
  ../../wm
  ../../../../intern/guardedalloc

  ${CMAKE_CURRENT_BINARY_DIR}/../../universe/intern
)

set(INC_SYS
  ${Boost_INCLUDE_DIRS}
  ${TBB_INCLUDE_DIRS}
  ${PYTHON_INCLUDE_DIR}
)

set(SRC
  # space_userpref.c
  userpref_ops.cc
  userpref_tokens.cc

  # userpref_intern.h
)

set(LIB
  maelstrom
  maelstrom_static
  ${TBB_LIBRARIES}
  ${BOOST_LIBRARIES}
  ${PYTHON_LIBRARIES}
)

kraken_add_lib(kraken_editor_space_userpref "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")

add_dependencies(kraken_editor_space_userpref kraken_universe)
