set(INC
  .
  ../../../. # Pixar USD
  ../../../source
  ../../../source/kraken
  ./apple
  ./interop
  ./backend
  ./intern
  ../krakernel
  ../kraklib
  ../universe

  ../../../intern/guardedalloc
  # ${CMAKE_SOURCE_DIR}/source/kraken/anchor
)

# ------------------------------------ SHARED BACKENDS -----
list (APPEND SRC_PLATFORM_BACKENDS

)

# -------------------------------------- UNIX BACKENDS -----
if(UNIX AND NOT APPLE)
  # Unix currently gets SDL.
  list(APPEND SRC_PLATFORM_BACKENDS
    backend/backend_sdl.cpp
    backend/ANCHOR_BACKEND_sdl.h
    backend/backend_vulkan.cpp
    backend/ANCHOR_BACKEND_vulkan.h
  )
endif()

if (APPLE)
  list(APPEND SRC_APPLE_BACKEND
    apple/anchor_system.swift
    ${CMAKE_SOURCE_DIR}/source/creator/kraken.docc
  )
  list(APPEND SRC_PLATFORM_BACKENDS
    intern/system_paths.mm
    backend/backend_cocoa.cpp
    backend/context_metal.cpp
    backend/ANCHOR_BACKEND_cocoa.h
    backend/ANCHOR_BACKEND_metal.h
    backend/ANCHOR_CONTEXT_metal.h
  )
endif()

# ------------------------------------- WIN32 BACKENDS -----
if(WIN32)
  # Windows currently gets native Win32 Support.
  list(APPEND SRC_PLATFORM_BACKENDS
    backend/backend_win32.cpp
    backend/ANCHOR_BACKEND_win32.h
    backend/backend_vulkan.cpp
    backend/ANCHOR_BACKEND_vulkan.h
  )
endif()

# --------------------------------------- SHARED SOURCE -----
set(SRC
  # All Systems ->
  intern/system_interfaces.cpp

  # Platform Agnostic Implementation.
  intern/anchor_rect.cpp
  intern/api.cpp
  intern/buttons.cpp
  intern/curve_edit.cpp
  intern/debug_codes.cpp
  intern/demo.cpp
  intern/display_manager.cpp
  intern/draw.cpp
  intern/event_consumer.cpp
  intern/event_manager.cpp
  intern/freetype.cpp
  intern/gizmo.cpp
  intern/gradient.cpp
  intern/graph_editor.cpp
  intern/modifier_keys.cpp
  intern/sequencer.cpp
  intern/system_paths.cpp
  intern/system.cpp
  intern/tables.cpp
  intern/widgets.cpp
  intern/window_manager.cpp
  intern/window.cpp

  ${SRC_PLATFORM_BACKENDS}

  # Platform Agnostic API
  ANCHOR_api.h
  ANCHOR_buttons.h
  ANCHOR_curve_edit.h
  ANCHOR_debug_codes.h
  ANCHOR_display_manager.h
  ANCHOR_event_consumer.h
  ANCHOR_event_manager.h
  ANCHOR_event.h
  ANCHOR_freetype.h
  ANCHOR_gizmo.h
  ANCHOR_gradient.h
  ANCHOR_graph_editor.h
  ANCHOR_internal.h
  ANCHOR_modifier_keys.h
  ANCHOR_path-api.h
  ANCHOR_rectpack.h
  ANCHOR_sequencer.h
  ANCHOR_rect.h
  ANCHOR_system_paths.h
  ANCHOR_system.h
  ANCHOR_textedit.h
  ANCHOR_truetype.h
  ANCHOR_types.h
  ANCHOR_version.h
  ANCHOR_window_manager.h
  ANCHOR_window.h
  ANCHOR_zoom_slider.h
)

set(INC_SYS
  ${Boost_INCLUDE_DIRS}
  ${TBB_INCLUDE_DIRS}
  ${PYTHON_INCLUDE_DIRS}
  ${SDL2_INCLUDE_DIR}
  ${Vulkan_INCLUDE_DIRS}
  ${FREETYPE_INCLUDE_DIRS}
)

set(LIB
  maelstrom
  maelstrom_static
  kraken_os
  kraken_gpu
  kraken_kernel
  ${BOOST_LIBRARIES}
  ${TBB_LIBRARIES}
  ${PYTHON_LIBRARIES}
  ${SDL2_LIBRARIES}
  ${VULKAN_LIBRARIES}
  ${FREETYPE_LIBRARIES}
  ${FREETYPE_LIBRARY}
)

if(WIN32)
  list(APPEND INC
    ../../../intern/utfconv
  )

  list(APPEND LIB
    kraken_intern_utfconv
    comctl32
    shcore
    shlwapi
    d3d12
    d3dcompiler
    dxgi
  )
endif()

kraken_add_lib(kraken_anchor "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")
if (APPLE)
  # to bind the entirety of Pixar USD to Swift.
  # and make it usable via "import Pixar"...
  target_include_directories(kraken_anchor PUBLIC
    ${TBB_INCLUDE_DIRS}
    ${PYTHON_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    ${CMAKE_BINARY_DIR}/bin/include/wabi)
  set_property(SOURCE ${CMAKE_SOURCE_DIR}/source/creator/kraken.docc PROPERTY XCODE_LAST_KNOWN_FILE_TYPE folder.documentationcatalog)
  kraken_add_lib(AnchorSystem "${SRC_APPLE_BACKEND}" "${INC}" "${INC_SYS}" "${LIB}")
  target_compile_options(AnchorSystem PRIVATE
    "-Xcc" "-DWITH_PYTHON"
    "-Xcc" "-DWITH_TBB"
    "-Xcc" "-DWITH_BOOST"
    "-Xcc" "-DWITH_FREETYPE"
    "-Xcc" "-DBOOST_NO_CXX98_FUNCTION_BASE"
    "-Xcc" "-D_LIBCPP_ENABLE_CXX17_REMOVED_UNARY_BINARY_FUNCTION"
    "-static"
    "-emit-dependencies"
    "-emit-library"
    "-emit-module"
    "SHELL:-index-store-path \"${CMAKE_BINARY_DIR}/symbol_index\"")
  add_dependencies(AnchorSystem kraken_anchor)
  target_link_libraries(AnchorSystem PRIVATE kraken_anchor)
endif()
if(WIN32)
  kraken_precompile_headers(kraken_anchor pch.cpp pch.h)
endif()