set(INC
  .
  ../../krakernel
  ../../kraklib
  ../../gpu
  ../../universe
  ../../luxo
  ../../../../intern/guardedalloc
)

set(INC_SYS
  ${Epoxy_INCLUDE_DIRS}
  ${PYTHON_INCLUDE_DIR}
)

set(SRC
  py_capi_utils.c

  py_capi_utils.h

  # header-only
  python_utildefines.h
)

set(LIB
  ${Epoxy_LIBRARIES}
  ${PYTHON_LINKFLAGS}
  ${PYTHON_LIBRARIES}
)

kraken_add_lib(kraken_python_ext "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")
