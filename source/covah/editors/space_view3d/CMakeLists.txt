set(INC
  .
  ../../../../.
  ../include/.
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
  ${VULKAN_INCLUDE_DIRS}
)

set(SRC
  VIEW3D_debug.cpp
  VIEW3D_init.cpp
  VIEW3D_runtime.cpp
  VIEW3D_tokens.cpp

  ../include/ED_defines.h
  ../include/ED_debug_codes.h
  ../include/ED_tokens.h
  ../include/ED_view3d.h
)

set(LIB
  covahverse
  covah_anchor
  ${TBB_LIBRARIES}
  ${BOOST_LIBRARIES}
  ${PYTHON_LIBRARIES}
  ${VULKAN_LIBRARIES}
  ${OPENGL_LIBRARIES}
)

add_definitions(-DVIEW3D_EDITOR_EXPORTS)

covah_add_lib(covah_editor_spaceview3d "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")