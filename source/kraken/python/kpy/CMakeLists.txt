set(INC
  .
  ./intern/.
  ../../../../. # Pixar USD
  ../editors/include
  ../../krakernel
  ../../kraklib
  ../../universe
  ../../wm
)

set(KPY_INSTALL_PATH "release/scripts/modules/kpy")

set(SRC
  intern/kpy_init_exit.cpp
  intern/kpy_interface.cpp
  intern/kpy_intern_string.cpp
  # intern/kpy_module.cpp
  intern/kpy_uni.cpp

  intern/kpy_interface.h
  intern/kpy_intern_string.h
  intern/kpy_uni.h

  KPY_api.h
  KPY_init_exit.h
)

set(INC_SYS
  ${TBB_INCLUDE_DIR}
  ${Boost_INCLUDE_DIRS}
  ${PYTHON_INCLUDE_DIR}
)

set(LIB
  maelstrom
  kraken_kernel
  kraken_universe
  ${BOOST_LIBRARIES}
  ${TBB_LIBRARIES}
  ${PYTHON_LINKFLAGS}
  ${PYTHON_LIBRARIES}
)

configure_file(module/__init__.py.in ${CMAKE_SOURCE_DIR}/${KPY_INSTALL_PATH}/__init__.py)

add_definitions(-DKRAKEN_PYTHON_EXPORTS)

kraken_add_lib(kraken_python "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")