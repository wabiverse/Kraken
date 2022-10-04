# SPDX-License-Identifier: GPL-2.0-or-later

set(INC
  ../include
  # ../space_sequencer
  # ../../krakenfont
  ../../krakernel
  ../../kraklib
  ../../gpu
  ../../imbuf
  ../../universe
  ../../luxo
  # ../../sequencer
  ../../wm
  ../../../../intern/atomic
  ../../../../intern/clog
  ../../../../intern/guardedalloc

  # PRIM_prototypes.h
  ${CMAKE_BINARY_DIR}/source/kraken/luxo
)


set(SRC
  ed_util.cpp

  # general includes
  ../include/ED_code.h
  ../include/ED_debug_codes.h
  ../include/ED_defines.h
  ../include/ED_fileselect.h
  ../include/ED_screen.h
  ../include/ED_tokens.h
  ../include/ED_util.h
  ../include/ED_view3d.h
  ../include/ED_icons.h
  ../include/ED_interface_icons.h
  ../include/ED_interface.h
  ../include/ED_resources.h
  ../include/ED_tokens.h
  ../include/UI_abstract_view.hh
  ../include/UI_icons.h
  ../include/UI_interface_icons.h
  ../include/UI_interface.h
  ../include/UI_resources.h
  ../include/UI_tokens.h
  ../include/UI_view2d.h
)

set(LIB
)


if(WITH_PYTHON)
  add_definitions(-DWITH_PYTHON)
  list(APPEND INC
    ../../python
  )
endif()

kraken_add_lib(kraken_editor_util "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")

# RNA_prototypes.h
add_dependencies(kraken_editor_util kraken_luxo)
