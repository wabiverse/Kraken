#pragma KRAKEN_REQUIRE(common_view_clipping_lib.glsl)
#pragma KRAKEN_REQUIRE(common_view_lib.glsl)
#pragma KRAKEN_REQUIRE(common_hair_lib.glsl)

void main()
{
  bool is_persp = (ProjectionMatrix[3][3] == 0.0);
  float time, thick_time, thickness;
  vec3 world_pos, tan, binor;
  hair_get_pos_tan_binor_time(is_persp,
                              ModelMatrixInverse,
                              ViewMatrixInverse[3].xyz,
                              ViewMatrixInverse[2].xyz,
                              world_pos,
                              tan,
                              binor,
                              time,
                              thickness,
                              thick_time);
  gl_Position = point_world_to_ndc(world_pos);

  if (is_point_domain) {
    finalColor = texelFetch(color_tx, hair_get_base_id());
  }
  else {
    finalColor = texelFetch(color_tx, hair_get_strand_id());
  }
}