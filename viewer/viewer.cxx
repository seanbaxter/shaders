#define CGLTF_IMPLEMENTATION
#include "../thirdparty/cgltf/cgltf.h"

#define STB_IMAGE_IMPLEMENTATION
#include "texture.hxx"

#include "appglfw.hxx"
#include <vector>
#include <iostream>
#include <type_traits>
#include <cassert>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

#include "brdf.hxx"
#include "tonemapping.hxx"


// PBR metallic roughness
struct pbrMetallicRoughness_t {
  // baseColorTexture
  // metallicRoughnessTexture
  vec4 baseColorFactor;
  float metallicFactor;
  float roughnessFactor;
  float padding0, padding1;
};

// KHR_materials_pbrSpecularGlossiness
struct KHR_materials_pbrSpecularGlossiness_t {
  // diffuseTexture
  // specularGlossinessTexture
  vec4 diffuseFactor;
  vec3 specularFactor;
  float glossinessFactor;
};

// KHR_materials_clearcoat
struct KHR_materials_clearcoat_t {
  // clearcoatTexture
  // clearcoatRoughnessTexture
  // clearcoatNormalTexture
  float clearcoatFactor;
  float clearcoatRoughnessFactor;
  float padding0, padding1;
};

// KHR_materials_transmission 
struct KHR_materials_transmission_t {
  // transmissionTexture
  float transmissionFactor;
  float ior;
  float padding0, padding1;
};

struct material_uniform_t {
  // Textures:
  // normalTexture
  // occlusionTexture
  // emissiveTexture
  vec3 emissiveFactor;
  float alphaCutoff;

  // Material models
  pbrMetallicRoughness_t pbrMetallicRoughness;
  KHR_materials_pbrSpecularGlossiness_t pbrSpecularGlossiness;
  KHR_materials_clearcoat_t clearcoat;
  KHR_materials_transmission_t transmission;
};

enum light_type_t {
  light_type_directional,
  light_type_point,
  light_type_spot,
};

struct light_t {
  vec3 direction;
  float range;

  vec3 color;
  float intensity;

  vec3 position;
  float innerConeCos;

  float outerConeCos;
  light_type_t type;

  vec2 padding;
};

struct uniform_t {
  // Transform model space into world space.
  mat4 model_to_world;

  // Transform world space into clip space.
  mat4 view_projection;

  // The sky projection*view. This removes any translation.
  mat4 sky_view_projection;

  // The normal matrix is just a rotation (and possibly a scale). It 
  // ignores translation, so encode it as a mat3.
  mat3x4 normal;

  vec3 camera;     // Position of camera.

  float normal_scale;

  material_uniform_t material;
  float exposure;

  int light_count;

  /*
  light_t lights[16];

  mat4 joint_matrices[75];

  // TODO: Make these mat4x3. We only use 3D matrix operations.
  mat4 joint_normal_matrices[75];
  */
};


////////////////////////////////////////////////////////////////////////////////
// Vertex attribute and sampler binding locations.

enum typename vattrib_index_t {
  // Shader locations for vertex attributes.
  vattrib_position = vec3,
  vattrib_normal   = vec3,
  vattrib_tangent  = vec3,
  vattrib_binormal = vec3,

  // Repeat these vertex attributes in cycles of three. We don't have to 
  // bind all of them.
  vattrib_texcoord0 = vec2,
  vattrib_joints0   = ivec4,
  vattrib_weights0  = vec4,

  vattrib_texcoord1 = vec2,
  vattrib_joints1   = ivec4,
  vattrib_weights1  = vec4,
};

enum typename sampler_index_t {
  // Core
  sampler_normal               = sampler2D,
  sampler_occlusion            = sampler2D,
  sampler_emissive             = sampler2D,

  // pbrMetallicRoughness
  sampler_baseColor            = sampler2D,
  sampler_metallicRoughness    = sampler2D,

  // pbrSpecularGlossiness
  sampler_diffuse              = sampler2D,
  sampler_specularGlossiness   = sampler2D,

  // KHR_materials_clearcoat
  sampler_clearcoat            = sampler2D,
  sampler_clearcoatRoughness   = sampler2D,
  sampler_clearcoatNormal      = sampler2D,

  // KHR_materials_transmission
  sampler_transmission         = sampler2D,

  // Image-based lighting textures and LUTs.
  // The Env samplers are cube maps.
  sampler_GGXLut               = sampler2D,
  sampler_GGXEnv               = samplerCube,
  sampler_LambertianEnv        = samplerCube,
  sampler_CharlieLut           = sampler2D,
  sampler_CharlieEnv           = samplerCube,
};


////////////////////////////////////////////////////////////////////////////////
// Vertex and fragment shader feature sets. The spaceship generates
// relational operators so we can use these as keys in maps.
// Shader specialization constants 

struct vert_features_t {
  // Each flag indicates the availability of a vertex attribute.
  // vattrib_position is always available.
  bool normal;      // vattrib_normal
  bool tangent;     // vattrib_tangent + vattrib_binormal
  bool texcoord0;   // vattrib_texcoord0
  bool texcoord1;   // vattrib_texcoord1
  bool joints0;     // vattrib_joints0 + vattrib_weights0
  bool joints1;     // vattrib_joints1 + vattrib_weights1
};

[[spirv::constant(0)]]
vert_features_t vert_features;

struct frag_features_t {
  // Incoming vertex properties.
  bool normal;
  bool tangents;

  // Available texture maps.
  bool normal_map;
  bool emissive_map;
  bool occlusion_map;
  
  // Material properties.
  bool metallicRoughness;
  bool anisotropy;
  bool ibl;
  bool point_lights;
};

[[spirv::constant(0)]]
frag_features_t frag_features;

[[using spirv: uniform, binding(0)]]
uniform_t uniforms;


inline mat4 skinning_matrix(ivec4 joints, vec4 weights, const mat4* matrices) {
  mat4 skin =
    weights.x * matrices[joints.x] +
    weights.y * matrices[joints.y] +
    weights.z * matrices[joints.z] +
    weights.w * matrices[joints.w];
  return skin;
}

