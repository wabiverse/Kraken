set(INC
  .
  ./intern/.
  ../../../../. # Pixar USD
  ../editors/include
  ../../luxo
  ../../krakernel
  ../../kraklib
  ../../universe
  ../../wm
)

# set(KPY_INSTALL_PATH "release/scripts/modules/kpy")

set(SRC
  intern/kpy_app.cpp
  intern/kpy_capi_packarray.c
  intern/kpy_capi_utils.cpp
  intern/kpy_driver.cpp
  intern/kpy_init_exit.cpp
  intern/kpy_interface_atexit.cpp
  intern/kpy_interface_run.cpp
  intern/kpy_interface.cpp
  intern/kpy_intern_string.cpp
  intern/kpy_library_load.cpp
  intern/kpy_path.cpp
  intern/kpy_uni.cpp
  intern/kpy.cpp

  intern/kpy_app.h
  intern/kpy_capi_packarray.h
  intern/kpy_capi_utils.h
  intern/kpy_driver.h
  intern/kpy_interface.h
  intern/kpy_intern_string.h
  intern/kpy_library.h
  intern/kpy_path.h
  intern/kpy_uni.h
  intern/kpy_utildefines.h
  intern/kpy.h

  KPY_api.h
  KPY_extern_python.h
  KPY_extern_run.h
  KPY_init_exit.h
)

set(INC_SYS
  ${TBB_INCLUDE_DIR}
  ${Boost_INCLUDE_DIRS}
  ${PYTHON_INCLUDE_DIR}
  ${PTHREADS_INCLUDE_DIR}
)

set(LIB
  maelstrom
  kraken_kernel
  kraken_luxo
  kraken_universe
  ${BOOST_LIBRARIES}
  ${TBB_LIBRARIES}
  ${PYTHON_LINKFLAGS}
  ${PYTHON_LIBRARIES}
  ${PTHREADS_LIBRARY}
)

# configure_file(module/__init__.py.in ${CMAKE_SOURCE_DIR}/${KPY_INSTALL_PATH}/__init__.py)

add_definitions(-DKRAKEN_PYTHON_EXPORTS)

kraken_import_nuget_packages("source/kraken/python/kpy/kraken_python")
kraken_add_lib(kraken_python "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")