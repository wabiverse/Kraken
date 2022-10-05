# kraken lib depends on eigen.
add_subdirectory(eigen)

set(SRC_USD_INC
  ${CMAKE_CURRENT_SOURCE_DIR}/universe/USD_ID.h
  ${CMAKE_CURRENT_SOURCE_DIR}/universe/USD_ID_enums.h
  ${CMAKE_CURRENT_SOURCE_DIR}/universe/USD_api.h
  ${CMAKE_CURRENT_SOURCE_DIR}/universe/USD_area.h
  ${CMAKE_CURRENT_SOURCE_DIR}/universe/USD_color_types.h
  ${CMAKE_CURRENT_SOURCE_DIR}/universe/USD_context.h
  ${CMAKE_CURRENT_SOURCE_DIR}/universe/USD_curveprofile_types.h
  ${CMAKE_CURRENT_SOURCE_DIR}/universe/USD_factory.h
  ${CMAKE_CURRENT_SOURCE_DIR}/universe/USD_file.h
  ${CMAKE_CURRENT_SOURCE_DIR}/universe/USD_listBase.h
  ${CMAKE_CURRENT_SOURCE_DIR}/universe/USD_object.h
  ${CMAKE_CURRENT_SOURCE_DIR}/universe/USD_operator.h
  ${CMAKE_CURRENT_SOURCE_DIR}/universe/USD_path_defaults.h
  ${CMAKE_CURRENT_SOURCE_DIR}/universe/USD_pixar_utils.h
  ${CMAKE_CURRENT_SOURCE_DIR}/universe/USD_region.h
  ${CMAKE_CURRENT_SOURCE_DIR}/universe/USD_scene.h
  ${CMAKE_CURRENT_SOURCE_DIR}/universe/USD_screen.h
  ${CMAKE_CURRENT_SOURCE_DIR}/universe/USD_space_types.h
  ${CMAKE_CURRENT_SOURCE_DIR}/universe/USD_system.h
  ${CMAKE_CURRENT_SOURCE_DIR}/universe/USD_texture_types.h
  ${CMAKE_CURRENT_SOURCE_DIR}/universe/USD_types.h
  ${CMAKE_CURRENT_SOURCE_DIR}/universe/USD_userpref.h
  ${CMAKE_CURRENT_SOURCE_DIR}/universe/USD_vec_types.h
  ${CMAKE_CURRENT_SOURCE_DIR}/universe/USD_window.h
  ${CMAKE_CURRENT_SOURCE_DIR}/universe/USD_wm_types.h
  ${CMAKE_CURRENT_SOURCE_DIR}/universe/USD_workspace.h
)

set(SRC_USD_DEFAULTS_INC
  ${CMAKE_CURRENT_SOURCE_DIR}/universe/USD_default_tables.h
)

add_subdirectory(datatoc)
add_subdirectory(KrakenOS)
add_subdirectory(anchor)
add_subdirectory(editors)
add_subdirectory(wm)
add_subdirectory(krakernel)
add_subdirectory(kraklib)
add_subdirectory(draw)
add_subdirectory(krakfont)
add_subdirectory(krakloader)
add_subdirectory(gpu)
add_subdirectory(imbuf)
add_subdirectory(universe)
add_subdirectory(luxo)
add_subdirectory(python)

# if(WITH_TESTS)
#   add_subdirectory(test)
# endif()