set(INC
  .
  ../include/.
  ../../../../.
  ../../anchor
  ../../covalib
  ../../covakernel
  ../../universe
  ../../wm
)

set(INC_SYS
  ${Boost_INCLUDE_DIRS}
  ${TBB_INCLUDE_DIRS}
  ${PYTHON_INCLUDE_DIR}
  ${OPENGL_INCLUDE_DIRS}
)

set(SRC  
  SCREEN_area.cpp
  SCREEN_edit.cpp
  SCREEN_geometry.cpp
  SCREEN_workspace_layout_edit.cpp

  ../include/ED_defines.h
  ../include/ED_screen.h
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

covah_add_lib(covah_editor_screen "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")