template<int location, typename type_t>
[[spirv::in(location)]]
type_t shader_in;

template<int location, typename type_t>
[[spirv::out(location)]]
type_t shader_out;

struct uniforms_t {
  mat4 proj;
  vec2 mouse_pos;
  vec2 resolution;
  float time;
  uint render_mode;
  uint lane_size;
};

[[spirv::uniform(0)]]
uniforms_t u;

extern "C" [[spirv::vert]]
void wave_vert() {
  glvert_Output.Position = u.proj * shader_in<0, vec4>;
  shader_out<0, vec4> = shader_in<1, vec4>;
}

inline float tex_pattern(vec2 texcoord) {
  float scale = .13;
  float t = sin(texcoord.x * scale) + cos(texcoord.y * scale);
  float c = smoothstep(0.f, 0.2f, t * t);
  return c;
}

inline uint get_num_active_lanes() {
  // Submit a flag for each active lane and add up the bits.
  ivec4 bc = bitCount(gl_subgroupBallot(true));
  uint num_active_lanes = bc.x + bc.y + bc.z + bc.w;
  return num_active_lanes;
}

extern "C" [[spirv::frag]]
void wave_frag() {
  vec4 pos = glfrag_FragCoord;
  float texP = tex_pattern(pos.xy);
  vec4 color = texP * shader_in<0, vec4>;

  switch(u.render_mode) {
    
    case 1:
      // Default coloring.
      break;

    case 2: {
      // Color by lane ID.
      float x = gl_SubgroupInvocationID / float(u.lane_size);
      color = vec4(x, x, x, 1);
      break;
    }

    case 3:
      // Mark the first lane as white pixel.
      if(gl_subgroupElect())
        color = vec4(1);
      break;

    case 4:
      // Color the first lane white and the last active lane red.
      if(gl_subgroupElect())
        color = vec4(1);
      else if(gl_SubgroupInvocationID == gl_subgroupMax(gl_SubgroupInvocationID))
        color = vec4(1, 0, 0, 1);
      break;
 
    case 5: {
      float active_ratio = get_num_active_lanes() / float(u.lane_size);
      color = vec4(active_ratio, active_ratio, active_ratio, 1);
      break;
    }
    
    case 6:
      // Broadcast the color in the first lane.
      color = gl_subgroupBroadcastFirst(color);
      break;

    case 7: {
      // Paint the wave with the averaged color inside the wave.
      color = gl_subgroupAdd(color) / get_num_active_lanes();
      break;
    }

    case 8: {
      // First, compute the prefix sum of distance each lane to first lane.
      vec4 base_pos = gl_subgroupBroadcastFirst(pos);
      vec4 prefix_sum = gl_subgroupExclusiveAdd(pos - base_pos);

      // Then, normalize by the number of active lanes.
      color = prefix_sum / get_num_active_lanes();
      break;
    }

    case 9: {
      float dx = gl_subgroupQuadSwapHorizontal(pos.x) - pos.x;
      float dy = gl_subgroupQuadSwapVertical(pos.y) - pos.y;

      if(dx > 0 && dy > 0)
        color = vec4(1, 0, 0, 1);
      else if(dx < 0 && dy > 0)
        color = vec4(0, 1, 0, 1);
      else if(dx > 0 && dy < 0)
        color = vec4(0, 0, 1, 1);
      else if(dx < 0 && dy < 0)
        color = vec4(1, 1, 1, 1);
      else
        color = vec4(0, 0, 0, 1);
      break;
    }
    
  }

  shader_out<0, vec4> = color;
}

////////////////////////////////////////////////////////////////////////////////

[[spirv::uniform(0)]] texture2D g_texture;
[[spirv::uniform(1)]] texture2D g_ui_layer;
[[spirv::uniform(0)]] sampler g_sampler;

extern "C" [[spirv::vert]]
void mag_vert() {
  glvert_Output.Position = u.proj * shader_in<0, vec4>;
  shader_out<0, vec2> = shader_in<1, vec2>;
}

extern "C" [[spirv::frag]]
void mag_frag() {
  
  float ar = u.resolution.x / u.resolution.y;
  float mag_factor = 6;
  float mag_area_size = .05;
  float mag_area_border = .005;

  // Check the distance between this pixel and mouse location in UV space.
  vec2 norm_pixel_pos = shader_in<0, vec2>;
  vec2 norm_mouse_pos = u.mouse_pos / u.resolution;
  vec2 diff = abs(norm_pixel_pos - norm_mouse_pos);
  
  vec4 color = texture(combine(g_texture, g_sampler), norm_pixel_pos);
  vec4 ui = texture(combine(g_ui_layer, g_sampler), norm_pixel_pos);
  
  color = mix(color, ui, ui.a);

  if(diff.x < (mag_area_size + mag_area_border) && 
    diff.y < (mag_area_size + mag_area_border) * ar)
    color = vec4(0, 1, 1, 1);

  if(diff.x < mag_area_size && diff.y < mag_area_size * ar)
    color = texture(combine(g_texture, g_sampler), 
      norm_mouse_pos + (norm_pixel_pos - norm_mouse_pos) / mag_factor);

  shader_out<0, vec4> = color;
}
 
