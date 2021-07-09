set(INC
  .
  ../../../. # Pixar USD
  ../editors/include
  ../krakernel
  ../kraklib
  ../wm
)

set(INC_SYS
  ${Boost_INCLUDE_DIRS}
  ${TBB_INCLUDE_DIRS}
  ${PYTHON_INCLUDE_DIR}
)

set(SRC
  intern/UNI_create.cpp
  intern/UNI_default_tables.cpp
  intern/UNI_pixar_utils.cpp

  UNI_api.h
  UNI_area.h
  UNI_context.h
  UNI_default_tables.h
  UNI_factory.h
  UNI_object.h
  UNI_operator.h
  UNI_path_defaults.h
  UNI_pixar_utils.h
  UNI_region.h
  UNI_scene.h
  UNI_screen.h
  UNI_system.h
  UNI_userpref.h
  UNI_wm_types.h
  UNI_window.h
  UNI_workspace.h
)

set(LIB
  krakenverse
  kraken_lib
  kraken_kernel
  ${BOOST_LIBRARIES}
  ${TBB_LIBRARIES}
  ${PYTHON_LIBRARIES}
)

add_definitions(-DKRAKEN_UNIVERSE_EXPORTS)
kraken_add_lib(kraken_universe "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")