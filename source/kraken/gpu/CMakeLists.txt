set(INC
  .
  ../../../. # Pixar USD
  ../../../source
  metal
  ../krakernel
  ../kraklib
  ../draw
  ../universe
  ../luxo
  ../anchor
  ../editors/include
)

set(INC_SYS
  ${Boost_INCLUDE_DIRS}
  ${TBB_INCLUDE_DIRS}
  ${PYTHON_INCLUDE_DIR}
)

set(SRC "")

set(OPENGL_SRC "")

set(DX3D_SRC
  dx3d/dx3d_stub.cpp
)

set(METAL_SRC
  metal/mtl_context.cpp

  metal/mtl_context.h
)

if(WITH_DIRECTX)
  list(APPEND SRC ${DX3D_SRC})
endif()

if(WITH_METAL)
  list(APPEND SRC ${METAL_SRC})
endif()

set(LIB
  maelstrom
  maelstrom_static
  kraken_anchor
  kraken_lib
  kraken_kernel
  kraken_universe
  ${BOOST_LIBRARIES}
  ${TBB_LIBRARIES}
  ${PYTHON_LIBRARIES}
)

set(MSL_SRC "")

set(GLSL_SRC "")

# set(GLSL_C)
# foreach(GLSL_FILE ${GLSL_SRC})
#   data_to_c_simple(${GLSL_FILE} GLSL_C)
# endforeach()


# if(WITH_METAL_BACKEND)
#   set(MSL_C)
#   foreach(MSL_FILE ${MSL_SRC})
#     data_to_c_simple(${MSL_FILE} MSL_C)
#   endforeach()
#   list(APPEND GLSL_C ${MSL_C})
# endif()

# kraken_add_lib(kraken_gpu_shaders "${GLSL_C}" "" "" "")

# list(APPEND LIB
#   kraken_gpu_shaders
# )

# set(GLSL_SOURCE_CONTENT "")
# foreach(GLSL_FILE ${GLSL_SRC})
#   get_filename_component(GLSL_FILE_NAME ${GLSL_FILE} NAME)
#   string(REPLACE "." "_" GLSL_FILE_NAME_UNDERSCORES ${GLSL_FILE_NAME})
#   string(APPEND GLSL_SOURCE_CONTENT "SHADER_SOURCE\(datatoc_${GLSL_FILE_NAME_UNDERSCORES}, \"${GLSL_FILE_NAME}\", \"${GLSL_FILE}\"\)\n")
# endforeach()

# set(glsl_source_list_file "${CMAKE_CURRENT_BINARY_DIR}/glsl_gpu_source_list.h")
# file(GENERATE OUTPUT ${glsl_source_list_file} CONTENT "${GLSL_SOURCE_CONTENT}")
# list(APPEND SRC ${glsl_source_list_file})
# list(APPEND INC ${CMAKE_CURRENT_BINARY_DIR})

set(SRC_SHADER_CREATE_INFOS "")

# set(SHADER_CREATE_INFOS_CONTENT "")
# foreach(DESCRIPTOR_FILE ${SRC_SHADER_CREATE_INFOS})
#   string(APPEND SHADER_CREATE_INFOS_CONTENT "#include \"${DESCRIPTOR_FILE}\"\n")
# endforeach()

# set(shader_create_info_list_file "${CMAKE_CURRENT_BINARY_DIR}/gpu_shader_create_info_list.hh")
# file(GENERATE OUTPUT ${shader_create_info_list_file} CONTENT "${SHADER_CREATE_INFOS_CONTENT}")

if(WITH_OPENCOLORIO)
  add_definitions(-DWITH_OCIO)
endif()

kraken_add_lib(kraken_gpu "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")
# target_link_libraries(kraken_gpu PUBLIC
  # kraken_draw_shaders
  # kraken_gpu_shaders
# )

# if(WITH_OPENCOLORIO)
#   target_link_libraries(kraken_gpu PUBLIC kraken_ocio_shaders)
# endif()


# if(CXX_WARN_NO_SUGGEST_OVERRIDE)
#   target_compile_options(kraken_gpu PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-Wsuggest-override>)
# endif()



# if(WITH_GPU_BUILDTIME_SHADER_BUILDER)
  # if(APPLE)
  #   add_executable(shader_builder
  #     intern/gpu_shader_builder.cc
  #     ${shader_create_info_list_file}
  #   )

  #   setup_platform_linker_flags(shader_builder)
  #   target_link_libraries(shader_builder PUBLIC buildinfoobj)
  # else()
  #   if(WIN32)
  #     # We can re-use the manifest from tests.exe here since it's
  #     # rather generic and just selects the appropriate common
  #     # controls version.
  #     set(MANIFEST "${CMAKE_BINARY_DIR}/tests.exe.manifest")
  #   endif()
  #   add_executable(shader_builder
  #     intern/gpu_shader_builder.cc
  #     intern/gpu_shader_builder_stubs.cc
  #     ${shader_create_info_list_file}
  #     ${MANIFEST}
  #   )

  # endif()
  # target_link_libraries(shader_builder PUBLIC
  #   kraken_gpu
  #   kraken_kraklib
  #   kraken_anchor
  #   ${PLATFORM_LINKLIBS}
  # )
  # target_include_directories(shader_builder PRIVATE ${INC} ${CMAKE_CURRENT_BINARY_DIR})

  # set(SRC_BAKED_CREATE_INFOS_FILE ${CMAKE_CURRENT_BINARY_DIR}/shader_baked.hh)

  # add_custom_command(
  #   OUTPUT
  #   ${SRC_BAKED_CREATE_INFOS_FILE}
  #   COMMAND
  #     "$<TARGET_FILE:shader_builder>" ${SRC_BAKED_CREATE_INFOS_FILE}
  #   DEPENDS shader_builder
  # )
  # set(GPU_SHADER_INFO_SRC
  #   intern/gpu_shader_info_baked.cc
  #   ${SRC_BAKED_CREATE_INFOS_FILE}

  #   # For project files to be aware of these headers.
  #   ${SRC_SHADER_CREATE_INFOS}
  #   shaders/infos/gpu_interface_info.hh
  # )

  # kraken_add_lib(kraken_gpu_shader_infos "${GPU_SHADER_INFO_SRC}" "" "" "")
# endif()


