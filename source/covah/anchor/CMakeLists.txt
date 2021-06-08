set(INC
  .
  ./backend
  ./intern
  ./misc/freetype
  ../../../.
  ../covakernel
  ../covalib
)

set(SRC
  backend/ANCHOR_impl_sdl.cpp
  backend/ANCHOR_impl_vulkan.cpp

  misc/freetype/ANCHOR_freetype.cpp

  intern/api.cpp
  intern/debug_codes.cpp
  intern/demo.cpp
  intern/draw.cpp
  intern/tables.cpp
  intern/vulkan.cpp
  intern/widgets.cpp

  backend/ANCHOR_impl_sdl.h
  backend/ANCHOR_impl_vulkan.h

  misc/freetype/ANCHOR_freetype.h

  ANCHOR_api.h
  ANCHOR_debug_codes.h
  ANCHOR_internal.h
  ANCHOR_rectpack.h
  ANCHOR_textedit.h
  ANCHOR_truetype.h
  ANCHOR_vulkan.h
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
  covahverse
  ${BOOST_LIBRARIES}
  ${TBB_LIBRARIES}
  ${PYTHON_LIBRARIES}
  ${SDL2_LIBRARIES}
  ${Vulkan_LIBRARIES}
  ${FREETYPE_LIBRARIES}
)

add_definitions(-DANCHOR_ENABLE_FREETYPE)

covah_add_lib(covah_anchor "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")