[[spirv::vert]]
void vert_main() {
  // Always load the position attribute.
  vec4 pos = vec4(shader_in<vattrib_position>, 1);

  // TODO: Apply skeletal animation.
  /*
  if(vert_features.joints0) {
    // Compute the first 4 components of the skin matrix.
    mat4 skin = skinning_matrix(
      shader_in<vattrib_joints0, ivec4>, 
      shader_in<vattrib_weights0>,
      uniforms.joint_matrices
    );

    if(vert_features.joints1) {
      // Compute the next 4 components of the skin matrix.
      skin += skinning_matrix(
        shader_in<vattrib_joints1, ivec4>,
        shader_in<vattrib_weights1>,
        uniforms.joint_matrices
      );
    }

    // Advance the position by the skin matrix.
    pos = skin * pos;
  }
  */
  // Transform the model vertex into world space.
  pos = uniforms.model_to_world * pos;

  // Pass the vertex position to the fragment shader.
  shader_out<vattrib_position> = pos.xyz / pos.w;

  // Pass the vertex normal to the fragment shader.
  if(vert_features.normal) {
    // Load the vertex normal attribute.
    vec3 n = shader_in<vattrib_normal>;

    // TODO: Apply morphing.
    // TODO: Apply skinning.

    // Rotate into normal space and send to the fragment shader.
    shader_out<vattrib_normal> = normalize(mat3(uniforms.normal) * n);
  }

  // Pass through texcoords.
  if(vert_features.texcoord0)
    shader_out<vattrib_texcoord0> = shader_in<vattrib_texcoord0>;

  if(vert_features.texcoord1)
    shader_out<vattrib_texcoord1> = shader_in<vattrib_texcoord1>;

  // Set the vertex positions.
  glvert_Output.Position = uniforms.view_projection * pos;
}


////////////////////////////////////////////////////////////////////////////////

struct normal_info_t {
  vec3 ng;
  vec3 n;
  vec3 t;
  vec3 b;
};

inline normal_info_t get_normal_info(vec3 pos, vec3 v, vec2 uv) {
  // Get derivatives of texcoord wrt fragment coordinates.
  vec3 uv_dx = vec3(glfrag_dFdx(uv), 0);
  vec3 uv_dy = vec3(glfrag_dFdy(uv), 0);
  vec3 t_ = (uv_dy.t * glfrag_dFdx(pos) - uv_dx.t * glfrag_dFdy(pos)) /
    (uv_dx.s * uv_dy.t - uv_dy.s * uv_dx.t);

  vec3 ng, t, b;
  if(frag_features.tangents) {
    ng = shader_in<vattrib_normal>;
    t = shader_in<vattrib_tangent>;
    b = shader_in<vattrib_binormal>;

  } else {
    if(frag_features.normal) {
      ng = shader_in<vattrib_normal>;

    } else {
      ng = normalize(cross(glfrag_dFdx(pos), glfrag_dFdy(pos)));
    }

    // Compute tangent and binormal.
    t = normalize(t_ - ng * dot(ng, t_));
    b = cross(ng, t);
  }

  // For back-facing surface, the tangential basis vectors are negated.
  float facing = 2 * step(0.f, dot(v, ng)) - 1;
  t *= facing;
  b *= facing;
  ng *= facing;

  vec3 direction;
  if(frag_features.anisotropy) {

  } else {
    direction = vec3(1, 0, 0);
  }

  t = mat3(t, b, ng) * direction;
  b = normalize(cross(ng, t));

  vec3 n;
  if(frag_features.normal_map) {
    n = 2 * texture(shader_sampler<sampler_normal>, uv).rgb - 1;
    n *= vec3(uniforms.normal_scale, uniforms.normal_scale, 1);
    n = mat3(t, b, ng) * normalize(n);

  } else {
    n = ng;

  }

  normal_info_t info;
  info.ng = ng;
  info.t = t;
  info.b = b;
  info.n = n;
  return info;
}


struct material_info_t {
  vec3 f0;
  float roughness;

  vec3 albedo;
  float alpha_roughness;

  vec3 f90;
  float metallic;

  vec3 n;
  vec3 base_color;
};

inline void get_metallic_roughness(material_info_t& info, float f0, vec2 uv) {
  info.metallic = 1; uniforms.material.pbrMetallicRoughness.metallicFactor;
  info.roughness = 1; uniforms.material.pbrMetallicRoughness.roughnessFactor;

  // Sample the metallic-roughness texture. This has g and b channels.
  vec4 mr = texture(shader_sampler<sampler_metallicRoughness>, uv);
  info.roughness *= mr.g;
  info.metallic *= mr.b;

  // TODO: Provide for specular override of f0.

  info.albedo = mix(info.base_color * (1 - f0), 0, info.metallic);
  info.f0 = mix(f0, info.base_color, info.metallic);
}

// https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#appendix-b-brdf-implementation
inline float get_range_attenuation(float range, float distance) {
  // NOTE: Multiple returns cause validation error.
  if(range <= 0)
    return 1;
  else
    return clamp(1.f - pow(distance / range, 4.f), 1.f, 0.f) / 
      (distance * distance);
}

inline float get_spot_attenuation(vec3 point_to_light, vec3 direction, 
  float outer_cos, float inner_cos) {

  float cos = dot(normalize(direction), -normalize(point_to_light));

  return 
    cos <= outer_cos ? 0 :
    cos >= inner_cos ? 1 :
    smoothstep(outer_cos, inner_cos, cos);
}

// Image-based lighting.

inline vec3 getIBLRadianceGGX(vec3 n, vec3 v, float roughness, vec3 color) {
  int levels = textureQueryLevels(shader_sampler<sampler_GGXEnv>);
  
  float NdotV = clamp(dot(n, v), 0.f, 1.f);
  float lod = levels * clamp(roughness, 0.f, 1.f);

  vec3 reflection = normalize(reflect(-v, n));

  vec2 brdfSamplePoint = clamp(vec2(NdotV, roughness), 0, 1);
  vec2 brdf = texture(
    shader_sampler<sampler_GGXLut>, 
    brdfSamplePoint
  ).rg;

  vec3 specular = textureLod(
    shader_sampler<sampler_GGXEnv>,
    reflection, 
    lod
  ).rgb;

  return specular * (color * brdf.x + brdf.y);
}

