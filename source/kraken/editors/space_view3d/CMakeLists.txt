set(INC
  .
  ../../../../. # Pixar USD
  ../../../../source
  ../include/.
  ../../anchor
  ../../kraklib
  ../../krakernel
  ../../luxo
  ../../universe
  ../../wm
  ../../../../intern/atomic
  ../../../../intern/clog
  ../../../../intern/guardedalloc
)

set(INC_SYS
  ${Boost_INCLUDE_DIRS}
  ${TBB_INCLUDE_DIRS}
  ${PYTHON_INCLUDE_DIR}
  ${OPENGL_INCLUDE_DIRS}
  ${VULKAN_INCLUDE_DIRS}
)

set(SRC
  VIEW3D_debug.cpp
  VIEW3D_init.cpp
  VIEW3D_runtime.cpp
  VIEW3D_tokens.cpp
)

set(LIB
  maelstrom
  maelstrom_static
  kraken_kernel
  ${TBB_LIBRARIES}
  ${BOOST_LIBRARIES}
  ${PYTHON_LIBRARIES}
  ${VULKAN_LIBRARIES}
  ${OPENGL_LIBRARIES}
)

add_definitions(-DVIEW3D_EDITOR_EXPORTS)

kraken_add_lib(kraken_editor_space_view3d "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")