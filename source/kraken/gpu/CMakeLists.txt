set(INC
  .
  intern
  metal
  ../../../. # Pixar USD
  ../../../source
  ../krakernel
  ../kraklib
  ../imbuf
  ../draw
  ../draw/intern
  ../universe
  ../luxo
  ../anchor
  ../editors/include
  ../../../intern/clog
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
  intern/gpu_batch_presets.c
  intern/gpu_batch.cc
  intern/gpu_capabilities.cc
  intern/gpu_context.cc
  intern/gpu_codegen.cc
  intern/gpu_debug.cc
  intern/gpu_framebuffer.cc
  intern/gpu_index_buffer.cc
  intern/gpu_immediate.cc
  intern/gpu_immediate_util.c
  intern/gpu_init_exit.c
  intern/gpu_material.c
  intern/gpu_matrix.cc
  intern/gpu_node_graph.c
  intern/gpu_platform.cc
  intern/gpu_shader.cc
  intern/gpu_shader_builtin.c
  intern/gpu_shader_create_info.cc
  intern/gpu_shader_dependency.cc
  intern/gpu_shader_log.cc
  intern/gpu_state.cc
  intern/gpu_storage_buffer.cc
  intern/gpu_texture.cc
  intern/gpu_uniform_buffer.cc
  intern/gpu_vertex_buffer.cc
  intern/gpu_vertex_format.cc
  intern/gpu_viewport.c

  GPU_batch_presets.h
  GPU_batch.h
  GPU_capabilities.h
  GPU_common.h
  GPU_common_types.h
  GPU_context.h
  GPU_debug.h
  GPU_framebuffer.h
  GPU_immediate.h
  GPU_immediate_util.h
  GPU_index_buffer.h
  GPU_init_exit.h
  GPU_material.h
  GPU_matrix.h
  GPU_platform.h
  GPU_primitive.h
  GPU_shader.h
  GPU_shader_shared.h
  GPU_shader_shared_utils.h
  GPU_state.h
  GPU_storage_buffer.h
  GPU_texture.h
  GPU_uniform_buffer.h
  GPU_vertex_buffer.h
  GPU_vertex_format.h
  GPU_viewport.h

  intern/gpu_backend.hh
  intern/gpu_batch_private.hh
  intern/gpu_capabilities_private.hh
  intern/gpu_codegen.h
  intern/gpu_context_private.hh
  intern/gpu_debug_private.hh
  intern/gpu_framebuffer_private.hh
  intern/gpu_immediate_private.hh
  intern/gpu_index_buffer_private.hh
  intern/gpu_material_library.h
  intern/gpu_matrix_private.h
  intern/gpu_node_graph.h
  intern/gpu_platform_private.hh
  intern/gpu_private.h
  intern/gpu_shader_create_info.hh
  intern/gpu_shader_create_info_private.hh
  intern/gpu_shader_dependency_private.h
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
  metal/mtl_backend.cc
  metal/mtl_context.cc
  metal/mtl_texture.cc
  metal/mtl_texture_util.cc

  metal/mtl_backend.hh
  metal/mtl_capabilities.hh
  metal/mtl_common.hh
  metal/mtl_context.hh
  metal/mtl_debug.hh
  metal/mtl_framebuffer.hh
  metal/mtl_immediate.hh
  metal/mtl_memory.hh
  metal/mtl_pso_descriptor_state.hh
  metal/mtl_shader.hh
  metal/mtl_shader_interface.hh
  metal/mtl_shader_interface_type.hh
  metal/mtl_shader_shared.h
  metal/mtl_state.hh
  metal/mtl_texture.hh
  metal/mtl_vertex_buffer.hh
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
  ${ALL_IMAGING_LIBRARIES}
)

set(MSL_SRC
  shaders/metal/mtl_shader_defines.msl
  shaders/metal/mtl_shader_common.msl

  metal/kernels/compute_texture_update.msl
  metal/kernels/compute_texture_read.msl
  metal/kernels/depth_2d_update_float_frag.glsl
  metal/kernels/depth_2d_update_int24_frag.glsl
  metal/kernels/depth_2d_update_int32_frag.glsl
  metal/kernels/depth_2d_update_vert.glsl
  metal/kernels/gpu_shader_fullscreen_blit_vert.glsl
  metal/kernels/gpu_shader_fullscreen_blit_frag.glsl
)

