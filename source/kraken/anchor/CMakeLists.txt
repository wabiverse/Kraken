set(INC
  .
  ./backend
  ./intern
  ../../../.
  ../krakernel
  ../kraklib
)

# ------------------------------------ SHARED BACKENDS -----
list (APPEND SRC_PLATFORM_BACKENDS
  # Everyone gets Vulkan.
  backend/backend_vulkan.cpp
  backend/ANCHOR_BACKEND_vulkan.h
)

# -------------------------------------- UNIX BACKENDS -----
if(UNIX)
  # Unix currently gets SDL.
  list(APPEND SRC_PLATFORM_BACKENDS
    backend/backend_sdl.cpp
    backend/ANCHOR_BACKEND_sdl.h
  )
endif()

# ------------------------------------- WIN32 BACKENDS -----
if(WIN32)
  # Windows currently gets native Win32 Support.
  list(APPEND SRC_PLATFORM_BACKENDS
    backend/backend_win32.cpp
    backend/ANCHOR_BACKEND_win32.h
  )
endif()

# --------------------------------------- SHARED SOURCE -----
set(SRC
  # All Systems ->
  intern/system_interfaces.cpp

  # Platform Agnostic Implementation.
  intern/api.cpp
  intern/buttons.cpp
  intern/debug_codes.cpp
  intern/demo.cpp
  intern/display_manager.cpp
  intern/draw.cpp
  intern/event_consumer.cpp
  intern/event_manager.cpp
  intern/freetype.cpp
  intern/modifier_keys.cpp
  intern/anchor_rect.cpp
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
  ANCHOR_debug_codes.h
  ANCHOR_display_manager.h
  ANCHOR_event_consumer.h
  ANCHOR_event_manager.h
  ANCHOR_event.h
  ANCHOR_freetype.h
  ANCHOR_internal.h
  ANCHOR_modifier_keys.h
  ANCHOR_rectpack.h
  ANCHOR_Rect.h
  ANCHOR_system_paths.h
  ANCHOR_system.h
  ANCHOR_textedit.h
  ANCHOR_truetype.h
  ANCHOR_window_manager.h
  ANCHOR_window.h
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
  krakenverse
  ${BOOST_LIBRARIES}
  ${TBB_LIBRARIES}
  ${PYTHON_LIBRARIES}
  ${SDL2_LIBRARIES}
  ${VULKAN_LIBRARIES}
  ${FREETYPE_LIBRARIES}
)

add_definitions(-DANCHOR_ENABLE_FREETYPE)

kraken_add_lib(kraken_anchor "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")
