/* Kraken. Copyright 2022, Wabi Animation Studios, Ltd. Co. All Rights Reserved. */

/**
 * @file
 * GPU.
 * Pixel Magic.
 */

#include "gpu_shader_create_info.hh"

GPU_SHADER_CREATE_INFO(gpu_shader_2D_image_desaturate_color)
    .additional_info("gpu_shader_2D_image_common")
    .push_constant(Type::VEC4, "color")
    .push_constant(Type::FLOAT, "factor")
    .fragment_source("gpu_shader_image_desaturate_frag.glsl")
    .do_static_compilation(true);
