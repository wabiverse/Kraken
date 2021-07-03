set(INC
  .
  ./zep
  ./zep/imgui
  ./zep/qt
  ./zep/mcommon/animation
  ./zep/mcommon/file
  ./zep/mcommon/math
  ./zep/mcommon/string
  ./zep/mcommon/utf8
  ../include/.
  ../../../../.
  ../../anchor
  ../../covalib
  ../../covakernel
  ../../universe
)

set(INC_SYS
  ${Boost_INCLUDE_DIRS}
  ${TBB_INCLUDE_DIRS}
  ${PYTHON_INCLUDE_DIR}
  ${OPENGL_INCLUDE_DIRS}
)

set(SRC
  intern/buffer.cpp
  intern/commands.cpp
  intern/display.cpp
  intern/editor.cpp
  intern/filesystem.cpp
  intern/glyph_iterator.cpp
  intern/indexer.cpp
  intern/keymap.cpp
  intern/line_widgets.cpp
  intern/mode_search.cpp
  intern/mode_standard.cpp
  intern/mode_tree.cpp
  intern/mode_vim.cpp
  intern/mode.cpp
  intern/murmur_hash.cpp
  intern/path.cpp
  intern/range_markers.cpp
  intern/regress.cpp
  intern/scroller.cpp
  intern/splits.cpp
  intern/stringutils.cpp
  intern/syntax_markdown.cpp
  intern/syntax_providers.cpp
  intern/syntax_rainbow_brackets.cpp
  intern/syntax_tree.cpp
  intern/syntax.cpp
  intern/tab_window.cpp
  intern/theme.cpp
  intern/timer.cpp
  intern/window.cpp
  
  CODE_editor.cpp

  zep/buffer.h
  zep/commands.h
  zep/display.h
  zep/editor.h
  zep/filesystem.h
  zep/gap_buffer.h
  zep/glyph_iterator.h
  zep/indexer.h
  zep/keymap.h
  zep/line_widgets.h
  zep/mode.h
  zep/mode_search.h
  zep/mode_standard.h
  zep/mode_tree.h
  zep/mode_vim.h
  zep/range_markers.h
  zep/regress.h
  zep/scroller.h
  zep/splits.h
  zep/syntax.h
  zep/syntax_markdown.h
  zep/syntax_providers.h
  zep/syntax_rainbow_brackets.h
  zep/syntax_tree.h
  zep/tab_window.h
  zep/theme.h
  zep/window.h
  zep/zep_config.h  

  zep/imgui/console_imgui.h
  zep/imgui/display_imgui.h
  zep/imgui/editor_imgui.h
  zep/imgui/usb_hid_keys.h
  
  zep/mcommon/callback.h
  zep/mcommon/gsl-lite.hpp
  zep/mcommon/logger.h
  zep/mcommon/platform.h
  zep/mcommon/registrar.h
  zep/mcommon/signals.h
  zep/mcommon/threadpool.h
  zep/mcommon/threadutils.h
  zep/mcommon/utf8.h

  zep/mcommon/animation/timer.h

  zep/mcommon/file/cpptoml.h
  zep/mcommon/file/fnmatch.h
  zep/mcommon/file/path.h

  zep/mcommon/math/math.h

  zep/mcommon/string/murmur_hash.h
  zep/mcommon/string/stringutils.h

  zep/mcommon/utf8/checked.h
  zep/mcommon/utf8/core.h
  zep/mcommon/utf8/cpp11.h
  zep/mcommon/utf8/unchecked.h

  zep/qt/common_qt.h
  zep/qt/zepdisplay_qt.h
  zep/qt/zepwidget_qt.h

  zep.h

  ../include/ED_defines.h
  ../include/ED_code.h
)

set(LIB
  krakenverse
  kraken_anchor
  ${TBB_LIBRARIES}
  ${BOOST_LIBRARIES}
  ${PYTHON_LIBRARIES}
  ${VULKAN_LIBRARIES}
  ${OPENGL_LIBRARIES}
)

add_definitions(-DZEP_USE_SDL)
add_definitions(-DCODE_EDITOR_EXPORTS)

kraken_add_lib(kraken_editor_code "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")