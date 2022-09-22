set(INC
  .
  ../../../. # Pixar USD
  ../../../source
  ../../../source/kraken
  ../luxo
  ../anchor
  ../editors/include
  ../krakernel
  ../kraklib
  ../universe
  ../python/kpy
)

set(INC_SYS
  ${Boost_INCLUDE_DIRS}
  ${TBB_INCLUDE_DIRS}
  ${OPENSUBDIV_INCLUDE_DIRS}
  ${ZMQ_INCLUDE_DIR}
  ${PYTHON_INCLUDE_DIR}
  ${OPENGL_INCLUDE_DIRS}
  ${PTHREADS_INCLUDE_DIR}
)

set(SRC
  intern/wm_cursors.c
  intern/wm_cursors.cpp
  intern/wm_debug_codes.cpp
  intern/wm_draw.cpp
  intern/wm_dragdrop.cpp
  intern/wm_event_system.cpp
  intern/wm_files.cpp
  intern/wm_init_exit.cpp
  intern/wm_msgbus.cpp
  intern/wm_operators.cpp
  intern/wm_panel_type.cpp
  intern/wm_styles.cpp
  intern/wm_tokens.cpp
  intern/wm_tooltip.cpp
  intern/wm_window.cpp
  intern/wm.cpp

  WM_api.h
  WM_cursors_api.h # for CXX
  WM_cursors.h # for C
  WM_debug_codes.h
  WM_draw.h
  WM_dragdrop.h
  WM_event_system.h
  WM_files.h
  WM_init_exit.h
  WM_inline_tools.h
  WM_msgbus.h
  WM_operators.h
  WM_tokens.h
  WM_tooltip.h
  WM_window.h
)

set(LIB
  maelstrom
  maelstrom_static
  kraken_anchor
  kraken_editor_interface
  kraken_editor_spacefile
  kraken_editor_screen
  kraken_editor_spaceview3d
  # kraken_editor_code
  kraken_lib
  kraken_luxo
  kraken_universe
  ${BOOST_LIBRARIES}
  ${TBB_LIBRARIES}
  ${PYTHON_LIBRARIES}
  ${VULKAN_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${PTHREADS_LIBRARY}
)

add_definitions(-DKRAKEN_WM_EXPORTS)

kraken_add_lib(kraken_wm "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")