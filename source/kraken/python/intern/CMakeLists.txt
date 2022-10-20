set(INC
  ..
  ../../../../. # Pixar USD
  ../../../../source
  ../../editors/include
  ../../luxo
  ../../krakernel
  ../../kraklib
  ../../universe
  ../../wm
  ../../../../intern/clog
  ../../../../intern/guardedalloc
  ${CMAKE_BINARY_DIR}/source/kraken/luxo
)

set(SRC
  kpy_app.cpp
  kpy_capi_packarray.c
  kpy_capi_utils.cpp
  kpy_driver.cpp
  kpy_interface_atexit.cpp
  kpy_interface_run.cpp
  kpy_interface.cpp
  kpy_intern_string.cpp
  kpy_library_load.cpp
  kpy_path.cpp
  kpy_pixar_data.cpp
  kpy_stage.cpp
  kpy.cpp

  kpy_app.h
  kpy_capi_packarray.h
  kpy_capi_utils.h
  kpy_driver.h
  kpy_interface.h
  kpy_intern_string.h
  kpy_library.h
  kpy_path.h
  kpy_stage.h
  kpy.h

  wabi_python.h

  ../KPY_api.h
  ../KPY_extern_python.h
  ../KPY_extern_run.h
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
  kraken_editor_interface
  kraken_editor_space_api
  kraken_python_ext
  kraken_python_gpu
  _tf
  _gf
  _trace
  _work
  _plug
  _vt
  _ar
  _kind
  _sdf
  _ndr
  _sdr
  _pcp
  _usd
  _usdGeom
  _usdVol
  _usdMedia
  _usdShade
  _usdLux
  _usdRender
  _usdHydra
  _usdRi
  _usdSkel
  _usdUI
  _usdUtils
  _usdPhysics
  _usdAbc
  _usdDraco
  _garch
  _cameraUtil
  _pxOsd
  _glf
  _usdImagingGL
  _usdAppUtils
  _usdviewq
  ${BOOST_LIBRARIES}
  ${TBB_LIBRARIES}
  ${PYTHON_LINKFLAGS}
  ${PYTHON_LIBRARIES}
  ${PTHREADS_LIBRARY}
)

if(WITH_BUILDINFO)
  add_definitions(-DBUILD_DATE)
endif()

add_definitions(-DKRAKEN_PYTHON_EXPORTS)

kraken_add_lib(kraken_python "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")