inline vec3 getIBLRadianceLambertian(vec3 n, vec3 color) {
  vec3 diffuseLight = texture(
    shader_sampler<sampler_LambertianEnv>, 
    n
  ).rgb;

  return diffuseLight * color;
}


[[spirv::frag]]
void frag_main() {
  vec3 pos = shader_in<vattrib_position>;
  vec2 uv = shader_in<vattrib_texcoord0>;

  vec4 base_color = texture(shader_sampler<sampler_baseColor>, uv);
  base_color = sRGBToLinear(base_color);

  vec3 v = normalize(uniforms.camera - pos);

  // Get the position of the fragment.
  normal_info_t normal = get_normal_info(pos, v, uv);

  vec3 n = normal.n;
  vec3 t = normal.t;
  vec3 b = normal.b;

  float NdotV = clamp(dot(n, v), 0.f, 1.f);
  float TdotV = clamp(dot(t, v), 0.f, 1.f);
  float BdotV = clamp(dot(b, v), 0.f, 1.f);

  // The default index of refraction of 1.5 yields a dielectric normal incidence reflectance of 0.04.
  float ior = 1.5;
  float f0_ior = 0.04;

  material_info_t material { };
  material.base_color = base_color.rgb;

  if(frag_features.metallicRoughness) {
    // Load metallic-roughness properties once. We'll need these for each light
    // in the scene.
    get_metallic_roughness(material, f0_ior, uv);
  }

  // Clamp the metallic-roughness parameters.
  material.roughness = clamp(material.roughness, 0.f, 1.f);
  material.metallic = clamp(material.metallic, 0.f, 1.f);

  material.alpha_roughness = material.roughness * material.roughness;

  // Compute reflectance.
  float reflectance = max(max(material.f0.r, material.f0.g), material.f0.b);
  material.f90 = vec3(clamp(50 * reflectance, 0.f, 1.f));

  material.n = n;

  vec3 f_diffuse(0);
  vec3 f_specular(0);

  if(frag_features.ibl) {
    // Use image-based lighting.
    f_specular += getIBLRadianceGGX(n, v, material.roughness, material.f0);
    f_diffuse += getIBLRadianceLambertian(n, material.albedo);
  }

  /*
  // Specialization constant over point lighting.
  if(frag_features.point_lights) {

    // Dynamic uniform control flow over all lights.
    for(int i = 0; i < uniforms.light_count; ++i) {
      light_t light = uniforms.lights[i];

      vec3 point_to_light = -light.direction;

      float attenuation = 1;

      if(light_type_directional == light.type) {
        point_to_light = -light.direction;

      } else if(light_type_point == light.type) {
        point_to_light = light.position - pos;
        attenuation = get_range_attenuation(light.range, length(point_to_light));

      } else {
        // light_type_spot
        point_to_light = light.position - pos;

        attenuation = get_range_attenuation(light.range, length(point_to_light));
        attenuation *= get_spot_attenuation(point_to_light, light.direction, 
          light.outerConeCos, light.innerConeCos);
      }

      vec3 intensity = attenuation * light.intensity * light.color;

      vec3 l = normalize(point_to_light);
      vec3 h = normalize(l + v);    // Half-angle vector.
      float NdotL = clamp(dot(n, l), 0.f, 1.f);
      float NdotH = clamp(dot(n, h), 0.f, 1.f);
      float LdotH = clamp(dot(l, h), 0.f, 1.f);
      float VdotH = clamp(dot(v, h), 0.f, 1.f);

      if(NdotL > 0 || NdotV > 0) {
        // Calculation of analytical light
        //https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#acknowledgments AppendixB
        f_diffuse += intensity * NdotL *  BRDF_lambertian(material.f0, 
          material.f90, material.albedo, VdotH);

        f_specular += intensity * NdotL * BRDF_specularGGX(material.f0, 
          material.f90, material.alpha_roughness, VdotH, NdotL, NdotV, 
          NdotH);
      }
    }
  }
  */

  vec3 f_emissive(0);
  if(frag_features.emissive_map) {
    vec3 sample = texture(shader_sampler<sampler_emissive>, uv).rgb;
    f_emissive = sRGBToLinear(sample);
  }

  vec3 color = f_diffuse + f_specular + f_emissive;

  if(frag_features.occlusion_map) {
    float ao = texture(shader_sampler<sampler_occlusion>, uv).r;
    color = mix(color, color * ao, 1.f);
  }

  shader_out<0, vec4> = vec4(toneMap(color, uniforms.exposure), base_color.a);
}

////////////////////////////////////////////////////////////////////////////////
// Skybox shaders

[[spirv::vert]]
void vert_sky() {
  vec3 vertex = shader_in<vattrib_position>;
  vec4 pos = uniforms.sky_view_projection * vec4(vertex, 1);

  // Return .z with the .w value to get maximum depth for the skybox.
  glvert_Output.Position = pos.xyww;

  // Use the cube vertex position as the sampler.
  shader_out<vattrib_position> = vertex;
}

[[spirv::frag]]
void frag_sky() {
  vec3 texcoord = shader_in<vattrib_position>;
  vec3 color = texture(
    shader_sampler<sampler_GGXEnv>, 
    texcoord,
    -.5f
  ).rgb;

  shader_out<0, vec4> = vec4(toneMap(color, uniforms.exposure), 1);
}

////////////////////////////////////////////////////////////////////////////////

struct env_map_t {
  // GL textures.
  GLuint GGXLut;          // specular/specular.ktx2
  GLuint GGXEnv;          // images/lut_ggx.png
  GLuint LambertianEnv;   // lambertian/diffuse.ktx2
  GLuint CharlieLut;      // images/lut_charlie.png
  GLuint CharlieEnv;      // charlie/sheen.ktx2
};

struct env_paths_t {
  const char* GGXLut;
  const char* GGXEnv;
  const char* LambertianEnv;
  const char* CharlieLut;
  const char* CharlieEnv;
};

