#pragma KRAKEN_REQUIRE(gpu_shader_colorspace_lib.glsl)

void main()
{
  fragColor = finalColor;
  fragColor = kraken_srgb_to_framebuffer_space(fragColor);
}
