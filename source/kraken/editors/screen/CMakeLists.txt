set(INC
  .
  ../include/.
  ../../../../. # Pixar USD
  ../../anchor
  ../../kraklib
  ../../krakernel
  ../../luxo
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
  SCREEN_workspace_edit.cpp
  SCREEN_workspace_layout_edit.cpp

  ../include/ED_defines.h
  ../include/ED_screen.h
)

set(LIB
  maelstrom
  kraken_anchor
  kraken_kernel
  ${TBB_LIBRARIES}
  ${BOOST_LIBRARIES}
  ${PYTHON_LIBRARIES}
  ${VULKAN_LIBRARIES}
  ${OPENGL_LIBRARIES}
)

kraken_import_nuget_packages("source/kraken/editors/screen/kraken_editor_screen")
kraken_add_lib(kraken_editor_screen "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")