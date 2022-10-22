set(INC
  .
  ../../../. # Pixar USD
  ../../../source
  ../editors/include
  ../luxo
  ../krakernel
  ../kraklib
  ../wm
  ../../../intern/atomic
  ../../../intern/clog
  ../../../intern/guardedalloc
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
  intern/USD_userdef_default_theme.c

  USD_api.h
  USD_area.h
  USD_color_types.h
  USD_context.h
  USD_curveprofile_types.h
  USD_customdata_types.h
  USD_default_tables.h
  USD_defs.h
  USD_factory.h
  USD_file.h
  USD_ID.h
  USD_ID_enums.h
  USD_listBase.h
  USD_object.h
  USD_operator.h
  USD_path_defaults.h
  USD_pixar_utils.h
  USD_region.h
  USD_scene.h
  USD_scene_types.h
  USD_screen.h
  USD_system.h
  USD_texture_types.h
  USD_types.h
  USD_userdef_types.h
  USD_userpref.h
  USD_wm_types.h
  USD_window.h
  USD_workspace.h
  USD_vec_types.h
  USD_view2d.h
  USD_view3d.h
)

set(LIB
  maelstrom
  maelstrom_static
  ${BOOST_LIBRARIES}
  ${TBB_LIBRARIES}
  ${PYTHON_LIBRARIES}
)

add_definitions(-DKRAKEN_USD_EXPORTS)

kraken_add_lib(kraken_universe "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")