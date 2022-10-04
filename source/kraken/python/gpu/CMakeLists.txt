set(INC
  .
  ../../../../. # Pixar USD
  ../../../../source
  ../../editors/include
  ../../gpu
  ../../imbuf
  ../../luxo
  ../../krakernel
  ../../kraklib
  ../../universe
  ../../wm
  ../../../../intern/guardedalloc
)

set(SRC
  gpu_py_api.c
  gpu_py_matrix.c

  gpu_py_api.h
  gpu_py_matrix.h
)

set(INC_SYS
  ${TBB_INCLUDE_DIR}
  ${Boost_INCLUDE_DIRS}
  ${PYTHON_INCLUDE_DIR}
  ${PTHREADS_INCLUDE_DIR}
)

set(LIB
  maelstrom
  maelstrom_static
  ${BOOST_LIBRARIES}
  ${TBB_LIBRARIES}
  ${PYTHON_LINKFLAGS}
  ${PYTHON_LIBRARIES}
  ${PTHREADS_LIBRARY}
)

kraken_add_lib(kraken_python_gpu "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")