GLuint create_hdr_cubemap(const char* path) {
  const uint8_t ktx2_version[12] {
    0xAB, 0x4B, 0x54, 0x58, 0x20, 0x32, 0x30, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A
  };

  struct ktx2_header_t {
    char identifier[12];
    uint32_t vkFormat;
    uint32_t typeSize;
    uint32_t width;
    uint32_t height;
    uint32_t pixelDepth;
    uint32_t layerCount;
    uint32_t faceCount;
    uint32_t levelCount;
    uint32_t supercompressionScheme;

    // Index 
    uint32_t dfdByteOffset;
    uint32_t dfdByteLength;
    uint32_t kvdByteOffset;
    uint32_t kvdByteLength;
    uint64_t sgdByteOffset;
    uint64_t sgdByteLength;

    struct indexing_t {
      uint64_t byteOffset;
      uint64_t byteLength;
      uint64_t uncompressedByteLength;
    };
    indexing_t levels[1];   // levelCount elements.
  };
    
  int fd = open(path, O_RDONLY);
  if(-1 == fd) {
    printf("cannot open file %s\n", path);
    exit(1);
  }

  struct stat statbuf;
  if(-1 == fstat(fd, &statbuf)) {
    printf("cannot stat file %s\n", path);
    exit(1);
  }

  if(statbuf.st_size < sizeof(ktx2_header_t)) {
    printf("file %s is too small to be a ktx2 file", path);
    exit(1);
  }

  const char* data = (const char*)mmap(nullptr, statbuf.st_size, PROT_READ, 
    MAP_PRIVATE, fd, 0);
  const ktx2_header_t& header = *(const ktx2_header_t*)data;

  if(memcmp(header.identifier, ktx2_version, 12)) {
    printf("file %s has the wrong KTX2 identifier", path);
    exit(1);
  }

  if(6 != header.faceCount) {
    printf("file %s does not hold a cube map", path);
    exit(1);
  }

  GLenum format, iformat;
  switch(header.vkFormat) {
    case 97:
      format = GL_HALF_FLOAT;
      iformat = GL_RGBA16F;
      break;

    case 109:
      format = GL_FLOAT;
      iformat = GL_RGBA32F;
      break;

    default:
      printf("cube map in %s is not encoded in a supported format", path);
      exit(1);
  }

  // Construct the cube map and all of its face-layers.
  GLuint cubemap;
  glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &cubemap);
  glTextureStorage2D(cubemap, header.levelCount, iformat, header.width,
    header.height);

  // The file is encoded by levels.
  for(int level = 0; level < header.levelCount; ++level) {
    ktx2_header_t::indexing_t indexing = header.levels[level];

    // Find the mip map level dimensions.
    uint32_t width = header.width >> level;
    uint32_t height = header.height >> level;

    // Upload all six levels at once.
    const char* level_data = data + indexing.byteOffset;
    glTextureSubImage3D(cubemap, level, 0, 0, 0, width, height, 6, GL_RGBA, 
      GL_HALF_FLOAT, level_data);
  }

  munmap((void*)data, statbuf.st_size);
  close(fd);

  glTextureParameteri(cubemap, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTextureParameteri(cubemap, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTextureParameteri(cubemap, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTextureParameteri(cubemap, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  
  return cubemap;
}

env_map_t load_env_map(env_paths_t paths) {
  env_map_t map { };

  map.GGXLut = load_texture(paths.GGXLut);
  map.GGXEnv = create_hdr_cubemap(paths.GGXEnv);

  map.LambertianEnv = create_hdr_cubemap(paths.LambertianEnv);

  map.CharlieLut = load_texture(paths.CharlieLut);
  map.CharlieEnv = create_hdr_cubemap(paths.CharlieEnv);
  return map;
}


////////////////////////////////////////////////////////////////////////////////

int find_node_index(const cgltf_data* data, const cgltf_node* node) {
  return node - data->nodes;
}

int find_image_index(const cgltf_data* data, cgltf_image* image) {
  return image - data->images;
}

int find_sampler_index(const cgltf_data* data, cgltf_sampler* sampler) {
  return sampler - data->samplers;
}

int find_texture_index(const cgltf_data* data, cgltf_texture* texture) {
  return texture - data->textures;
}

int find_material_index(const cgltf_data* data, cgltf_material* material) {
  return material - data->materials;
}

int find_view_index(const cgltf_data* data, const cgltf_buffer_view* view) {
  return view - data->buffer_views;
}

int find_buffer_index(const cgltf_data* data, const cgltf_buffer* buffer) {
  return buffer - data->buffers;
}

struct texture_view_t {
  // Index of the texture in the gltf stream.
  int index = -1;

  // Index of the TEXCOORD_n in the vertex attribute stream. 
  int texcoord = 0;

  // The scalar multiplied applied to each normal vector of the normal
  // texture.
  // Also strength for occlusionTextureInfo.
  float scale = 1.f;

  explicit operator bool() const noexcept { return -1 != index; }
};

struct material_textures_t {
  // Core:
  texture_view_t normal;
  texture_view_t occlusion;
  texture_view_t emissive;

  // pbrMetallicRoughness
  texture_view_t baseColor;
  texture_view_t metallicRoughness;

  // KHR_materials_pbrSpecularGlossiness
  texture_view_t diffuse;
  texture_view_t specularGlossiness;

  // KHR_materials_clearcoat
  texture_view_t clearcoat;
  texture_view_t clearcoatRoughness;
  texture_view_t clearcoatNormal;

  // KHR_materials_transmission
  texture_view_t transmission;
};

struct material_t {
  bool has_pbr_metallic_roughness;
  bool has_pbr_specular_glossiness;
  bool has_clearcoat;
  bool has_transmission;

  material_uniform_t uniform;
  material_textures_t textures;
};

// Binary search in the array of animation keyframe times. 
// This searches the input accessor of the animation sampler.
std::pair<int, float> find_animation_interpolate(
  const cgltf_animation_sampler* sampler, float t) {

  // Check sampler->input->min and max.
  cgltf_accessor* input = sampler->input;
  assert(cgltf_component_type_r_32f == input->component_type);
  assert(cgltf_type_scalar == input->type);

  const cgltf_buffer_view* view = input->buffer_view;
  const char* data = (const char*)view->buffer->data + view->offset;

  const float* times = (const float*)data;
  const float* lb = std::lower_bound(times, times + input->count, t);
  
  int index = lb - times;
  float weight = 0;

  if(index < input->count) {
    float t0 = times[index];
    float t1 = times[index + 1];
    weight = (t - t0) / (t1 - t0);

  } else {
    index = input->count - 1;
    weight = 1;
  }

  return { index, weight };
}

vec3 interpolate_lerp(const cgltf_animation_sampler* sampler, 
  std::pair<int, float> interpolant) {

  cgltf_accessor* output = sampler->output;
  assert(cgltf_component_type_r_32f == output->component_type);
  assert(cgltf_type_vec3 == output->type);

  const cgltf_buffer_view* view = output->buffer_view;
  const char* data = (const char*)view->buffer->data + view->offset;

  const vec3* vectors = (const vec3*)data;
  vec3 a = vectors[interpolant.first];
  vec3 b = vectors[interpolant.first + 1];

  return a * (1 - interpolant.second) + b * interpolant.second;
}

vec4 interpolate_slerp(const cgltf_animation_sampler* sampler, 
  std::pair<int, float> interpolant) {

  cgltf_accessor* output = sampler->output;
  assert(cgltf_component_type_r_32f == output->component_type);
  assert(cgltf_type_vec4 == output->type);

  const cgltf_buffer_view* view = output->buffer_view;
  const char* data = (const char*)view->buffer->data + view->offset;

  const vec4* quats = (const vec4*)data;
  vec4 a = quats[interpolant.first];
  vec4 b = quats[interpolant.first + 1];

  return slerp(a, b, interpolant.second);
}

struct animation_t {
  enum path_t {
    path_translation,
    path_rotation,
    path_scale,
    path_weights,
  };

  struct channel_t {
    int sampler;
    int node;
  };
  std::vector<channel_t> channels;

  struct sampler_t {
  };
};

struct sampler_t {
  GLenum mag_filter, min_filter;
  GLenum wrap_s, wrap_t;
};

struct texture_t {
  int image;
  int sampler;
};

struct prim_t {
  int offset;   // byte offset into the buffer.
  int count;
  vec3 min, max;

  int material;

  // The VAO for rendering the primitive.
  GLuint vao = 0;
  GLenum elements_type = GL_NONE;
};

struct mesh_t {
  std::vector<prim_t> primitives;
};

// Create array buffers for storing vertex data.
struct model_t {
  model_t(const char* path);
  ~model_t();
  
  GLuint load_buffer(const cgltf_buffer* buffer);
  GLuint load_image(const cgltf_image* image, const char* data_path);
  sampler_t load_sampler(const cgltf_sampler* sampler);
  texture_t load_texture(const cgltf_texture* texture);
  material_t load_material(const cgltf_material* material);
  texture_view_t load_texture_view(const cgltf_texture_view& view);
  prim_t load_prim(const cgltf_primitive* prim);
  mesh_t load_mesh(const cgltf_mesh* mesh);

  void bind_texture(sampler_index_t sampler_index, texture_view_t view);
  void bind_material(material_t& material);

  void render_primitive(mesh_t& mesh, prim_t& prim);

  std::vector<mesh_t> meshes;
  std::vector<GLuint> buffers;
  std::vector<GLuint> images;
  std::vector<sampler_t> samplers;
  std::vector<texture_t> textures;
  std::vector<material_t> materials;

  std::vector<light_t> lights;

  cgltf_data* data = nullptr;
};

model_t::model_t(const char* path) {
  cgltf_options options { };

  printf("Parsing %s...\n", path);
  cgltf_result result = cgltf_parse_file(&options, path, &data);
  if(cgltf_result_success != result) {
    std::cerr<< enum_to_string(result)<< "\n";
    exit(1);
  }

  result = cgltf_load_buffers(&options, data, path);
  if(cgltf_result_success != result) {
    std::cerr<< enum_to_string(result)<< "\n";
    exit(1);
  }

  // Load the buffers.
  buffers.resize(data->buffers_count);
  for(int i = 0; i < data->buffers_count; ++i) {
    buffers[i] = load_buffer(data->buffers + i);
  }

  // Load the images.
  images.resize(data->images_count);
  for(int i = 0; i < data->images_count; ++i) {
    images[i] = load_image(data->images + i, path);
  }

  // Load the textures.
  textures.resize(data->textures_count);
  for(int i = 0; i < data->textures_count; ++i) {
    textures[i] = load_texture(data->textures + i);
  }

  // Load the samplers. These apply glTextureParameters to the textures.
  samplers.resize(data->samplers_count);
  for(int i = 0; i < data->samplers_count; ++i) {
    samplers[i] = load_sampler(data->samplers + i);
  }

  // Load the materials.
  materials.resize(data->materials_count);
  for(int i = 0; i < data->materials_count; ++i) {
    materials[i] = load_material(data->materials + i);
  }

  // Load the meshes.
  for(int i = 0; i < data->meshes_count; ++i) {
    meshes.push_back(load_mesh(data->meshes + i));
  }
}

model_t::~model_t() {
  for(mesh_t& mesh : meshes) {
    for(prim_t& prim : mesh.primitives)
      glDeleteVertexArrays(1, &prim.vao);
  }

  glDeleteBuffers(buffers.size(), buffers.data());
  glDeleteTextures(images.size(), images.data());
  cgltf_free(data);
}

GLuint model_t::load_buffer(const cgltf_buffer* buffer) {
  GLuint buffer2;
  glCreateBuffers(1, &buffer2);
  glNamedBufferStorage(buffer2, buffer->size, buffer->data, 
    GL_DYNAMIC_STORAGE_BIT);

  printf("Loaded buffer with %zu bytes\n", buffer->size);
  return buffer2;
}

GLuint model_t::load_image(const cgltf_image* image, const char* base) {
  char path[260];

  const char* s0 = strrchr(base, '/');
  const char* s1 = strrchr(base, '\\');
  const char* slash = s0 ? (s1 && s1 > s0 ? s1 : s0) : s1;

  if(slash) {
    size_t prefix = slash - base + 1;
    strncpy(path, base, prefix);
    strcpy(path + prefix, image->uri);

  } else {
    strcpy(path, image->uri);
  }

  return ::load_texture(path);
}

sampler_t model_t::load_sampler(const cgltf_sampler* sampler) {
  sampler_t sampler2 { };

  sampler2.mag_filter = sampler->mag_filter ? sampler->mag_filter :
    GL_LINEAR;
  
  sampler2.min_filter = sampler->min_filter ? sampler->min_filter :
    GL_LINEAR_MIPMAP_LINEAR;

  sampler2.wrap_s = sampler->wrap_s;
  sampler2.wrap_t = sampler->wrap_t;
  return sampler2;
}

texture_t model_t::load_texture(const cgltf_texture* texture) {
  return {
    find_image_index(data, texture->image),
    find_sampler_index(data, texture->sampler)
  };
}


material_t model_t::load_material(const cgltf_material* material) {
  material_t material2 { };

  // Core terms.
  material2.uniform.emissiveFactor = vec3(
    material->emissive_factor[0],
    material->emissive_factor[1],
    material->emissive_factor[2]
  );
  material2.uniform.alphaCutoff = material->alpha_cutoff;

  material2.textures.normal = load_texture_view(material->normal_texture);
  material2.textures.occlusion = load_texture_view(material->occlusion_texture);
  material2.textures.emissive = load_texture_view(material->emissive_texture);

  if(material->has_pbr_metallic_roughness) {
    material2.has_pbr_metallic_roughness = true;

    material2.uniform.pbrMetallicRoughness.baseColorFactor = vec4(
      material->pbr_metallic_roughness.base_color_factor[0],
      material->pbr_metallic_roughness.base_color_factor[1],
      material->pbr_metallic_roughness.base_color_factor[2],
      material->pbr_metallic_roughness.base_color_factor[3]
    );
    material2.uniform.pbrMetallicRoughness.metallicFactor = 
      material->pbr_metallic_roughness.metallic_factor;
    material2.uniform.pbrMetallicRoughness.roughnessFactor = 
      material->pbr_metallic_roughness.roughness_factor;

    material2.textures.baseColor = load_texture_view(
      material->pbr_metallic_roughness.base_color_texture);
    material2.textures.metallicRoughness = load_texture_view(
      material->pbr_metallic_roughness.metallic_roughness_texture
    );
  }

  if(material->has_pbr_specular_glossiness) {
    material2.has_pbr_specular_glossiness = true;

    material2.uniform.pbrSpecularGlossiness.diffuseFactor = vec4(
      material->pbr_specular_glossiness.diffuse_factor[0],
      material->pbr_specular_glossiness.diffuse_factor[1],
      material->pbr_specular_glossiness.diffuse_factor[2],
      material->pbr_specular_glossiness.diffuse_factor[3]
    );
    material2.uniform.pbrSpecularGlossiness.specularFactor = vec3(
      material->pbr_specular_glossiness.specular_factor[0],
      material->pbr_specular_glossiness.specular_factor[1],
      material->pbr_specular_glossiness.specular_factor[2]
    );
    material2.uniform.pbrSpecularGlossiness.glossinessFactor = 
      material->pbr_specular_glossiness.glossiness_factor;

    material2.textures.diffuse = load_texture_view(
      material->pbr_specular_glossiness.diffuse_texture
    );
    material2.textures.specularGlossiness = load_texture_view(
      material->pbr_specular_glossiness.specular_glossiness_texture
    );
  }

  if(material->has_clearcoat) {
    material2.has_clearcoat = true;

    material2.uniform.clearcoat.clearcoatFactor = 
      material->clearcoat.clearcoat_factor;
    material2.uniform.clearcoat.clearcoatRoughnessFactor = 
      material->clearcoat.clearcoat_roughness_factor;

    material2.textures.clearcoat = load_texture_view(
      material->clearcoat.clearcoat_texture
    );
    material2.textures.clearcoatRoughness = load_texture_view(
      material->clearcoat.clearcoat_roughness_texture
    );
    material2.textures.clearcoatNormal = load_texture_view(
      material->clearcoat.clearcoat_normal_texture
    );
  }

  if(material->has_transmission) {
    material2.has_transmission = true;

    material2.uniform.transmission.transmissionFactor = 
      material->transmission.transmission_factor;

    material2.textures.transmission = load_texture_view(
      material->transmission.transmission_texture
    );
  }

  return material2;
}

texture_view_t model_t::load_texture_view(const cgltf_texture_view& view) {
  texture_view_t view2;
  view2.index = find_texture_index(data, view.texture);
  view2.texcoord = view2.texcoord;
  view2.scale = view2.scale;
  return view2;
}

prim_t model_t::load_prim(const cgltf_primitive* prim) {
  prim_t prim2 { };
  prim2.material = find_material_index(data, prim->material);

  glCreateVertexArrays(1, &prim2.vao);

  if(prim->indices) {
    // Bind the index array.
    const cgltf_accessor* accessor = prim->indices;
    const cgltf_buffer_view* view = accessor->buffer_view;

    prim2.offset = accessor->offset + view->offset;
    prim2.count = accessor->count;
    switch(accessor->component_type) {
      case cgltf_component_type_r_8:
      case cgltf_component_type_r_8u:
        prim2.elements_type = GL_UNSIGNED_BYTE;
        break;

      case cgltf_component_type_r_16:
      case cgltf_component_type_r_16u:
        prim2.elements_type = GL_UNSIGNED_SHORT;
        break;

      case cgltf_component_type_r_32u:
        prim2.elements_type = GL_UNSIGNED_INT;
        break;

      default:
        break;
    }

    // Associate the buffer holding the indices.
    GLuint buffer = buffers[find_buffer_index(data, view->buffer)];
    glVertexArrayElementBuffer(prim2.vao, buffer);
  }
  
  for(int a = 0; a < prim->attributes_count; ++a) {
    const cgltf_attribute* attrib = prim->attributes + a;
    const cgltf_accessor* accessor = attrib->data;
    const cgltf_buffer_view* view = accessor->buffer_view;

    // Get the attribute location.
    vattrib_index_t attribindex;
    switch(attrib->type) {
      case cgltf_attribute_type_position:
        attribindex = vattrib_position;
        break;

      case cgltf_attribute_type_normal:
        attribindex = vattrib_normal;
        break;

      case cgltf_attribute_type_texcoord:
        attribindex = (vattrib_index_t)(vattrib_texcoord0 + 3 * attrib->index);
        break;

      case cgltf_attribute_type_joints:
        attribindex = (vattrib_index_t)(vattrib_joints0 + 3 * attrib->index);
        break;

      case cgltf_attribute_type_weights:
        attribindex = (vattrib_index_t)(vattrib_weights0 + 3 * attrib->index);
        break;
    }

    // Use one binding per attribute. 
    GLuint buffer = buffers[find_buffer_index(data, view->buffer)];
    glVertexArrayVertexBuffer(prim2.vao, attribindex, buffer, 
      view->offset + accessor->offset, accessor->stride);

    // Enable the vertex attribute location.
    glEnableVertexArrayAttrib(prim2.vao, attribindex);

    // Get the attribute size and type.
    GLenum type = GL_NONE;
    int size = 0;
    switch(accessor->type) {
      case cgltf_type_scalar:  size = 1;  break;
      case cgltf_type_vec2:    size = 2;  break;
      case cgltf_type_vec3:    size = 3;  break;
      case cgltf_type_vec4:    size = 4;  break;
      default:                            break;
    }

    switch(accessor->component_type) {
      case cgltf_component_type_r_8:   type = GL_BYTE;           break;
      case cgltf_component_type_r_8u:  type = GL_UNSIGNED_BYTE;  break;
      case cgltf_component_type_r_16:  type = GL_SHORT;          break;
      case cgltf_component_type_r_16u: type = GL_UNSIGNED_SHORT; break;
      case cgltf_component_type_r_32u: type = GL_UNSIGNED_INT;   break;
      case cgltf_component_type_r_32f: type = GL_FLOAT;          break;
      default:                                                   break;
    }

    // Associate the buffer view with the attribute location.
    glVertexArrayAttribBinding(prim2.vao, attribindex, attribindex);

    if(accessor->normalized || GL_FLOAT == type) {
      glVertexArrayAttribFormat(prim2.vao, attribindex, size, type, 
        accessor->normalized, 0);

    } else {
      glVertexArrayAttribIFormat(prim2.vao, attribindex, size, type, 0);
    }
  }

  return prim2;
}

mesh_t model_t::load_mesh(const cgltf_mesh* mesh) {
  mesh_t mesh2;
  mesh2.primitives.resize(mesh->primitives_count);
  for(int i = 0; i < mesh->primitives_count; ++i) 
    mesh2.primitives[i] = load_prim(mesh->primitives + i);
  return mesh2;
}

void model_t::bind_texture(sampler_index_t sampler_index, 
  texture_view_t view) {

  texture_t texture = textures[view.index];
  GLuint image = images[texture.image];
  sampler_t sampler = samplers[texture.sampler];

  glTextureParameteri(image, GL_TEXTURE_WRAP_S, sampler.wrap_s);
  glTextureParameteri(image, GL_TEXTURE_WRAP_T, sampler.wrap_t);
  glTextureParameteri(image, GL_TEXTURE_MIN_FILTER, sampler.min_filter);
  glTextureParameteri(image, GL_TEXTURE_MAG_FILTER, sampler.mag_filter);

  float aniso = 0;
  glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &aniso); 
  
  glTextureParameteri(image, GL_TEXTURE_MAX_ANISOTROPY, aniso);

  glBindTextureUnit(sampler_index, image);
}

void model_t::bind_material(material_t& mat) {
  material_textures_t& tex = mat.textures;

  if(tex.normal)
    bind_texture(sampler_normal, tex.normal);

  if(tex.occlusion)
    bind_texture(sampler_occlusion, tex.occlusion);

  if(tex.emissive)
    bind_texture(sampler_emissive, tex.emissive);

  if(tex.baseColor)
    bind_texture(sampler_baseColor, tex.baseColor);

  if(tex.metallicRoughness)
    bind_texture(sampler_metallicRoughness, tex.metallicRoughness);

  if(tex.diffuse)
    bind_texture(sampler_diffuse, tex.diffuse);

  if(tex.specularGlossiness)
    bind_texture(sampler_specularGlossiness, tex.specularGlossiness);

  if(tex.clearcoat)
    bind_texture(sampler_clearcoat, tex.clearcoat);

  if(tex.clearcoatRoughness)
    bind_texture(sampler_clearcoatRoughness, tex.clearcoatRoughness);

  if(tex.clearcoatNormal)
    bind_texture(sampler_clearcoatNormal, tex.clearcoatNormal);

  if(tex.transmission)
    bind_texture(sampler_transmission, tex.transmission);
}

void model_t::render_primitive(mesh_t& mesh, prim_t& prim) {
  bind_material(materials[prim.material]);
  glBindVertexArray(prim.vao);

  glDrawElements(GL_TRIANGLES, prim.count, prim.elements_type, 
    (void*)prim.offset);
}

////////////////////////////////////////////////////////////////////////////////

struct myapp_t : app_t {
  myapp_t(const char* gltf_path, env_paths_t env_paths);
  void display() override;

  model_t model;
  env_map_t env_map;

  GLuint program;
  GLuint skybox;

  uniform_t uniforms;
  GLuint ubo;

  GLuint skybox_vao;
};


myapp_t::myapp_t(const char* gltf_path, env_paths_t env_paths) : 
  app_t("glTF viewer"), model(gltf_path) { 

  camera.distance = 2.5f;
  camera.pitch = radians(-10.f);
  camera.yaw = radians(-100.f);

  // Load the environment maps.
  env_map = load_env_map(env_paths);

  // Compile the shaders.
  GLuint vs = glCreateShader(GL_VERTEX_SHADER);
  GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
  GLuint vs_sky = glCreateShader(GL_VERTEX_SHADER);
  GLuint fs_sky = glCreateShader(GL_FRAGMENT_SHADER);

  GLuint shaders[] { vs, fs, vs_sky, fs_sky };
  glShaderBinary(4, shaders, GL_SHADER_BINARY_FORMAT_SPIR_V_ARB,
    __spirv_data, __spirv_size);

  vert_features_t vert_features { };
  vert_features.normal = true;
  vert_features.texcoord0 = true;
  specialize_shader(vs, @spirv(vert_main), vert_features);

  frag_features_t frag_features { };
  frag_features.normal = true;
  frag_features.normal_map = true;
  frag_features.emissive_map = true;
  frag_features.occlusion_map = true;
  frag_features.metallicRoughness = true;
  frag_features.ibl = true;
  specialize_shader(fs, @spirv(frag_main), frag_features);
  
  program = glCreateProgram();
  glAttachShader(program, vs);
  glAttachShader(program, fs);
  glLinkProgram(program);

  skybox = glCreateProgram();
  glSpecializeShader(vs_sky, @spirv(vert_sky), 0, nullptr, nullptr);
  glSpecializeShader(fs_sky, @spirv(frag_sky), 0, nullptr, nullptr);
  glAttachShader(skybox, vs_sky);
  glAttachShader(skybox, fs_sky);
  glLinkProgram(skybox);

  // Load the uniforms. 
  glCreateBuffers(1, &ubo);
  glNamedBufferStorage(ubo, sizeof(uniform_t), nullptr, 
    GL_DYNAMIC_STORAGE_BIT);

  // Create the skybox vertex array.
  const vec3 cube_vertices[] {
    -1, -1,  1,
     1, -1,  1,
     1,  1,  1,
    -1,  1,  1,
    -1, -1, -1,
     1, -1, -1,
     1,  1, -1,
    -1,  1, -1,
  };
  const uint cube_indices[] {
    0, 1, 2, 2, 3, 0,
    1, 5, 6, 6, 2, 1,
    7, 6, 5, 5, 4, 7,
    4, 0, 3, 3, 7, 4,
    4, 5, 1, 1, 0, 4,
    3, 2, 6, 6, 7, 3,
  };

  glCreateVertexArrays(1, &skybox_vao);

  GLuint vbo;
  glCreateBuffers(1, &vbo);
  glNamedBufferStorage(vbo, sizeof(cube_vertices), cube_vertices, 0);
  glVertexArrayVertexBuffer(skybox_vao, 0, vbo, 0, sizeof(vec3));
  glEnableVertexArrayAttrib(skybox_vao, 0);
  glVertexArrayAttribFormat(skybox_vao, 0, 3, GL_FLOAT, false, 0);

  GLuint ibo;
  glCreateBuffers(1, &ibo);
  glNamedBufferStorage(ibo, sizeof(cube_indices), cube_indices, 0);
  glVertexArrayElementBuffer(skybox_vao, ibo);
}

void myapp_t::display() {
  const float bg[4] { 0 };
  glClearBufferfv(GL_COLOR, 0, bg);
  glClear(GL_DEPTH_BUFFER_BIT);

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CCW);

  glUseProgram(program);

  int width, height;
  glfwGetWindowSize(window, &width, &height);

  mat4 view = camera.get_view();
  mat4 projection = camera.get_perspective(width, height);

  double int_part;
  double speed = 15; // 15 seconds per rotation
  double angle = modf(glfwGetTime() / speed, &int_part) * (2 * M_PI);
  mat4 model_to_world = make_rotateX(radians(-90.f));
  model_to_world = make_rotateY(angle) * model_to_world;

  uniforms.model_to_world = model_to_world;
  uniforms.view_projection = projection * view;

  view[3] = vec4(0, 0, 0, 1); // remove translation for skybox.
  uniforms.sky_view_projection = projection * view;

  uniforms.normal = mat3x4(uniforms.model_to_world);
  
  uniforms.camera = camera.get_eye();
  uniforms.normal_scale = 1;
  uniforms.light_count = 0;

  // TODO: Add a number of lights on random paths.
  light_t light { };

  // light.direction = vec3(-.7399, -.6428, -.1983);
  light.position = 2 * vec3(0, sin(angle), cos(angle));
  light.range = -1;
  light.color = vec3(1, 1, 1);
  light.intensity = 1;
  light.innerConeCos = 0;
  light.outerConeCos = cos(radians(45.f));
  light.type = light_type_point;

  uniforms.light_count = 0;

  // Bind the environment maps.
  glBindTextureUnit(sampler_GGXLut, env_map.GGXLut);
  glBindTextureUnit(sampler_GGXEnv, env_map.GGXEnv);
  glBindTextureUnit(sampler_LambertianEnv, env_map.LambertianEnv);
  glBindTextureUnit(sampler_CharlieLut, env_map.CharlieLut);
  glBindTextureUnit(sampler_CharlieEnv, env_map.CharlieEnv);

  for(mesh_t& mesh : model.meshes) {
    for(prim_t& prim : mesh.primitives) {
      // Set the material for this primitive.
      uniforms.material = model.materials[prim.material].uniform;
      uniforms.exposure = 1;

      glNamedBufferSubData(ubo, 0, sizeof(uniform_t), &uniforms);
      glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);

      model.render_primitive(mesh, prim);
    }
  }

  // Render the skybox using the lambertian texture.
  glDepthFunc(GL_LEQUAL);
  glFrontFace(GL_CW);
  glUseProgram(skybox);
  glBindVertexArray(skybox_vao);
  glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}

int main(int argc, char** argv) {
  glfwInit();
  gl3wInit();

  const char* path = (2 == argc) ? 
    argv[1] :
    "../assets/DamagedHelmet/glTF/DamagedHelmet.gltf";

  env_paths_t env_paths {
    "../assets/lut_ggx.png",
    "../assets/helipad/ggx/specular.ktx2",
    "../assets/helipad/lambertian/diffuse.ktx2",
    "../assets/lut_charlie.png",
    "../assets/helipad/charlie/sheen.ktx2",
  };

  myapp_t myapp(path, env_paths);
  myapp.loop();
  return 0;
}
