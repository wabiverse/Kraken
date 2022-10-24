set(INC
  .
  intern

  ../../../. # Pixar USD
  ../../../source
  ../editors/include
  ../editors/space_view3d
  ../gpu
  ../gpu/intern
  ../imbuf
  ../krakernel
  ../krakfont
  ../kraklib
  ../luxo
  ../render
  ../render/intern
  ../universe
  ../wm

  ../../../intern/atomic
  ../../../intern/clog
  ../../../intern/guardedalloc
  # ../../../intern/opensubdiv

  # dna_type_offsets.h
  ${CMAKE_CURRENT_BINARY_DIR}/../universe/intern
  ${OPENSUBDIV_INCLUDE_DIRS}
)

set(SRC
  intern/draw_manager.c

  engines/phoenix/phoenix_engine.cc

  DRW_engine.h
  intern/DRW_render.h
  intern/draw_common_shader_shared.h
  intern/draw_manager.h
  intern/draw_shader_shared.h
  intern/draw_state.h
  engines/phoenix/phoenix_api.h
  engines/phoenix/phoenix_engine.h
  engines/phoenix/phoenix_private.h
)

set(LIB
  kraken_kernel
  kraken_lib
  kraken_wm
)

set(GLSL_SRC
  # phoenix render engine.
  engines/phoenix/shaders/ambient_occlusion_lib.glsl
  engines/phoenix/shaders/background_vert.glsl
  engines/phoenix/shaders/common_uniforms_lib.glsl
  engines/phoenix/shaders/common_utiltex_lib.glsl
  engines/phoenix/shaders/lights_lib.glsl
  engines/phoenix/shaders/lightprobe_lib.glsl
  engines/phoenix/shaders/lightprobe_filter_glossy_frag.glsl
  engines/phoenix/shaders/lightprobe_filter_diffuse_frag.glsl
  engines/phoenix/shaders/lightprobe_filter_visibility_frag.glsl
  engines/phoenix/shaders/lightprobe_geom.glsl
  engines/phoenix/shaders/lightprobe_vert.glsl
  engines/phoenix/shaders/lightprobe_cube_display_frag.glsl
  engines/phoenix/shaders/lightprobe_cube_display_vert.glsl
  engines/phoenix/shaders/lightprobe_grid_display_frag.glsl
  engines/phoenix/shaders/lightprobe_grid_display_vert.glsl
  engines/phoenix/shaders/lightprobe_grid_fill_frag.glsl
  engines/phoenix/shaders/lightprobe_planar_display_frag.glsl
  engines/phoenix/shaders/lightprobe_planar_display_vert.glsl
  engines/phoenix/shaders/lookdev_world_frag.glsl
  engines/phoenix/shaders/closure_eval_lib.glsl
  engines/phoenix/shaders/closure_eval_diffuse_lib.glsl
  engines/phoenix/shaders/closure_eval_glossy_lib.glsl
  engines/phoenix/shaders/closure_eval_surface_lib.glsl
  engines/phoenix/shaders/closure_eval_refraction_lib.glsl
  engines/phoenix/shaders/closure_eval_volume_lib.glsl
  engines/phoenix/shaders/closure_eval_translucent_lib.glsl
  engines/phoenix/shaders/closure_type_lib.glsl
  engines/phoenix/shaders/phoenix_empty.glsl
  engines/phoenix/shaders/phoenix_empty_volume.glsl
  engines/phoenix/shaders/effect_bloom_frag.glsl
  engines/phoenix/shaders/effect_dof_bokeh_frag.glsl
  engines/phoenix/shaders/effect_dof_dilate_tiles_frag.glsl
  engines/phoenix/shaders/effect_dof_downsample_frag.glsl
  engines/phoenix/shaders/effect_dof_filter_frag.glsl
  engines/phoenix/shaders/effect_dof_flatten_tiles_frag.glsl
  engines/phoenix/shaders/effect_dof_gather_frag.glsl
  engines/phoenix/shaders/effect_dof_lib.glsl
  engines/phoenix/shaders/effect_dof_reduce_frag.glsl
  engines/phoenix/shaders/effect_dof_resolve_frag.glsl
  engines/phoenix/shaders/effect_dof_scatter_frag.glsl
  engines/phoenix/shaders/effect_dof_scatter_vert.glsl
  engines/phoenix/shaders/effect_dof_setup_frag.glsl
  engines/phoenix/shaders/effect_reflection_lib.glsl
  engines/phoenix/shaders/effect_reflection_resolve_frag.glsl
  engines/phoenix/shaders/effect_reflection_trace_frag.glsl
  engines/phoenix/shaders/effect_downsample_frag.glsl
  engines/phoenix/shaders/effect_downsample_cube_frag.glsl
  engines/phoenix/shaders/effect_gtao_frag.glsl
  engines/phoenix/shaders/effect_velocity_resolve_frag.glsl
  engines/phoenix/shaders/effect_velocity_tile_frag.glsl
  engines/phoenix/shaders/effect_minmaxz_frag.glsl
  engines/phoenix/shaders/effect_mist_frag.glsl
  engines/phoenix/shaders/effect_motion_blur_frag.glsl
  engines/phoenix/shaders/effect_subsurface_frag.glsl
  engines/phoenix/shaders/effect_translucency_frag.glsl
  engines/phoenix/shaders/effect_temporal_aa.glsl
  engines/phoenix/shaders/lightprobe_planar_downsample_frag.glsl
  engines/phoenix/shaders/lightprobe_planar_downsample_geom.glsl
  engines/phoenix/shaders/lightprobe_planar_downsample_vert.glsl
  engines/phoenix/shaders/object_motion_frag.glsl
  engines/phoenix/shaders/object_motion_vert.glsl
  engines/phoenix/shaders/prepass_frag.glsl
  engines/phoenix/shaders/shadow_accum_frag.glsl
  engines/phoenix/shaders/shadow_frag.glsl
  engines/phoenix/shaders/shadow_vert.glsl
  engines/phoenix/shaders/bsdf_lut_frag.glsl
  engines/phoenix/shaders/btdf_lut_frag.glsl
  engines/phoenix/shaders/bsdf_common_lib.glsl
  engines/phoenix/shaders/irradiance_lib.glsl
  engines/phoenix/shaders/octahedron_lib.glsl
  engines/phoenix/shaders/cubemap_lib.glsl
  engines/phoenix/shaders/bsdf_sampling_lib.glsl
  engines/phoenix/shaders/random_lib.glsl
  engines/phoenix/shaders/raytrace_lib.glsl
  engines/phoenix/shaders/renderpass_lib.glsl
  engines/phoenix/shaders/renderpass_postprocess_frag.glsl
  engines/phoenix/shaders/cryptomatte_lib.glsl
  engines/phoenix/shaders/cryptomatte_frag.glsl
  engines/phoenix/shaders/cryptomatte_vert.glsl
  engines/phoenix/shaders/ltc_lib.glsl
  engines/phoenix/shaders/ssr_lib.glsl
  engines/phoenix/shaders/surface_frag.glsl
  engines/phoenix/shaders/surface_geom.glsl
  engines/phoenix/shaders/surface_lib.glsl
  engines/phoenix/shaders/surface_vert.glsl
  engines/phoenix/shaders/update_noise_frag.glsl
  engines/phoenix/shaders/volumetric_accum_frag.glsl
  engines/phoenix/shaders/volumetric_lib.glsl
  engines/phoenix/shaders/volumetric_frag.glsl
  engines/phoenix/shaders/volumetric_geom.glsl
  engines/phoenix/shaders/volumetric_vert.glsl
  engines/phoenix/shaders/volumetric_resolve_frag.glsl
  engines/phoenix/shaders/volumetric_scatter_frag.glsl
  engines/phoenix/shaders/volumetric_integration_frag.glsl
  engines/phoenix/shaders/world_vert.glsl

  intern/shaders/common_aabb_lib.glsl
  intern/shaders/common_attribute_lib.glsl
  intern/shaders/common_colormanagement_lib.glsl
  intern/shaders/common_debug_draw_lib.glsl
  intern/shaders/common_debug_print_lib.glsl
  intern/shaders/common_debug_shape_lib.glsl
  intern/shaders/common_fullscreen_vert.glsl
  intern/shaders/common_fxaa_lib.glsl
  intern/shaders/common_gpencil_lib.glsl
  intern/shaders/common_hair_lib.glsl
  intern/shaders/common_hair_refine_comp.glsl
  intern/shaders/common_hair_refine_vert.glsl
  intern/shaders/common_intersect_lib.glsl
  intern/shaders/common_math_geom_lib.glsl
  intern/shaders/common_math_lib.glsl
  intern/shaders/common_pointcloud_lib.glsl
  intern/shaders/common_shape_lib.glsl
  intern/shaders/common_smaa_lib.glsl
  intern/shaders/common_subdiv_custom_data_interp_comp.glsl
  intern/shaders/common_subdiv_ibo_lines_comp.glsl
  intern/shaders/common_subdiv_ibo_tris_comp.glsl
  intern/shaders/common_subdiv_lib.glsl
  intern/shaders/common_subdiv_normals_accumulate_comp.glsl
  intern/shaders/common_subdiv_normals_finalize_comp.glsl
  intern/shaders/common_subdiv_patch_evaluation_comp.glsl
  intern/shaders/common_subdiv_vbo_edge_fac_comp.glsl
  intern/shaders/common_subdiv_vbo_edituv_strech_angle_comp.glsl
  intern/shaders/common_subdiv_vbo_edituv_strech_area_comp.glsl
  intern/shaders/common_subdiv_vbo_lnor_comp.glsl
  intern/shaders/common_subdiv_vbo_sculpt_data_comp.glsl
  intern/shaders/common_view_clipping_lib.glsl
  intern/shaders/common_view_lib.glsl
  intern/shaders/draw_command_generate_comp.glsl
  intern/shaders/draw_debug_draw_display_frag.glsl
  intern/shaders/draw_debug_draw_display_vert.glsl
  intern/shaders/draw_debug_info.hh
  intern/shaders/draw_debug_print_display_frag.glsl
  intern/shaders/draw_debug_print_display_vert.glsl
  intern/shaders/draw_resource_finalize_comp.glsl
  intern/shaders/draw_visibility_comp.glsl

  intern/draw_command_shared.hh
  intern/draw_common_shader_shared.h
  intern/draw_defines.h
  intern/draw_shader_shared.h

  # basic engine.
  engines/basic/shaders/basic_conservative_depth_geom.glsl
  engines/basic/shaders/basic_depth_vert.glsl
  engines/basic/shaders/basic_depth_curves_vert.glsl
  engines/basic/shaders/basic_depth_pointcloud_vert.glsl
  engines/basic/shaders/basic_depth_frag.glsl

  # overlay engine.
  engines/overlay/shaders/overlay_antialiasing_frag.glsl
  engines/overlay/shaders/overlay_armature_dof_solid_frag.glsl
  engines/overlay/shaders/overlay_armature_dof_vert.glsl
  engines/overlay/shaders/overlay_armature_envelope_outline_vert.glsl
  engines/overlay/shaders/overlay_armature_envelope_solid_frag.glsl
  engines/overlay/shaders/overlay_armature_envelope_solid_vert.glsl
  engines/overlay/shaders/overlay_armature_shape_outline_geom.glsl
  engines/overlay/shaders/overlay_armature_shape_outline_vert.glsl
  engines/overlay/shaders/overlay_armature_shape_outline_vert_no_geom.glsl
  engines/overlay/shaders/overlay_armature_shape_solid_frag.glsl
  engines/overlay/shaders/overlay_armature_shape_solid_vert.glsl
  engines/overlay/shaders/overlay_armature_shape_wire_vert.glsl
  engines/overlay/shaders/overlay_armature_sphere_outline_vert.glsl
  engines/overlay/shaders/overlay_armature_sphere_solid_frag.glsl
  engines/overlay/shaders/overlay_armature_sphere_solid_vert.glsl
  engines/overlay/shaders/overlay_armature_stick_frag.glsl
  engines/overlay/shaders/overlay_armature_stick_vert.glsl
  engines/overlay/shaders/overlay_armature_wire_frag.glsl
  engines/overlay/shaders/overlay_armature_wire_vert.glsl
  engines/overlay/shaders/overlay_background_frag.glsl
  engines/overlay/shaders/overlay_clipbound_vert.glsl
  engines/overlay/shaders/overlay_common_lib.glsl
  engines/overlay/shaders/overlay_depth_only_frag.glsl
  engines/overlay/shaders/overlay_depth_only_vert.glsl
  engines/overlay/shaders/overlay_edit_curve_handle_geom.glsl
  engines/overlay/shaders/overlay_edit_curve_handle_vert.glsl
  engines/overlay/shaders/overlay_edit_curve_handle_vert_no_geom.glsl
  engines/overlay/shaders/overlay_edit_curve_point_vert.glsl
  engines/overlay/shaders/overlay_edit_curve_wire_vert.glsl
  engines/overlay/shaders/overlay_edit_gpencil_canvas_vert.glsl
  engines/overlay/shaders/overlay_edit_gpencil_guide_vert.glsl
  engines/overlay/shaders/overlay_edit_gpencil_vert.glsl
  engines/overlay/shaders/overlay_edit_lattice_point_vert.glsl
  engines/overlay/shaders/overlay_edit_lattice_wire_vert.glsl
  engines/overlay/shaders/overlay_edit_mesh_analysis_frag.glsl
  engines/overlay/shaders/overlay_edit_mesh_analysis_vert.glsl
  engines/overlay/shaders/overlay_edit_mesh_common_lib.glsl
  engines/overlay/shaders/overlay_edit_mesh_frag.glsl
  engines/overlay/shaders/overlay_edit_mesh_geom.glsl
  engines/overlay/shaders/overlay_edit_mesh_normal_vert.glsl
  engines/overlay/shaders/overlay_edit_mesh_skin_root_vert.glsl
  engines/overlay/shaders/overlay_edit_mesh_vert.glsl
  engines/overlay/shaders/overlay_edit_mesh_vert_no_geom.glsl
  engines/overlay/shaders/overlay_edit_particle_point_vert.glsl
  engines/overlay/shaders/overlay_edit_particle_strand_vert.glsl
  engines/overlay/shaders/overlay_edit_uv_edges_frag.glsl
  engines/overlay/shaders/overlay_edit_uv_edges_geom.glsl
  engines/overlay/shaders/overlay_edit_uv_edges_vert.glsl
  engines/overlay/shaders/overlay_edit_uv_face_dots_vert.glsl
  engines/overlay/shaders/overlay_edit_uv_faces_vert.glsl
  engines/overlay/shaders/overlay_edit_uv_image_mask_frag.glsl
  engines/overlay/shaders/overlay_edit_uv_image_vert.glsl
  engines/overlay/shaders/overlay_edit_uv_stretching_vert.glsl
  engines/overlay/shaders/overlay_edit_uv_tiled_image_borders_vert.glsl
  engines/overlay/shaders/overlay_edit_uv_verts_frag.glsl
  engines/overlay/shaders/overlay_edit_uv_verts_vert.glsl
  engines/overlay/shaders/overlay_extra_frag.glsl
  engines/overlay/shaders/overlay_extra_groundline_vert.glsl
  engines/overlay/shaders/overlay_extra_lightprobe_grid_vert.glsl
  engines/overlay/shaders/overlay_extra_loose_point_frag.glsl
  engines/overlay/shaders/overlay_extra_loose_point_vert.glsl
  engines/overlay/shaders/overlay_extra_point_vert.glsl
  engines/overlay/shaders/overlay_extra_vert.glsl
  engines/overlay/shaders/overlay_extra_wire_frag.glsl
  engines/overlay/shaders/overlay_extra_wire_vert.glsl
  engines/overlay/shaders/overlay_facing_frag.glsl
  engines/overlay/shaders/overlay_facing_vert.glsl
  engines/overlay/shaders/overlay_grid_background_frag.glsl
  engines/overlay/shaders/overlay_grid_frag.glsl
  engines/overlay/shaders/overlay_grid_vert.glsl
  engines/overlay/shaders/overlay_image_frag.glsl
  engines/overlay/shaders/overlay_image_vert.glsl
  engines/overlay/shaders/overlay_motion_path_line_frag.glsl
  engines/overlay/shaders/overlay_motion_path_line_geom.glsl
  engines/overlay/shaders/overlay_motion_path_line_vert.glsl
  engines/overlay/shaders/overlay_motion_path_line_vert_no_geom.glsl
  engines/overlay/shaders/overlay_motion_path_point_vert.glsl
  engines/overlay/shaders/overlay_outline_detect_frag.glsl
  engines/overlay/shaders/overlay_outline_prepass_curves_vert.glsl
  engines/overlay/shaders/overlay_outline_prepass_frag.glsl
  engines/overlay/shaders/overlay_outline_prepass_geom.glsl
  engines/overlay/shaders/overlay_outline_prepass_gpencil_frag.glsl
  engines/overlay/shaders/overlay_outline_prepass_gpencil_vert.glsl
  engines/overlay/shaders/overlay_outline_prepass_pointcloud_vert.glsl
  engines/overlay/shaders/overlay_outline_prepass_vert.glsl
  engines/overlay/shaders/overlay_paint_face_vert.glsl
  engines/overlay/shaders/overlay_paint_point_vert.glsl
  engines/overlay/shaders/overlay_paint_texture_frag.glsl
  engines/overlay/shaders/overlay_paint_texture_vert.glsl
  engines/overlay/shaders/overlay_paint_vertcol_frag.glsl
  engines/overlay/shaders/overlay_paint_vertcol_vert.glsl
  engines/overlay/shaders/overlay_paint_weight_frag.glsl
  engines/overlay/shaders/overlay_paint_weight_vert.glsl
  engines/overlay/shaders/overlay_paint_wire_vert.glsl
  engines/overlay/shaders/overlay_particle_frag.glsl
  engines/overlay/shaders/overlay_particle_vert.glsl
  engines/overlay/shaders/overlay_point_varying_color_frag.glsl
  engines/overlay/shaders/overlay_point_varying_color_varying_outline_aa_frag.glsl
  engines/overlay/shaders/overlay_pointcloud_only_vert.glsl
  engines/overlay/shaders/overlay_sculpt_curves_selection_frag.glsl
  engines/overlay/shaders/overlay_sculpt_curves_selection_vert.glsl
  engines/overlay/shaders/overlay_sculpt_mask_frag.glsl
  engines/overlay/shaders/overlay_sculpt_mask_vert.glsl
  engines/overlay/shaders/overlay_uniform_color_frag.glsl
  engines/overlay/shaders/overlay_varying_color.glsl
  engines/overlay/shaders/overlay_viewer_attribute_curve_vert.glsl
  engines/overlay/shaders/overlay_viewer_attribute_curves_vert.glsl
  engines/overlay/shaders/overlay_viewer_attribute_frag.glsl
  engines/overlay/shaders/overlay_viewer_attribute_mesh_vert.glsl
  engines/overlay/shaders/overlay_viewer_attribute_pointcloud_vert.glsl
  engines/overlay/shaders/overlay_volume_gridlines_vert.glsl
  engines/overlay/shaders/overlay_volume_velocity_vert.glsl
  engines/overlay/shaders/overlay_wireframe_frag.glsl
  engines/overlay/shaders/overlay_wireframe_vert.glsl
  engines/overlay/shaders/overlay_xray_fade_frag.glsl

  engines/overlay/overlay_shader_shared.h
)

