template<int location, typename type_t>
[[using spirv: in, location(location)]]
type_t shader_in;

template<int location, typename type_t>
[[using spirv: out, location(location)]]
type_t shader_out;

struct object_data_t {
  mat4 gWorld;
};
[[using spirv: uniform, binding(0)]]
object_data_t object_data;

struct pass_data_t {
  mat4 gViewProj;
  vec3 eyePos;
};
[[using spirv: uniform, binding(1)]]
pass_data_t pass_data;

extern "C" [[spirv::vert]]
void vert() {
  glvert_Output.Position = vec4(shader_in<0, vec3>, 1);
}

inline void patch_constant() {
  vec3 p0 = gltesc_Input[0].Position.xyz;
  vec3 p1 = gltesc_Input[1].Position.xyz;
  vec3 p2 = gltesc_Input[2].Position.xyz;
  vec3 p3 = gltesc_Input[3].Position.xyz;

  vec3 centerL = .25f * (p0 + p1 + p2 + p3);
  vec3 centerW = (object_data.gWorld * vec4(centerL, 1)).xyz;

  float d = distance(centerW, pass_data.eyePos);
  float d0 = 20;
  float d1 = 100;
  float tess = 64 * saturate((d1 - d) / (d1 - d0));

  // Uniformly tessellate the patch.
  gltesc_LevelOuter[0] = tess;
  gltesc_LevelOuter[1] = tess;
  gltesc_LevelOuter[2] = tess;
  gltesc_LevelOuter[3] = tess;
  gltesc_LevelInner[0] = tess;
  gltesc_LevelInner[1] = tess;
}

extern "C"
[[using spirv: tesc(quads, 4), output(triangle_cw), spacing(fractional_even)]]
[[dxil::patch_constant(patch_constant)]]
void tesc() {
  // Pass the patch positions through.
  gltesc_Output.Position = gltesc_Input[gltesc_InvocationID].Position;
}

extern "C"
[[using spirv: tese(quads, 4)]]
void tese() {
  float u = gltese_TessCoord.x;
  float v = gltese_TessCoord.y;

  vec3 p0 = gltese_Input[0].Position.xyz;
  vec3 p1 = gltese_Input[1].Position.xyz;
  vec3 p2 = gltese_Input[2].Position.xyz;
  vec3 p3 = gltese_Input[3].Position.xyz;
  
  vec3 v1 = mix(p0, p1, u);
  vec3 v2 = mix(p2, p3, u);
  vec3 p = mix(v1, v2, v);

  // Displacement mapping.
  p.y = .3f * (p.z * sin(p.x) + p.x * cos(p.z));
  vec4 pos = vec4(p, 1) * object_data.gWorld * pass_data.gViewProj;

  glvert_Output.Position = pos;
}

extern "C" [[spirv::frag]]
void frag() {
  shader_out<0, vec4> = vec4(1);
}

