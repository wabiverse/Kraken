set(INC
  .
  ../../../. # Pixar USD
  ../../../source
  ./intern
  ./metal
  ../krakernel
  ../kraklib
  ../imbuf
  ../draw
  ../universe
  ../luxo
  ../anchor
  ../editors/include
  ../../../intern/atomic
  ../../../intern/guardedalloc
)

set(INC_SYS
  ${Boost_INCLUDE_DIRS}
  ${TBB_INCLUDE_DIRS}
  ${PYTHON_INCLUDE_DIR}
)

# Shared & Agnostic GPU API.
set(SRC
  # intern/gpu_batch.cc
  # intern/gpu_batch_presets.c
  # intern/gpu_batch_utils.c
  # intern/gpu_capabilities.cc
  # intern/gpu_codegen.cc
  # intern/gpu_compute.cc
  intern/gpu_context.cc
  intern/gpu_debug.cc
  # intern/gpu_drawlist.cc
  # intern/gpu_framebuffer.cc
  # intern/gpu_immediate.cc
  # intern/gpu_immediate_util.c
  intern/gpu_index_buffer.cc
  # intern/gpu_init_exit.c
  # intern/gpu_material.c
  intern/gpu_matrix.cc
  # intern/gpu_node_graph.c
  # intern/gpu_platform.cc
  # intern/gpu_query.cc
  # intern/gpu_select.c
  # intern/gpu_select_pick.c
  # intern/gpu_select_sample_query.cc
  intern/gpu_shader.cc
  # intern/gpu_shader_builtin.c
  # intern/gpu_shader_create_info.cc
  # intern/gpu_shader_dependency.cc
  # intern/gpu_shader_interface.cc
  intern/gpu_shader_log.cc
  intern/gpu_state.cc
  intern/gpu_storage_buffer.cc
  intern/gpu_texture.cc
  intern/gpu_uniform_buffer.cc
  intern/gpu_vertex_buffer.cc
  intern/gpu_vertex_format.cc
  intern/gpu_viewport.c

  GPU_batch.h
  # GPU_batch_presets.h
  # GPU_batch_utils.h
  GPU_capabilities.h
  GPU_common.h
  GPU_common_types.h
  # GPU_compute.h
  GPU_context.h
  GPU_debug.h
  # GPU_drawlist.h
  GPU_framebuffer.h
  GPU_immediate.h
  # GPU_immediate_util.h
  GPU_index_buffer.h
  # GPU_init_exit.h
  GPU_material.h
  GPU_matrix.h
  GPU_platform.h
  GPU_primitive.h
  # GPU_select.h
  GPU_shader.h
  # GPU_shader_shared.h
  GPU_state.h
  GPU_storage_buffer.h
  GPU_texture.h
  GPU_uniform_buffer.h
  GPU_vertex_buffer.h
  GPU_vertex_format.h
  GPU_viewport.h

  intern/gpu_backend.hh
  intern/gpu_batch_private.hh
  # intern/gpu_capabilities_private.hh
  # intern/gpu_codegen.h
  intern/gpu_context_private.hh
  intern/gpu_debug_private.hh
  # intern/gpu_drawlist_private.hh
  intern/gpu_framebuffer_private.hh
  # intern/gpu_immediate_private.hh
  intern/gpu_index_buffer_private.hh
  # intern/gpu_material_library.h
  intern/gpu_matrix_private.h
  # intern/gpu_node_graph.h
  # intern/gpu_platform_private.hh
  # intern/gpu_private.h
  # intern/gpu_query.hh
  # intern/gpu_select_private.h
  # intern/gpu_shader_create_info.hh
  # intern/gpu_shader_create_info_private.hh
  # intern/gpu_shader_dependency_private.h
  intern/gpu_shader_interface.hh
  intern/gpu_shader_private.hh
  intern/gpu_state_private.hh
  intern/gpu_storage_buffer_private.hh
  intern/gpu_texture_private.hh
  intern/gpu_uniform_buffer_private.hh
  intern/gpu_vertex_buffer_private.hh
  intern/gpu_vertex_format_private.h
)

# OpenGL Implementation.
set(OPENGL_SRC "")

# Vulkan Implementation.
set(VULKAN_SRC "")

# DirectX3D Implementation.
set(DX3D_SRC
  dx3d/dx3d_stub.cpp
)

# Metal Implementation.
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
  kraken_lib
  kraken_kernel
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


