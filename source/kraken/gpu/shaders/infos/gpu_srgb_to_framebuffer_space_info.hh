/* Kraken. Copyright 2022, Wabi Animation Studios, Ltd. Co. All Rights Reserved. */

/**
 * @file
 * GPU.
 * Pixel Magic.
 */

#include "gpu_shader_create_info.hh"

GPU_SHADER_CREATE_INFO(gpu_srgb_to_framebuffer_space)
    .push_constant(Type::BOOL, "srgbTarget")
    .define("blender_srgb_to_framebuffer_space(a) a");
