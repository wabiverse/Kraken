# SPDX-License-Identifier: GPL-2.0-or-later

set(INC
  .
  ../include
  ../../../../. # Pixar USD
  ../../../../source
  ../../krakfont
  ../../krakernel
  ../../kraklib
  # ../../hydra
  # ../../functions
  # ../../gpencil_modifiers
  ../../gpu
  # ../../ikplugin
  ../../imbuf
  ../../universe
  ../../luxo
  # ../../modifiers
  ../../python
  ../../render
  # ../../shader_fx
  ../../wm
  ../../../../intern/clog
  ../../../../intern/guardedalloc

  ${CMAKE_BINARY_DIR}/source/kraken/universe/intern
  # RNA_prototypes.h
  ${CMAKE_BINARY_DIR}/source/kraken/luxo
)

set(SRC
  object_ops.cc
)

set(LIB
  kraken_kernel
  kraken_lib
  # kraken_editor_mesh
  kraken_render
  kraken_wm
)


if(WITH_PYTHON)
  add_definitions(-DWITH_PYTHON)
endif()

if(WITH_EXPERIMENTAL_FEATURES)
  add_definitions(-DWITH_SIMULATION_DATABLOCK)
  add_definitions(-DWITH_POINT_CLOUD)
endif()

kraken_add_lib(kraken_editor_object "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")

add_dependencies(kraken_editor_object kraken_universe)
# RNA_prototypes.h
add_dependencies(kraken_editor_object kraken_luxo)
