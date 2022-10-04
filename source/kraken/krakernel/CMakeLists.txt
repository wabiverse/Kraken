set(INC
  .
  ../../../. # Pixar USD
  ../../../source
  ../anchor
  ../anchor/intern
  ../anchor/backend
  ../editors/include
  ../kraklib
  ../wm
  ../luxo
  ../universe
  ../gatekeeper
  ../python
  ../../../intern/atomic
  ../../../intern/clog
  ../../../intern/guardedalloc
)

set(SRC
  intern/KKE_appdir.cpp
  intern/KKE_context.cpp
  intern/KKE_kraken.cpp
  intern/KKE_diagnostics.cpp
  intern/KKE_icons.cpp
  intern/KKE_idprop.cpp
  intern/KKE_lib_id.cpp
  intern/KKE_plugins.cpp
  intern/KKE_report.cpp
  intern/KKE_scene.cpp
  intern/KKE_screen.cpp
  intern/KKE_utils.cpp
  intern/KKE_workspace.cpp
  intern/appdir.c
  intern/cryptomatte.cc

  KKE_api.h
  KKE_appdir.h
  KKE_appdir.hh
  KKE_colortools.h
  KKE_cryptomatte.h
  KKE_cryptomatte.hh
  KKE_context.h
  KKE_global.h
  KKE_icons.h
  KKE_idprop.h
  KKE_idtype.h
  KKE_lib_id.h
  KKE_main.h
  KKE_report.h
  KKE_robinhood.h
  KKE_scene.h
  KKE_screen.h
  KKE_space.h
  KKE_utils.h
  KKE_workspace.h
)

set(INC_SYS
  ${Boost_INCLUDE_DIRS}
  ${TBB_INCLUDE_DIRS}
  ${PYTHON_INCLUDE_DIR}
  ${SDL2_INCLUDE_DIR}
  ${Vulkan_INCLUDE_DIRS}
  ${PTHREADS_INCLUDE_DIR}
)

set(LIB
  maelstrom
  maelstrom_static
  kraken_anchor
  kraken_draw
  kraken_gpu
  kraken_imbuf
  kraken_krakfont
  kraken_lib
  kraken_luxo
  kraken_python
  kraken_intern_clog
  kraken_intern_guardedalloc
  ${TBB_LIBRARIES}
  ${BOOST_LIBRARIES}
  ${PYTHON_LIBRARIES}
  ${SDL2_LIBRARIES}
  ${VULKAN_LIBRARIES}
  ${PTHREADS_LIBRARY}
)

if(WIN32)
  list(APPEND INC
    ../../../intern/utfconv
  )

  list(APPEND LIB
    kraken_intern_utfconv
  )
endif()

if(WITH_PYTHON)
  get_filename_component(_python_exe_name ${PYTHON_EXECUTABLE} NAME)
  add_definitions(-DPYTHON_EXECUTABLE_NAME=${_python_exe_name})
  unset(_python_exe_name)
endif()

add_definitions(-DKRAKEN_KERNEL_EXPORTS)

kraken_add_lib(kraken_kernel "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")