set(GLSL_SRC 
  GPU_shader_shared.h

  shaders/gpu_shader_depth_only_frag.glsl
  shaders/gpu_shader_uniform_color_frag.glsl
  shaders/gpu_shader_checker_frag.glsl
  shaders/gpu_shader_diag_stripes_frag.glsl
  shaders/gpu_shader_simple_lighting_frag.glsl
  shaders/gpu_shader_flat_color_frag.glsl
  shaders/gpu_shader_flat_color_alpha_test_0_frag.glsl
  shaders/gpu_shader_flat_id_frag.glsl
  shaders/gpu_shader_2D_vert.glsl
  shaders/gpu_shader_2D_area_borders_vert.glsl
  shaders/gpu_shader_2D_area_borders_frag.glsl
  shaders/gpu_shader_2D_widget_base_vert.glsl
  shaders/gpu_shader_2D_widget_base_frag.glsl
  shaders/gpu_shader_2D_widget_shadow_vert.glsl
  shaders/gpu_shader_2D_widget_shadow_frag.glsl
  shaders/gpu_shader_2D_nodelink_frag.glsl
  shaders/gpu_shader_2D_nodelink_vert.glsl
  shaders/gpu_shader_2D_line_dashed_frag.glsl
  shaders/gpu_shader_2D_image_vert.glsl
  shaders/gpu_shader_2D_image_rect_vert.glsl
  shaders/gpu_shader_2D_image_multi_rect_vert.glsl
  shaders/gpu_shader_image_frag.glsl
  shaders/gpu_shader_image_desaturate_frag.glsl
  shaders/gpu_shader_image_overlays_merge_frag.glsl
  shaders/gpu_shader_image_overlays_stereo_merge_frag.glsl
  shaders/gpu_shader_image_shuffle_color_frag.glsl
  shaders/gpu_shader_image_color_frag.glsl
  shaders/gpu_shader_image_varying_color_frag.glsl
  shaders/gpu_shader_3D_image_vert.glsl
  shaders/gpu_shader_3D_vert.glsl
  shaders/gpu_shader_3D_normal_vert.glsl
  shaders/gpu_shader_3D_flat_color_vert.glsl
  shaders/gpu_shader_3D_line_dashed_uniform_color_vert.glsl
  shaders/gpu_shader_3D_polyline_frag.glsl
  shaders/gpu_shader_3D_polyline_geom.glsl
  shaders/gpu_shader_3D_polyline_vert.glsl
  shaders/gpu_shader_3D_polyline_vert_no_geom.glsl
  shaders/gpu_shader_3D_smooth_color_vert.glsl
  shaders/gpu_shader_3D_smooth_color_frag.glsl
  shaders/gpu_shader_3D_passthrough_vert.glsl
  shaders/gpu_shader_3D_clipped_uniform_color_vert.glsl

  shaders/gpu_shader_instance_variying_size_variying_color_vert.glsl

  shaders/gpu_shader_point_uniform_color_aa_frag.glsl
  shaders/gpu_shader_point_uniform_color_outline_aa_frag.glsl
  shaders/gpu_shader_point_varying_color_varying_outline_aa_frag.glsl
  shaders/gpu_shader_point_varying_color_frag.glsl
  shaders/gpu_shader_3D_point_fixed_size_varying_color_vert.glsl
  shaders/gpu_shader_3D_point_varying_size_varying_color_vert.glsl
  shaders/gpu_shader_3D_point_uniform_size_aa_vert.glsl
  shaders/gpu_shader_2D_point_varying_size_varying_color_vert.glsl
  shaders/gpu_shader_2D_point_uniform_size_aa_vert.glsl
  shaders/gpu_shader_2D_point_uniform_size_outline_aa_vert.glsl

  shaders/gpu_shader_text_vert.glsl
  shaders/gpu_shader_text_frag.glsl
  shaders/gpu_shader_keyframe_shape_vert.glsl
  shaders/gpu_shader_keyframe_shape_frag.glsl

  shaders/gpu_shader_codegen_lib.glsl

  shaders/gpu_shader_gpencil_stroke_vert.glsl
  shaders/gpu_shader_gpencil_stroke_frag.glsl
  shaders/gpu_shader_gpencil_stroke_geom.glsl

  shaders/gpu_shader_cfg_world_clip_lib.glsl
  shaders/gpu_shader_colorspace_lib.glsl

  GPU_shader_shared_utils.h
)

set(MTL_BACKEND_GLSL_SRC
  metal/kernels/compute_texture_update.msl
  metal/kernels/compute_texture_read.msl
  metal/kernels/depth_2d_update_float_frag.glsl
  metal/kernels/depth_2d_update_int24_frag.glsl
  metal/kernels/depth_2d_update_int32_frag.glsl
  metal/kernels/depth_2d_update_vert.glsl
  metal/kernels/gpu_shader_fullscreen_blit_vert.glsl
  metal/kernels/gpu_shader_fullscreen_blit_frag.glsl
)

set(MSL_SRC
  shaders/metal/mtl_shader_defines.msl
  shaders/metal/mtl_shader_common.msl
  metal/mtl_shader_shared.h
)