set(GLSL_C)

foreach(GLSL_FILE ${GLSL_SRC})
  data_to_c_simple(${GLSL_FILE} GLSL_C)
endforeach()

kraken_add_lib(kraken_draw_shaders "${GLSL_C}" "" "" "")

list(APPEND LIB
  kraken_draw_shaders
)

set(GLSL_SOURCE_CONTENT "")

foreach(GLSL_FILE ${GLSL_SRC})
  get_filename_component(GLSL_FILE_NAME ${GLSL_FILE} NAME)
  string(REPLACE "." "_" GLSL_FILE_NAME_UNDERSCORES ${GLSL_FILE_NAME})
  string(APPEND GLSL_SOURCE_CONTENT "SHADER_SOURCE\(datatoc_${GLSL_FILE_NAME_UNDERSCORES}, \"${GLSL_FILE_NAME}\", \"${GLSL_FILE}\"\)\n")
endforeach()

set(glsl_source_list_file "${CMAKE_CURRENT_BINARY_DIR}/glsl_draw_source_list.h")
file(GENERATE OUTPUT ${glsl_source_list_file} CONTENT "${GLSL_SOURCE_CONTENT}")
list(APPEND SRC ${glsl_source_list_file})
list(APPEND INC ${CMAKE_CURRENT_BINARY_DIR})

target_include_directories(kraken_draw_shaders PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

list(APPEND INC)

kraken_add_lib(kraken_draw "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")

add_dependencies(kraken_draw kraken_universe)
