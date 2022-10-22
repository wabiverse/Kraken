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
  ../krakfont
  ../draw
  ../gpu
  ../universe
  ../python
  ../../../intern/atomic
  ../../../intern/clog
  ../../../intern/guardedalloc
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
  intern/wm_keymap.cpp
  intern/wm_menu_type.cpp
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
  WM_keymap.h
  WM_menu_type.h
  WM_msgbus.h
  WM_operators.h
  WM_tokens.h
  WM_tooltip.h
  WM_window.h
  WM_window.hh
)

set(LIB
  maelstrom
  kraken_editor_screen
  ${BOOST_LIBRARIES}
  ${TBB_LIBRARIES}
  ${PYTHON_LIBRARIES}
  ${VULKAN_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${PTHREADS_LIBRARY}
)

add_definitions(-DKRAKEN_WM_EXPORTS)

kraken_add_lib(kraken_wm "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")

add_dependencies(kraken_wm kraken_universe)
add_dependencies(kraken_wm kraken_luxo)