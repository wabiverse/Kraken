set(INC
  .
  ../../../../.
  ../editors/include
  ../../covakernel
  ../../covalib
  ../../universe
)

set(WPY_INSTALL_PATH "release/scripts/modules/wpy")

set(SRC
  # intern/WPY_module.cpp
  intern/WPY_init_exit.cpp

  WPY_api.h
  WPY_init_exit.h
)

set(INC_SYS
  ${TBB_INCLUDE_DIR}
  ${Boost_INCLUDE_DIRS}
  ${PYTHON_INCLUDE_DIR}
)

set(LIB
  covahverse

  ${BOOST_LIBRARIES}
  ${TBB_LIBRARIES}
  ${PYTHON_LINKFLAGS}
  ${PYTHON_LIBRARIES}
)

configure_file(module/__init__.py.in ${CMAKE_SOURCE_DIR}/${WPY_INSTALL_PATH}/__init__.py)

add_definitions(-DCOVAH_PYTHON_EXPORTS)

covah_add_lib(covah_python "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")