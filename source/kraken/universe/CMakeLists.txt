set(INC
  .
  ../../../. # Pixar USD
  ../../../source
  ../editors/include
  ../luxo
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
  intern/USD_create.cpp
  intern/USD_default_tables.cpp
  intern/USD_file.cpp
  intern/USD_object.cpp
  intern/USD_pixar_utils.cpp

  USD_api.h
  USD_area.h
  USD_color_types.h
  USD_context.h
  USD_curveprofile_types.h
  USD_default_tables.h
  USD_factory.h
  USD_file.h
  USD_object.h
  USD_operator.h
  USD_path_defaults.h
  USD_pixar_utils.h
  USD_region.h
  USD_scene.h
  USD_screen.h
  USD_system.h
  USD_texture_types.h
  USD_types.h
  USD_userpref.h
  USD_wm_types.h
  USD_window.h
  USD_workspace.h
)

set(LIB
  maelstrom
  maelstrom_static
  kraken_lib
  kraken_kernel
  ${BOOST_LIBRARIES}
  ${TBB_LIBRARIES}
  ${PYTHON_LIBRARIES}
)

add_definitions(-DKRAKEN_USD_EXPORTS)

kraken_add_lib(kraken_universe "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")