if(WITH_METAL)
  list(APPEND GLSL_SRC ${MTL_BACKEND_GLSL_SRC})

  set(MSL_C)
  foreach(MSL_FILE ${MSL_SRC})
    data_to_c_simple(${MSL_FILE} MSL_C)
  endforeach()
endif()

set(GLSL_C)
foreach(GLSL_FILE ${GLSL_SRC})
  data_to_c_simple(${GLSL_FILE} GLSL_C)
endforeach()

set(SHADER_C)
list(APPEND SHADER_C ${GLSL_C})
if(WITH_METAL)
  list(APPEND SHADER_C ${MSL_C})
endif()

kraken_add_lib(kraken_gpu_shaders "${SHADER_C}" "" "" "")

list(APPEND LIB
  kraken_gpu_shaders
)

set(GLSL_SOURCE_CONTENT "")
foreach(GLSL_FILE ${GLSL_SRC})
  get_filename_component(GLSL_FILE_NAME ${GLSL_FILE} NAME)
  string(REPLACE "." "_" GLSL_FILE_NAME_UNDERSCORES ${GLSL_FILE_NAME})
  string(APPEND GLSL_SOURCE_CONTENT "SHADER_SOURCE\(datatoc_${GLSL_FILE_NAME_UNDERSCORES}, \"${GLSL_FILE_NAME}\", \"${GLSL_FILE}\"\)\n")
endforeach()

set(glsl_source_list_file "${CMAKE_CURRENT_BINARY_DIR}/glsl_gpu_source_list.h")
file(GENERATE OUTPUT ${glsl_source_list_file} CONTENT "${GLSL_SOURCE_CONTENT}")
list(APPEND SRC ${glsl_source_list_file})
list(APPEND INC ${CMAKE_CURRENT_BINARY_DIR})

set(SRC_SHADER_CREATE_INFOS
  # Basic Engine Shaders...
  ../draw/engines/basic/shaders/infos/basic_depth_info.hh

  # Overlay Engine Shaders...
  ../draw/engines/overlay/shaders/infos/overlay_antialiasing_info.hh
  ../draw/engines/overlay/shaders/infos/overlay_armature_info.hh
  ../draw/engines/overlay/shaders/infos/overlay_viewer_attribute_info.hh
  ../draw/engines/overlay/shaders/infos/overlay_background_info.hh
  ../draw/engines/overlay/shaders/infos/overlay_edit_mode_info.hh
  ../draw/engines/overlay/shaders/infos/overlay_extra_info.hh
  ../draw/engines/overlay/shaders/infos/overlay_facing_info.hh
  ../draw/engines/overlay/shaders/infos/overlay_grid_info.hh
  ../draw/engines/overlay/shaders/infos/overlay_outline_info.hh
  ../draw/engines/overlay/shaders/infos/overlay_paint_info.hh
  ../draw/engines/overlay/shaders/infos/overlay_sculpt_curves_info.hh
  ../draw/engines/overlay/shaders/infos/overlay_sculpt_info.hh
  ../draw/engines/overlay/shaders/infos/overlay_volume_info.hh
  ../draw/engines/overlay/shaders/infos/overlay_wireframe_info.hh

  # Shaders shared by all engines...
  ../draw/intern/shaders/draw_debug_info.hh
  ../draw/intern/shaders/draw_fullscreen_info.hh
  ../draw/intern/shaders/draw_hair_refine_info.hh
  ../draw/intern/shaders/draw_object_infos_info.hh
  ../draw/intern/shaders/draw_view_info.hh

  shaders/infos/gpu_clip_planes_info.hh
  shaders/infos/gpu_shader_2D_area_borders_info.hh
  shaders/infos/gpu_shader_2D_checker_info.hh
  shaders/infos/gpu_shader_2D_diag_stripes_info.hh
  shaders/infos/gpu_shader_2D_image_desaturate_color_info.hh
  shaders/infos/gpu_shader_2D_image_info.hh
  shaders/infos/gpu_shader_2D_image_multi_rect_color_info.hh
  shaders/infos/gpu_shader_2D_image_overlays_merge_info.hh
  shaders/infos/gpu_shader_2D_image_overlays_stereo_merge_info.hh
  shaders/infos/gpu_shader_2D_image_rect_color_info.hh
  shaders/infos/gpu_shader_2D_image_shuffle_color_info.hh
  shaders/infos/gpu_shader_2D_nodelink_info.hh
  shaders/infos/gpu_shader_2D_point_uniform_size_uniform_color_aa_info.hh
  shaders/infos/gpu_shader_2D_point_uniform_size_uniform_color_outline_aa_info.hh
  shaders/infos/gpu_shader_2D_point_varying_size_varying_color_info.hh
  shaders/infos/gpu_shader_2D_widget_info.hh
  shaders/infos/gpu_shader_3D_depth_only_info.hh
  shaders/infos/gpu_shader_3D_flat_color_info.hh
  shaders/infos/gpu_shader_3D_image_info.hh
  shaders/infos/gpu_shader_3D_point_info.hh
  shaders/infos/gpu_shader_3D_polyline_info.hh
  shaders/infos/gpu_shader_3D_smooth_color_info.hh
  shaders/infos/gpu_shader_3D_uniform_color_info.hh
  shaders/infos/gpu_shader_gpencil_stroke_info.hh
  shaders/infos/gpu_shader_instance_varying_color_varying_size_info.hh
  shaders/infos/gpu_shader_keyframe_shape_info.hh
  shaders/infos/gpu_shader_line_dashed_uniform_color_info.hh
  shaders/infos/gpu_shader_simple_lighting_info.hh
  shaders/infos/gpu_shader_text_info.hh
  shaders/infos/gpu_srgb_to_framebuffer_space_info.hh
)

