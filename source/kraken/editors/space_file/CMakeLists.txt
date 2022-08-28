set(INC
  .
  ../include/.
  ../../../../. # Pixar USD
  ../../../../source
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
  FILE_list.cpp

  ../include/ED_defines.h
  ../include/ED_fileselect.h
)

set(LIB
  maelstrom
  maelstrom_static
  kraken_anchor
  kraken_kernel
  ${TBB_LIBRARIES}
  ${BOOST_LIBRARIES}
  ${PYTHON_LIBRARIES}
  ${VULKAN_LIBRARIES}
  ${OPENGL_LIBRARIES}
)

kraken_add_lib(kraken_editor_spacefile "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")