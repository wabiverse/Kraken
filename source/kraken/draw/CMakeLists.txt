set(INC
  .
  intern

  ../krakfont
  ../krakkernel
  ../kraklib
  ../editors/include
  ../editors/space_view3d
  ../gpu
  ../gpu/intern
  ../imbuf
  ../universe
  ../luxo
  ../wm

  ../../../intern/atomic
  ../../../intern/guardedalloc

  # dna_type_offsets.h
  ${CMAKE_CURRENT_BINARY_DIR}/../universe/intern
  ${OPENSUBDIV_INCLUDE_DIRS}
)

set(SRC
  engines/phoenix/phoenix_stub.cc

  DRW_engine.h
)

set(LIB
  kraken_kernel
  kraken_lib
  kraken_wm
)

set(GLSL_SRC "")

# set(GLSL_C)
# foreach(GLSL_FILE ${GLSL_SRC})
#   data_to_c_simple(${GLSL_FILE} GLSL_C)
# endforeach()

# kraken_add_lib(kraken_draw_shaders "${GLSL_C}" "" "" "")

# list(APPEND LIB
#   kraken_draw_shaders
# )

# set(GLSL_SOURCE_CONTENT "")
# foreach(GLSL_FILE ${GLSL_SRC})
#   get_filename_component(GLSL_FILE_NAME ${GLSL_FILE} NAME)
#   string(REPLACE "." "_" GLSL_FILE_NAME_UNDERSCORES ${GLSL_FILE_NAME})
#   string(APPEND GLSL_SOURCE_CONTENT "SHADER_SOURCE\(datatoc_${GLSL_FILE_NAME_UNDERSCORES}, \"${GLSL_FILE_NAME}\", \"${GLSL_FILE}\"\)\n")
# endforeach()

# set(glsl_source_list_file "${CMAKE_CURRENT_BINARY_DIR}/glsl_draw_source_list.h")
# file(GENERATE OUTPUT ${glsl_source_list_file} CONTENT "${GLSL_SOURCE_CONTENT}")
# list(APPEND SRC ${glsl_source_list_file})
# list(APPEND INC ${CMAKE_CURRENT_BINARY_DIR})

# target_include_directories(kraken_draw_shaders PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

list(APPEND INC)


kraken_add_lib(kraken_draw "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")

add_dependencies(kraken_draw kraken_universe)