set(SRC_SHADER_CREATE_INFOS_MTL
  metal/kernels/depth_2d_update_info.hh
  metal/kernels/gpu_shader_fullscreen_blit_info.hh
)

if(WITH_METAL)
  list(APPEND SRC_SHADER_CREATE_INFOS ${SRC_SHADER_CREATE_INFOS_MTL})
endif()

set(SHADER_CREATE_INFOS_CONTENT "")
foreach(DESCRIPTOR_FILE ${SRC_SHADER_CREATE_INFOS})
  string(APPEND SHADER_CREATE_INFOS_CONTENT "#include \"${DESCRIPTOR_FILE}\"\n")
endforeach()

set(shader_create_info_list_file "${CMAKE_CURRENT_BINARY_DIR}/gpu_shader_create_info_list.hh")
file(GENERATE OUTPUT ${shader_create_info_list_file} CONTENT "${SHADER_CREATE_INFOS_CONTENT}")

if(WITH_OPENCOLORIO)
  add_definitions(-DWITH_OCIO)
endif()

kraken_add_lib(kraken_gpu "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")
target_link_libraries(kraken_gpu PUBLIC
  kraken_draw_shaders
  kraken_gpu_shaders
)

if(WITH_OPENCOLORIO)
  target_link_libraries(kraken_gpu PUBLIC kraken_ocio_shaders)
endif()

if(CXX_WARN_NO_SUGGEST_OVERRIDE)
  target_compile_options(kraken_gpu PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-Wsuggest-override>)
endif()

if(WITH_GPU_BUILDTIME_SHADER_BUILDER)
  if(APPLE)
    add_executable(shader_builder
      intern/gpu_shader_builder.cc
      intern/gpu_shader_builder_stubs.cc
      ${shader_create_info_list_file}
    )

    setup_platform_linker_flags(shader_builder)
    target_link_libraries(shader_builder PUBLIC buildinfoobj)
  else()
    if(WIN32)
      # We can re-use the manifest from tests.exe here since it's
      # rather generic and just selects the appropriate common
      # controls version.
      set(MANIFEST "${CMAKE_BINARY_DIR}/tests.exe.manifest")
    endif()

    add_executable(shader_builder
      intern/gpu_shader_builder.cc
      intern/gpu_shader_builder_stubs.cc
      ${shader_create_info_list_file}
      ${MANIFEST}
    )
  endif()

  target_link_libraries(shader_builder PUBLIC
    kraken_gpu
    kraken_intern_clog
    kraken_lib
    kraken_anchor
    ${PLATFORM_LINKLIBS}
  )
  target_include_directories(shader_builder PRIVATE ${INC} ${CMAKE_CURRENT_BINARY_DIR})

  set(SRC_BAKED_CREATE_INFOS_FILE ${CMAKE_CURRENT_BINARY_DIR}/shader_baked.hh)

  add_custom_command(
    OUTPUT
    ${SRC_BAKED_CREATE_INFOS_FILE}
    COMMAND
    "$<TARGET_FILE:shader_builder>" ${SRC_BAKED_CREATE_INFOS_FILE}
    DEPENDS shader_builder
  )
  set(GPU_SHADER_INFO_SRC
    intern/gpu_shader_info_baked.cc
    ${SRC_BAKED_CREATE_INFOS_FILE}

    # For project files to be aware of these headers.
    ${SRC_SHADER_CREATE_INFOS}
    shaders/infos/gpu_interface_info.hh
  )

  kraken_add_lib(kraken_gpu_shader_infos "${GPU_SHADER_INFO_SRC}" "" "" "")
endif()
