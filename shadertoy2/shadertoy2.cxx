#if __circle_build__ < 105
#error "Circle build 105 required to reliably compile this sample"
#endif

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#define GL_GLEXT_PROTOTYPES
#include <gl3w/GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <cstdio>
#include <cstdlib>
#include <complex>
#include <memory>
#include <vector>
#include <atomic>
#include <thread>
#include <csignal>

#include <fstream>
#include "json.hpp"

// Interlacing
#include "adam7.hxx"

namespace imgui {
  // imgui attribute tags.
  using color3   [[attribute]] = void;
  using color4   [[attribute]] = void;
  
  template<typename type_t>
  struct minmax_t {
    type_t min, max;
  };
  using range_float [[attribute]] = minmax_t<float>;
  using range_int   [[attribute]] = minmax_t<int>;

  // Resolution for drag.
  using drag_float [[attribute]] = float;

  using title    [[attribute]] = const char*;
  using url      [[attribute]] = const char*;
}

// Return true if any option has changed.
template<typename options_t>
bool render_imgui(options_t& options, const char* child_name = nullptr) {

  bool changed = false;
  if(!child_name || 
    ImGui::TreeNodeEx(child_name, ImGuiTreeNodeFlags_DefaultOpen)) {

    using namespace imgui;
    if constexpr(@has_attribute(options_t, url))
      ImGui::Text(@attribute(options_t, url));

    @meta for(int i = 0; i < @member_count(options_t); ++i) {{
      typedef @member_type(options_t, i) type_t;
      const char* name = @member_name(options_t, i);
      auto& value = @member_value(options, i);

      if constexpr(@member_has_attribute(options_t, i, color4)) {
        static_assert(std::is_same_v<type_t, vec4>);
        changed |= ImGui::ColorEdit4(name, &value.x);

      } else if constexpr(@member_has_attribute(options_t, i, color3)) {
        static_assert(std::is_same_v<type_t, vec3>);
        changed |= ImGui::ColorEdit3(name, &value.x);

      } else if constexpr(@member_has_attribute(options_t, i, range_float)) {
        auto range = @member_attribute(options_t, i, range_float);

        if constexpr(std::is_same_v<type_t, vec4>) {
          changed |= ImGui::SliderFloat4(name, &value.x, range.min, range.max);

        } else if constexpr(std::is_same_v<type_t, vec3>) {
          changed |= ImGui::SliderFloat3(name, &value.x, range.min, range.max);

        } else if constexpr(std::is_same_v<type_t, vec2>) {
          changed |= ImGui::SliderFloat2(name, &value.x, range.min, range.max);

        } else {
          static_assert(std::is_same_v<type_t, float>);
          changed |= ImGui::SliderFloat(name, &value, range.min, range.max);
        }

      } else if constexpr(@member_has_attribute(options_t, i, drag_float)) {
        auto drag = @member_attribute(options_t, i, drag_float);

        if constexpr(std::is_same_v<type_t, vec4>) {
          changed |= ImGui::DragFloat4(name, &value.x, drag);

        } else if constexpr(std::is_same_v<type_t, vec3>) {
          changed |= ImGui::DragFloat3(name, &value.x, drag);

        } else if constexpr(std::is_same_v<type_t, vec2>) {
          changed |= ImGui::DragFloat2(name, &value.x, drag);

        } else {
          static_assert(std::is_same_v<type_t, float>);
          changed |= ImGui::DragFloat(name, &value, drag);
        }

      } else if constexpr(@member_has_attribute(options_t, i, range_int)) {
        auto range = @member_attribute(options_t, i, range_int);

        if constexpr(std::is_same_v<type_t, ivec4>) {
          changed |= ImGui::SliderInt4(name, &value.x, range.min, range.max);

        } else if constexpr(std::is_same_v<type_t, ivec3>) {
          changed |= ImGui::SliderInt3(name, &value.x, range.min, range.max);

        } else if constexpr(std::is_same_v<type_t, ivec2>) {
          changed |= ImGui::SliderInt2(name, &value.x, range.min, range.max);

        } else {
          static_assert(std::is_same_v<type_t, int>);
          changed |= ImGui::SliderInt(name, &value, range.min, range.max);
        }

      } else if constexpr(std::is_same_v<type_t, bool>) {
        changed |= ImGui::Checkbox(name, &value);

      } else if constexpr(std::is_same_v<type_t, vec4>) {
        changed |= ImGui::DragFloat4(name, &value.x, .01f);

      } else if constexpr(std::is_same_v<type_t, vec3>) {
        changed |= ImGui::DragFloat3(name, &value.x, .01f);

      } else if constexpr(std::is_same_v<type_t, vec2>) {
        changed |= ImGui::DragFloat2(name, &value.x, .01f);
        
      } else if constexpr(std::is_same_v<type_t, float>) {
        changed |= ImGui::DragFloat(name, &value, .01f);
    
      } else if constexpr(std::is_same_v<type_t, int>) {
        changed |= ImGui::DragInt(name, &value);

      } else if constexpr(std::is_class_v<type_t>) {
        // Iterate over each data member.
        changed |= render_imgui(value, name);

      }
    }}

    if(child_name)
      ImGui::TreePop();
  }

  return changed;
}

////////////////////////////////////////////////////////////////////////////////

[[using spirv: in, location(0)]]
vec4 vertex_in;

[[spirv::vert]]
void vert_main() {
  glvert_Output.Position = vertex_in;
}

[[using spirv: out, location(0)]]
vec4 fragColor;

struct shadertoy_uniforms_t {
  // shader-specific parameters.
  vec4 mouse;            // .xy is current or last drag position.
                         // .zw is current or last click.
                         // .zw is negative if mouse button is up.
  vec2 resolution;       // width and height of viewport in pixels.
  float time;            // seconds since simulation started.
};

[[using spirv: uniform, binding(0)]]
shadertoy_uniforms_t uniforms;

template<typename shader_t>
[[using spirv: uniform, binding(1)]]
shader_t shader_ubo;

template<typename shader_t>
[[spirv::frag(lower_left)]]
void frag_main() {
  fragColor = shader_ubo<shader_t>.render(glfrag_FragCoord.xy, uniforms);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// The MIT License
// Copyright © 2013 Inigo Quilez
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions: The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software. THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

// A list of useful distance function to simple primitives. All
// these functions (except for ellipsoid) return an exact
// euclidean distance, meaning they produce a better SDF than
// what you'd get if you were constructing them from boolean
// operations.
//
// More info here:
//
// https://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm

inline float sq(float x) noexcept { return x * x; }
inline float dot2(vec2 v) noexcept { return dot(v, v); }
inline float dot2(vec3 v) noexcept { return dot(v, v); }
inline float ndot(vec2 a, vec2 b) noexcept { return a.x * b.x - a.y * b.y; }

struct sphere_t {
  float sd(vec3 p) const noexcept {
    p -= pos;
    return length(p) - s;
  }

  vec3 pos;
  float s;
};

struct box_t {
  float sd(vec3 p) const noexcept {
    p -= pos;
    vec3 d = abs(p) - b;
    return min(max(d.x, max(d.y, d.z)), 0.f) + length(max(d, 0.f));
  }

  vec3 pos;
  vec3 b;
};

struct bounding_box_t {
  float sd(vec3 p) const noexcept {
    p -= pos;
    p = abs(p) - b;
    vec3 q = abs(p + e) - e;
    return min(min(
      length(max(vec3(p.x,q.y,q.z),0.f))+min(max(p.x,max(q.y,q.z)),0.f),
      length(max(vec3(q.x,p.y,q.z),0.f))+min(max(q.x,max(p.y,q.z)),0.f)),
      length(max(vec3(q.x,q.y,p.z),0.f))+min(max(q.x,max(q.y,p.z)),0.f));
  }

  vec3 pos;
  vec3 b;
  float e;
};

struct ellipsoid_t {
  float sd(vec3 p) const noexcept {
    p -= pos;
    float k0 = length(p / r);
    float k1 = length(p / (r * r));
    return k0 * (k0 - 1) / k1;
  }

  vec3 pos;
  vec3 r;
};

struct torus_t {
  float sd(vec3 p) const noexcept {
    p -= pos;
    return length(vec2(length(p.xz) - t.x, p.y)) - t.y;
  }

  vec3 pos;
  vec2 t;
};

struct capped_torus_t {
  float sd(vec3 p) const noexcept {
    p -= pos;
    p.x = abs(p.x);
    float k = (sc.y * p.x > sc.x * p.y) ? dot(p.xy, sc) : length(p.xy);
    return sqrt(dot2(p) + ra * ra - 2 * ra * k) - rb;
  }

  vec3 pos;
  vec2 sc;
  float ra, rb;
};

struct hex_prism_t {
  float sd(vec3 p) const noexcept {
    p -= pos;
    vec3 q = abs(p);

    const vec3 k = vec3(-0.8660254, 0.5, 0.57735);
    p = q;

    p.xy -= 2 * min(dot(k.xy, p.xy), 0.f) * k.xy;
    vec2 d(
      length(p.xy - vec2(clamp(p.x, -k.z*h.x, k.z*h.x), h.x)) * sign(p.y - h.x),
      p.z - h.y
    );
    return min(max(d.x, d.y), 0.f) + length(max(d, 0.f));
  }

  vec3 pos;
  vec2 h;
};

struct octagon_prism_t {
  float sd(vec3 p) const noexcept {
    p -= pos;

    const vec3 k = vec3(
      -0.9238795325,   // sqrt(2+sqrt(2))/2 
       0.3826834323,   // sqrt(2-sqrt(2))/2
       0.4142135623    // sqrt(2)-1 
    ); 

    // reflections
    p = abs(p);
    p.xy -= 2 * min(dot(vec2( k.x, k.y), p.xy), 0.f) * vec2( k.x, k.y);
    p.xy -= 2 * min(dot(vec2(-k.x, k.y), p.xy), 0.f) * vec2(-k.x, k.y);

    // polygon side
    p.xy -= vec2(clamp(p.x, -k.z * r, k.z * r), r);
    vec2 d(length(p.xy) * sign(p.y), p.z - h);
    return min(max(d.x, d.y), 0.f) + length(max(d, 0.f));
  }

  vec3 pos;
  float r, h;
};

struct capsule_t {
  float sd(vec3 p) const noexcept {
    p -= pos;
    vec3 pa = p - a, ba = b - a;
    float h = clamp(dot(pa, ba) / dot2(ba), 0.f, 1.f);
    return length(pa - ba * h) - r;
  }

  vec3 pos;
  vec3 a;
  vec3 b;
  float r;
};

struct round_cone_t {
  float sd(vec3 p) const noexcept {
    p -= pos;
    vec2 q(length(p.xz), p.y);

    float b = (r1 - r2) / h;
    float a = sqrt(1 - b * b);
    float k = dot(q, vec2(-b, a));

    float dist = 0;
    if(k < 0) 
      dist = length(q) - r1;
    else if(k > a * h) 
      dist = length(q - vec2(0, h)) - r2;
    else 
      dist = dot(q, vec2(a, b)) - r1;
    return dist;
  }

  vec3 pos;
  float r1, r2, h;
};

struct round_cone2_t {
  float sd(vec3 p) const noexcept {
    p -= pos;

    // sampling independent computations (only depend on shape)
    vec3 ba = b - a;
    float l2 = dot2(ba);
    float rr = r1 - r2;
    float a2 = l2 - rr * rr;
    float il2 = 1 / l2;

    // sampling dependant computations
    vec3 pa = p - a;
    float y = dot(pa, ba);
    float z = y - l2;
    float x2 = dot2(pa * l2 - ba * y);
    float y2 = y * y * l2;
    float z2 = z * z * l2;

    // single square root!
    float k = sign(rr) * rr * rr * x2;

    float dist = 0;
    if(sign(z) * a2 * z2 > k)
      dist = sqrt(x2 + z2) *il2 - r2;
    else if(sign(y) * a2 * y2 < k)
      dist = sqrt(x2 + y2) *il2 - r1;
    else
      dist = (sqrt(x2 * a2 * il2) + y * rr) * il2 - r1;
    return dist;
  }

  vec3 pos;
  vec3 a, b;
  float r1, r2;
};

struct tri_prism_t {
  float sd(vec3 p) const noexcept {
    p -= pos;

    const float k = sqrt(3.f);
    vec2 h2 = h;

    h2.x *= .5f * k;
    p.xy /= h2.x;
    p.x = abs(p.x) - 1;
    p.y = p.y + 1 / k;
    if(p.x + k * p.y > 0)
      p.xy = vec2(p.x - k * p.y, -k * p.x - p.y) / 2;
    p.x -= clamp(p.x, -2.f, 0.f);
    float d1 = length(p.xy) * sign(-p.y) * h2.x;
    float d2 = abs(p.z) - h2.y;
    return length(max(vec2(d1, d2), 0.f)) + min(max(d1, d2), 0.f);
  }

  vec3 pos;
  vec2 h;
};

struct cylinder_t {
  float sd(vec3 p) const noexcept {
    p -= pos;
    vec2 d = abs(vec2(length(p.xz), p.y)) - h;
    return min(max(d.x, d.y), 0.f) + length(max(d, 0.f));
  }

  vec3 pos;
  vec2 h;
};

struct cylinder2_t {
  float sd(vec3 p) const noexcept {
    p -= pos;

    vec3 pa = p - a;
    vec3 ba = b - a;
    float baba = dot2(ba);
    float paba = dot(pa, ba);

    float x = length(pa * baba - ba * paba) - r * baba;
    float y = abs(paba - baba * .5f) - baba * .5f;
    float x2 = x * x;
    float y2 = y * y * baba;
    float d = max(x, y) < 0 ? 
      -min(x2, y2) : 
      (x > 0 ? x2 : 0.f) + (y > 0 ? y2 : 0.f);
    return sign(d) * sqrt(abs(d)) / baba;
  }

  vec3 pos;
  vec3 a, b;
  float r;
};

struct cone_t {
  float sd(vec3 p) const noexcept {
    p -= pos;

    vec2 q = h * vec2(c.x, -c.y) / c.y;
    vec2 w(length(p.xz), p.y);

    vec2 a = w - q * clamp(dot(w, q) / dot2(q), 0.f, 1.f);
    vec2 b = w - q * vec2(clamp(w.x / q.x, 0.f, 1.f), 1.f);
    float k = sign(q.y);
    float d = min(dot2(a), dot2(b));
    float s = max(k * (w.x * q.y - w.y * q.x), k * (w.y - q.y));
    return sqrt(d) * sign(s);
  }

  vec3 pos;
  vec2 c;
  float h;
};

struct capped_cone_t {
  float sd(vec3 p) const noexcept {
    p -= pos;

    vec2 q(length(p.xz), p.y);

    vec2 k1(r2, h);
    vec2 k2(r2 - r1, 2 * h);
    vec2 ca(q.x - min(q.x, q.y < 0 ? r1 : r2), abs(q.y) - h);
    vec2 cb = q - k1 + k2 * clamp(dot(k1 - q, k2) / dot2(k2), 0.f, 1.f);

    float s = cb.x < 0 & ca.y < 0 ? -1.f : 1.f;
    return s * sqrt(min(dot2(ca), dot2(cb)));
  }

  vec3 pos;
  float h, r1, r2;
};

struct capped_cone2_t {
  float sd(vec3 p) const noexcept {
    p -= pos;

    float rba = rb - ra;
    float baba = dot2(b - a);
    float papa = dot2(p - a);
    float paba = dot(p - a, b - a) / baba;

    float x = sqrt(papa - paba * paba * baba);

    float cax = max(0.f, x - (paba < .5f ? ra : rb));
    float cay = abs(paba - .5f) - .5f;

    float k = rba * rba + baba;
    float f = clamp((rba * (x - ra) + paba * baba) / k, 0.f, 1.f);

    float cbx = x - ra - f * rba;
    float cby = paba - f;

    float s = cbx < 0 & cay < 0 ? -1.f : 1.f;
    return s * sqrt(min(cax * cax + cay * cay * baba, 
      cbx * cbx + cby * cby * baba));
  }

  vec3 pos;
  vec3 a, b;
  float ra, rb;
};

struct solid_angle_t {  
  float sd(vec3 p) const noexcept {
    p -= pos;
    vec2 p2(length(p.xz), p.y);
    float l = length(p2) - ra;
    float m = length(p2 - c * clamp(dot(p2, c), 0.f, ra));
    return max(l, m * sign(c.y * p2.x - c.x * p2.y));
  }

  vec3 pos;
  vec2 c;
  float ra;
};

struct octahedron_t {
  float sd(vec3 p) const noexcept {
    p -= pos;

    p = abs(p);
    return (p.x + p.y + p.z - s) * 0.57735027f;
  }

  vec3 pos;
  float s;
};

struct pyramid_t {
  float sd(vec3 p) const noexcept {
    p -= pos;

    float m2 = sq(h) + .25f;

    // symmetry
    p.xz = abs(p.xz);
    p.xz = p.z > p.x ? p.zx : p.xz;
    p.xz -= .5f;

    // project into face plane (2D)
    vec3 q(p.z, h * p.y - .5f * p.x, h * p.x + .5f * p.y);
    float s = max(-q.x, 0.f);
    float t = clamp((q.y - .5f * p.z) / (m2 + .25f), 0.f, 1.f);

    float a = m2 * (q.x + s) * (q.x + s) + q.y * q.y;
    float b = m2 * (q.x + .5f * t) * (q.x + .5f * t) + sq(q.y - m2 * t);
    float d2 = min(q.y, -q.x * m2 - q.y * .5f) > 0 ? 0.f : min(a, b);

    // recover 3D and scale, and add sign
    return sqrt((d2 + sq(q.z)) / m2) * sign(max(q.z, -p.y));
  }

  vec3 pos;
  float h;
};

struct rhombus_t {
  float sd(vec3 p) const noexcept {
    p -= pos;

    p = abs(p);
    vec2 b(la, lb);
    float f = clamp((ndot(b, b - 2 * p.xz)) / dot2(b), -1.f, 1.f);
    vec2 q(
      length(p.xz - .5f * b * vec2(1 - f, 1 + f)) * 
        sign(p.x * b.y + p.z * b.x - b.x * b.y) - ra,
      p.y - h
    );

    return min(max(q.x, q.y), 0.f) + length(max(q, 0.f));
  }

  vec3 pos;
  float la, lb, h, ra;
};

template<typename sdf_t>
struct shape_t {
  sdf_t sdf;

  [[.imgui::range_float { 0, 2 * M_PIf32 }]]
  float material;
};

////////////////////////////////////////////////////////////////////////////////
// Load an object from json.

inline void check(bool valid, const std::string& name, 
  const std::string error) {

  if(!valid) {
    fprintf(stderr, "%s: %s\n", name.c_str(), error.c_str());
    exit(1);
  }
}

// Load a class object consisting of class objects, vectors and scalars from
// a JSON.
template<typename obj_t>
obj_t load_from_json(std::string name, nlohmann::json& j) {
  obj_t obj { };

  if(j.is_null()) {
    fprintf(stderr, "no JSON item for %s\n", name.c_str());
    exit(1);
  }

  if constexpr(std::is_class_v<obj_t>) {
    // Read any class type.
    check(j.is_object(), name, "expected object type");
    @meta for(int i = 0; i < @member_count(obj_t); ++i)
      obj.@member_value(i) = load_from_json<@member_type(obj_t, i)>(
        name + "." + @member_name(obj_t, i),
        j[@member_name(obj_t, i)]
      );

  } else if constexpr(__is_vector(obj_t)) {
    static_assert(std::is_same_v<float, __underlying_type(obj_t)>);
    constexpr int size = __vector_size(obj_t);

    check(j.is_array(), name, "expected array type");
    check(j.size() == size, name, 
      "expected " + std::to_string(size) + " array elements");

    for(int i = 0; i < size; ++i) {
      obj[i] = load_from_json<__underlying_type(obj_t)>(
        name + "[" + std::to_string(i) + "]",
        j[i]
      );
    }

  } else {
    static_assert(std::is_integral_v<obj_t> || std::is_floating_point_v<obj_t>);
    check(j.is_number(), name, "expected number type");
    obj = j;
  }
  return obj;
}

////////////////////////////////////////////////////////////////////////////////

@meta nlohmann::json scene_json;

template<typename scene_t, bool inline_scene = false>
struct [[
  .imgui::title=@attribute(scene_t, imgui::title),
  .imgui::url="https://www.shadertoy.com/view/Xds3zN"
]] raymarch_prims_t {

  vec2 opU(vec2 d1, vec2 d2) const noexcept {
    return d1.x < d2.x ? d1 : d2;
  }

  vec2 map(vec3 pos) const noexcept {
    vec2 res(1e10, 0);

    if constexpr(inline_scene) {
      // Create a scene object that's an automatic variable. It's not a 
      // member of this class so won't be bound to the UBO.
      scene_t scene;
      @meta for(int i = 0; i < @member_count(scene_t); ++i) {
        res = opU(res, vec2(
          scene.@member_value(i).sdf.sd(pos), 
          scene.@member_value(i).material / 2 + 1.5f
        ));
      }

    } else {
      // Raymarch on the scene member object.
      @meta for(int i = 0; i < @member_count(scene_t); ++i) {
        res = opU(res, vec2(
          scene.@member_value(i).sdf.sd(pos), 
          scene.@member_value(i).material / 2 + 1.5f
        ));
      }
    }
    return res;
  }

  // http://iquilezles.org/www/articles/boxfunctions/boxfunctions.htm
  vec2 iBox(vec3 ro, vec3 rd, vec3 rad) const noexcept {
    vec3 m = 1 / rd;
    vec3 n = m * ro;
    vec3 k = abs(m) * rad;
    vec3 t1 = -n - k;
    vec3 t2 = -n + k;
    return vec2(
      max(max(t1.x, t1.y), t1.z),
      min(min(t2.x, t2.y), t2.z)
    );
  }

  float calcAO(vec3 pos, vec3 nor) const noexcept {
    float occ = 0.0;
    float sca = 1.0;
    for(int i = 0; i < 5; ++i) {
      float h = 0.01f + 0.12f * i / 4;
      float d = map(pos + h * nor).x;
      occ += (h - d) * sca;
      sca *= 0.95;
      if(occ > 0.35) 
        break;
    }
    return clamp(1 - 3 * occ, 0.f, 1.f) * (0.5f + 0.5f * nor.y);
  }

  // http://iquilezles.org/www/articles/checkerfiltering/checkerfiltering.htm
  float checkersGradBox(vec2 p, vec2 dpdx, vec2 dpdy) const noexcept {
    // filter kernel
    vec2 w = abs(dpdx) + abs(dpdy) + 0.001f;
    // analytical integral (box filter)
    vec2 i = 2 * (
      abs(fract((p - 0.5f * w) * 0.5f) - 0.5f) -
      abs(fract((p + 0.5f * w) * 0.5f) - 0.5f)
    ) / w;

    // xor pattern
    return 0.5f - 0.5f * i.x * i.y;                  
  }

  // http://iquilezles.org/www/articles/rmshadows/rmshadows.htm
  float calcSoftshadow(vec3 ro, vec3 rd, float tmin, float tmax) const noexcept {
    // bounding volume
    float tp = (0.8f - ro.y) / rd.y; 
    if(tp > 0) tmax = min(tmax, tp);

    float res = 1.0;
    float t = tmin;
    for(int i = 0; i < 24; ++i) {
      float h = map(ro + rd * t).x;
      float s = clamp(8 * h / t, 0.f, 1.f);
      res = min(res, s * s * (3 - 2 * s));
      t += clamp(h, 0.02f, 0.2f);
      if(res < 0.004f | t > tmax) 
        break;
    }
    return clamp(res, 0.f, 1.f);
  }

  vec3 calcNormal(vec3 pos) const noexcept {
    vec2 e = vec2(1, -1) * 0.5773f * 0.0005f;
    return normalize(
      e.xyy * map(pos + e.xyy).x + 
      e.yyx * map(pos + e.yyx).x + 
      e.yxy * map(pos + e.yxy).x + 
      e.xxx * map(pos + e.xxx).x
    );
  }

  vec2 raycast(vec3 ro, vec3 rd) const noexcept {
    vec2 res(-1);
    float tmin = 1;
    float tmax = 20;

    // raytrace floor plane
    float tp1 = (0 - ro.y) / rd.y;
    if(tp1 > 0) {
      tmax = min(tmax, tp1);
      res.x = tp1;
      res.y = 1;
    }

    // raymarch primitives
   // vec2 tb = iBox(ro - vec3(0, 0.4, -.5), rd, vec3(10, 0.41, 10.0));
   // if(tb.x < tb.y & tb.y > 0 & tb.x < tmax) {
    //  tmin = max(tb.x, tmin);
    //  tmax = max(tb.y, tmax);

      float t = tmin;
      // Use & as workaround for sturcture CFG bug.
      for(int i = 0; i < 70 & t < tmax; ++i) {
        vec2 h = map(ro + rd * t);
        if(abs(h.x) < .0001f * t) {
          res = vec2(t, h.y);
          break;
        }
        t += h.x;
      }
   // }
    return res;
  }

  vec3 render(vec3 ro, vec3 rd, vec3 rdx, vec3 rdy) const noexcept {
    // background
    vec3 col = vec3(.7, .7, .9) - max(rd.y, 0.f) * .3f;

    // raycast scene
    vec2 res = raycast(ro, rd);
    float t = res.x;
    float m = res.y;
    if(m > -.5f) {
      vec3 pos = ro + t * rd;
      vec3 nor = m < 1.5f ? vec3(0, 1, 0) : calcNormal(pos);
      vec3 ref = reflect(rd, nor);

      // material
      col = 0.2f + 0.2f * sin(2 * m + vec3(0, 1, 2));
      float ks = 1;

      if(m < 1.5f) {
        // project pixel footprint into the plane
        vec3 dpdx = ro.y * (rd / rd.y - rdx / rdx.y);
        vec3 dpdy = ro.y * (rd / rd.y - rdy / rdy.y);

        float f = checkersGradBox(3 * pos.xz, 3 * dpdx.xz, 3 * dpdy.xz);
        col = .15f + f * vec3(0.05);
        ks = .4;
      }

      // lighting
      float occ = calcAO(pos, nor);

      vec3 lin { };

      // sun
      {
        vec3 lig = normalize(vec3(-.5, .4, -.6));
        vec3 hal = normalize(lig - rd);
        float dif = clamp(dot(nor, lig), 0.f, 1.f);

        dif *= calcSoftshadow(pos, lig, 0.02, 2.5);
        float spe = pow(clamp(dot(nor, hal), 0.f, 1.f), 16.f);
        spe *= dif;
        spe *= 0.04f + 0.96f * pow(clamp(1 - dot(hal, lig), 0.f, 1.f), 5.f);
        lin += col * 2.2f * dif * vec3(1.3, 1, 0.7);
        lin +=       5.0f * spe * vec3(1.3, 1, 0.7) * ks;
      }

      // sky
      {
        float dif = sqrt(clamp(.5f + .5f * nor.y, 0.f, 1.f));
        dif *= occ;
        float spe = smoothstep(-.2f, .2f, ref.y);
        spe *= dif;
        spe *= 0.04f + 0.96f * pow(clamp(1 + dot(nor, rd), 0.f, 1.f), 5.f);
        spe *= calcSoftshadow(pos, ref, .02, 2.5);
        lin += col * 0.6f * dif * vec3(0.4, 0.6, 1.15);
        lin +=       2.0f * spe * vec3(0.4, 0.6, 1.30) * ks;
      }

      // back
      {
        float dif = clamp(dot(nor, normalize(vec3(0.5, 0.0, 0.6))), 0.f, 1.f) *
          clamp(1 - pos.y, 0.f, 1.f);
        dif *= occ;
        lin += col * 0.55f * dif * vec3(.25, .25, .25);
      }

      // sss
      {
        float dif = pow(clamp(1 + dot(nor, rd), 0.f, 1.f), 2.f);
        dif *= occ;
        lin += col * .25f * dif * vec3(1);
      }

      col = lin;
      col = mix(col, vec3(0.7, 0.7, 0.9), 1 - exp(-.0001f * t * t * t));
    }

    return vec3(clamp(col, 0.f, 1.f));
  }

  mat3 setCamera(vec3 ro, vec3 ta, float cr) const noexcept {
    vec3 cw = normalize(ta - ro);
    vec3 cp = vec3(sin(cr), cos(cr), 0);
    vec3 cu = normalize( cross(cw,cp) );
    vec3 cv =          ( cross(cu,cw) );
    return mat3( cu, cv, cw );
  }


  vec4 render(vec2 frag_coord, shadertoy_uniforms_t u) {
    vec2 mo = u.mouse.xy / u.resolution.xy;
    float time = 32 + u.time * 1.5f;

    // Camera.
    vec3 ta(.5, -.5, -.6);
    vec3 ro = ta + vec3(
      4.5f * cos(.1f * time + 7 * mo.x),
      1.3f + 2 * mo.y,
      4.5f * sin(.1f * time + 7 * mo.x)
    );
    ro = normalize(ro) * distance;
    mat3 ca = setCamera(ro, ta, 0);

    vec2 p = (2 * frag_coord - u.resolution.xy) / u.resolution.y;

    // ray direction.
    vec3 rd = ca * normalize(vec3(p, 2.5));

    // ray differentials.
    vec2 px = (2 * (frag_coord + vec2(1, 0)) - u.resolution.xy) / u.resolution.y;
    vec2 py = (2 * (frag_coord + vec2(0, 1)) - u.resolution.xy) / u.resolution.y;
    vec3 rdx = ca * normalize(vec3(px, 2.5f));
    vec3 rdy = ca * normalize(vec3(py, 2.5f));

    // render
    vec3 col = render(ro, rd, rdx, rdy);
    col = pow(col, vec3(0.4545));

    return vec4(col, 1);
  }

  float distance = 5;

  @meta if(!inline_scene)
    scene_t scene;
};

struct [[
  .imgui::title="C++-defined objects"
]] basic_scene_t {
  shape_t<sphere_t> sphere = { vec3(-2.0, 0.25, 0.0), .25, 26.9 };

  // Row 0.
  shape_t<torus_t>        torus          = { vec3( 0, 0.30,  1), vec2(.25, .05), 16.9f };
  shape_t<bounding_box_t> bounding_box   = { vec3( 0, 0.25,  0), vec3(.3, .25, .2), .025f, 25.0f };
  shape_t<cone_t>         cone           = { vec3( 0, 0.45, -1), vec2(.6, .8), .45f, 55.f };
  shape_t<capped_cone_t>  capped_cone    = { vec3( 0, 0.25, -2), .25f, .25f, .1f, 13.67f };
  shape_t<solid_angle_t>  solid_angle    = { vec3( 0, 0.00, -3), vec2(3, 4) / 5, 0.4f, 49.13f };
   
  // Row 1.   
  shape_t<capped_torus_t> capped_torus   = { vec3( 1, 0.30,  1), vec2(.866025, -.5), .25f, .05f, 8.5f };
  shape_t<box_t>          box            = { vec3( 1, 0.25,  0), vec3(.3, .25, .1), 3.0f };
  shape_t<capsule_t>      capsule        = { vec3( 1, 0.00, -1), vec3(-.1, .1, -.1), vec3(0.2, .4, .2), .1f, 31.9f };
  shape_t<cylinder_t>     cylinder       = { vec3( 1, 0.25, -2), vec2(.15, .25), 8.0f };
  shape_t<hex_prism_t>    hex_prism      = { vec3( 1, 0.20, -3), vec2(.2, .05), 18.4f };
   
  // Row 2.  
  shape_t<pyramid_t>      pyramid        = { vec3(-1, -0.6, -3), 1.f, 13.56f };
  shape_t<octahedron_t>   octahedron     = { vec3(-1, 0.35, -2), .35f, 23.56f };
  shape_t<tri_prism_t>    tri_prism      = { vec3(-1, 0.15, -1), vec2(.3, .05), 43.5f };
  shape_t<ellipsoid_t>    ellipsoid      = { vec3(-1, 0.25,  0), vec3(.2, .25, .05), 43.17f };
  shape_t<rhombus_t>      rhombus        = { vec3(-1, 0.34,  1), .15f, .25f, .04f, .08f, 17.0f };

  // Row 3.
  shape_t<octagon_prism_t> octagon_prism = { vec3( 2, 0.20, -3), 0.2f, 0.05f, 51.8f };
  shape_t<cylinder2_t>     cylinder2     = { vec3( 2, 0.15, -2), vec3(.1, -.1, 0), vec3(-.2, .35, .1), .08f, 32.1f };
  shape_t<capped_cone2_t>  capped_cone2  = { vec3( 2, 0.10, -1), vec3(.1, 0, 0), vec3(-.2, .4, .1), .15f, .05f, 46.1f };
  shape_t<round_cone_t>    round_cone    = { vec3( 2, 0.15,  1), .2f, .1f, .3f, 37.0f };
  shape_t<round_cone2_t>   round_cone2   = { vec3( 2, 0.15,  0), vec3(.1, 0, 0), vec3(-.1, .35, .1), .15f, .05f, 51.7f };
};

template<int scene_index>
struct [[
  .imgui::title=@string(scene_json[scene_index]["name"])
]] json_scene_t { 

  // Declare the data members.
  @meta for(auto& object : scene_json[scene_index]["objects"])
    shape_t<@type_id(object["type"])> @(object["name"]);

  json_scene_t() {
    // Initialize each data member from its json.
    @meta for(int i = 0; i < @member_count(json_scene_t); ++i) {
      // Set the sdf subobject.
      this->@member_value(i).sdf = 
        (@meta load_from_json<decltype(this->@member_value(i).sdf)>(
          @member_name(json_scene_t, i),
          scene_json[scene_index]["objects"][i]
        ));

      // Set the material.
      this->@member_value(i).material = 
        scene_json[scene_index]["objects"][i]["material"];
    }
  }
};

////////////////////////////////////////////////////////////////////////////////

@meta+ {
  std::ifstream i("scene.json");
  if(!i.is_open()) {
    fprintf(stderr, "cannot open scene file scene.json\n");
    exit(1);
  }

  i>> scene_json;
  scene_json = scene_json["scenes"];

  printf("scene.json loaded %zu scenes\n", scene_json.size());
}

enum typename class shader_program_t {
  raymarch_prims_t<basic_scene_t>;

  @meta for(int i = 0; i < scene_json.size(); ++i) {
    raymarch_prims_t<json_scene_t<i>, false>;
    raymarch_prims_t<json_scene_t<i>, true>;
  }
};


////////////////////////////////////////////////////////////////////////////////

struct program_base_t {
  // Return true if any parameter has changed.
  virtual bool configure(bool update_ubo) = 0;

  // Evaluate the shader with the CPU at this coordinate.
  virtual vec4 eval(vec2 coord, shadertoy_uniforms_t u, 
    bool signal = false) = 0;

  GLuint program;
  GLuint ubo;
};

template<typename shader_t>
struct program_t : program_base_t {
  // Keep an instance of the shader parameters in memory to drive ImGui.
  shader_t shader;

  program_t();
  bool configure(bool update_ubo) override;
  vec4 eval(vec2 coord, shadertoy_uniforms_t u, bool signal) override;
};

template<typename shader_t>
program_t<shader_t>::program_t() {
  // Create vertex and fragment shader handles.
  GLuint vs = glCreateShader(GL_VERTEX_SHADER);
  GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
  GLuint shaders[] { vs, fs };

  // Associate shader handlers with the translation unit's SPIRV data.
  glShaderBinary(2, shaders, GL_SHADER_BINARY_FORMAT_SPIR_V_ARB, 
    __spirv_data, __spirv_size);
  glSpecializeShader(vs, @spirv(vert_main), 0, nullptr, nullptr);
  glSpecializeShader(fs, @spirv(frag_main<shader_t>), 0, nullptr, nullptr);

  // Link the shaders into a program.
  program = glCreateProgram();
  glAttachShader(program, vs);
  glAttachShader(program, fs);
  glLinkProgram(program);

  // Create the UBO.
  glCreateBuffers(1, &ubo);
  glNamedBufferStorage(ubo, sizeof(shader_t), nullptr, 
    GL_DYNAMIC_STORAGE_BIT);
}

template<typename shader_t>
bool program_t<shader_t>::configure(bool update_ubo) {
  bool changed = render_imgui(shader);
  if(update_ubo)
    glNamedBufferSubData(ubo, 0, sizeof(shader_t), &shader);

  return changed;
}

template<typename shader_t>
vec4 program_t<shader_t>::eval(vec2 coord, shadertoy_uniforms_t u, 
  bool signal) {

  if(signal)
    raise(SIGINT);

  return shader.render(coord, u);
}

////////////////////////////////////////////////////////////////////////////////

struct software_fbo_t {
  software_fbo_t(int width, int height);
  ~software_fbo_t();

  // Maintain an FBO and texture for running the shader program on the CPU.
  // We'll then upload it to this texture and glNamedFramebufferBlit it.
  GLuint texture;
  GLuint fbo;
  int width, height;

  std::vector<uint32_t> data;

  void update();
  void blit(int width2, int height2);
  void set_block(vec4 color, int x, int y, int sx, int sy);
};

software_fbo_t::software_fbo_t(int width, int height) :
  width(width), height(height) {

  glCreateTextures(GL_TEXTURE_2D, 1, &texture);
  glCreateFramebuffers(1, &fbo);

  glTextureStorage2D(texture, 1, GL_RGBA8, width, height);
  glNamedFramebufferTexture(fbo, GL_COLOR_ATTACHMENT0, texture, 0);

  data.resize(width * height);
}

software_fbo_t::~software_fbo_t() {
  glDeleteFramebuffers(1, &fbo);
  glDeleteTextures(1, &texture);
}

void software_fbo_t::update() {
  // Upload the texture data.
  glTextureSubImage2D(texture, 0, 0, 0, width, height, GL_RGBA, 
    GL_UNSIGNED_BYTE, data.data());
}

void software_fbo_t::blit(int width2, int height2) {
  // Get the attached renderbuffer.
  GLint fbo_dest;
  glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &fbo_dest);

  // Blit the texture to the renderbuffer.
  glBlitNamedFramebuffer(fbo, fbo_dest, 0, 0, width2, height2,
    0, 0, width2, height2, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void software_fbo_t::set_block(vec4 color, int x, int y, int sx, int sy) {
  // Convert the color to RGBA8.
  ivec4 color2 = clamp((ivec4)(255.f * color + 1.f / 512), 0, 255);
  uint32_t packed = color2.r | (color2.g<< 8) | (color2.b<< 16) | 0xff000000;

  int index = width * y + x;
  for(int row = 0; row < sy; ++row) {
    for(int col = 0; col < sx; ++col)
      data[index + col] = packed;
    index += width;
  }
}

struct cpu_compute_t {
  cpu_compute_t(int num_threads, int width, int height);
  ~cpu_compute_t();

  void pool_execute();
  void thread_execute(int tid);
  bool pixel_execute(int x, int y, int sx, int sy);
  bool is_complete() const;
  void join();
  void update();

  static void thread_entry(cpu_compute_t* compute, int tid);

  std::vector<std::thread> threads;
  std::atomic<int> complete_count;
  std::atomic<bool> okay;

  int width, height;
  shadertoy_uniforms_t uniforms;
  int num_levels = 0;
  bool interlace = false;

  program_base_t* program;
  std::unique_ptr<software_fbo_t> fbo;
};

cpu_compute_t::cpu_compute_t(int num_threads, int width, int height) :
  width(width), height(height) {

  complete_count = num_threads;
  threads.resize(num_threads);
  okay = false;

  int width2 = (width + 7) & ~7;
  int height2 = (height + 7) & ~7;

  fbo = std::make_unique<software_fbo_t>(width2, height2);
}

cpu_compute_t::~cpu_compute_t() {
  join();
}

void cpu_compute_t::pool_execute() {
  okay = true;
  complete_count = 0;
  for(int tid = 0; tid < threads.size(); ++tid)
    threads[tid] = std::thread(thread_entry, this, tid);
}

void cpu_compute_t::thread_execute(int tid) {
  adam7_t adam7 { (width + 7) / 8, (height + 7) / 8 };

  auto f = [&](int x, int y, int sx, int sy) {
    return pixel_execute(x, y, sx, sy);
  };
  adam7.process(tid, threads.size(), num_levels, interlace, f);

  ++complete_count;
}

bool cpu_compute_t::pixel_execute(int x, int y, int sx, int sy) {
  // Immediately break if any setting has changed.
  if(!okay)
    return false;

  // Adjust to get the center of the pixel.
  float x2 = x + .5f;
  float y2 = y + .5f;

  vec4 color = program->eval(vec2(x2, y2), uniforms);
  fbo->set_block(color, x, y, sx, sy);

  return true;
}

bool cpu_compute_t::is_complete() const {
  return threads.size() == complete_count;
}

void cpu_compute_t::join() {
  okay = false;
  for(std::thread& t : threads)
    if(t.joinable())
      t.join();
}

void cpu_compute_t::thread_entry(cpu_compute_t* compute, int tid) {
  compute->thread_execute(tid);
}


////////////////////////////////////////////////////////////////////////////////

struct app_t {
  GLFWwindow* window = nullptr;
  GLuint vao = 0;
  GLuint array_buffer = 0;
  GLuint uniform_buffer = 0;

  // Indicate the current program being rendered.
  shader_program_t active_shader = { };

  shadertoy_uniforms_t uniforms;

  // Keep an instance of the current program.
  std::unique_ptr<program_base_t> program;

  float speed = 1;
  double time = 0;
  bool debug_on_click = false;
  bool render_cpu = false;
  bool interlace = false;
  bool asynchronous = true;
  int num_threads = 1;
  int num_levels = 1;
  std::unique_ptr<software_fbo_t> software_fbo;
  std::unique_ptr<cpu_compute_t> cpu_compute;

  app_t();
  void loop();
  void button_callback(int button, int action, int mods);

  bool configure();
  void set_active_shader(shader_program_t shader);

  void display_gpu();
  void display_cpu();

  // Fully compute one thread's contribution to the shader program.
  bool compute_cpu(int tid, int num_threads, int block_size);

  void framebuffer_callback(int width, int height);
  static void _framebuffer_callback(GLFWwindow* window, int width, int height);
  static void _button_callback(GLFWwindow* window, int button, int action, 
    int mods);

  static void _debug_callback(GLenum source, GLenum type, GLuint id, 
    GLenum severity, GLsizei length, const GLchar* message, 
    const void* user_param);
};

void app_t::framebuffer_callback(int width, int height) {
  glViewport(0, 0, width, height);
  cpu_compute.reset();
}

void app_t::_framebuffer_callback(GLFWwindow* window, int width, int height) {
  app_t* app = static_cast<app_t*>(glfwGetWindowUserPointer(window));
  app->framebuffer_callback(width, height);
}

void app_t::_button_callback(GLFWwindow* window, int button, int action,
  int mods) {
  app_t* app = static_cast<app_t*>(glfwGetWindowUserPointer(window));
  app->button_callback(button, action, mods);
}

void app_t::_debug_callback(GLenum source, GLenum type, GLuint id, 
  GLenum severity, GLsizei length, const GLchar* message, 
  const void* user_param) {

  if(GL_DEBUG_SEVERITY_HIGH == severity ||
    GL_DEBUG_SEVERITY_MEDIUM == severity)
    printf("OpenGL: %s\n", message);

  if(GL_DEBUG_SEVERITY_HIGH == severity)
    exit(1);
}

app_t::app_t() {
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6); 
  glfwWindowHint(GLFW_STENCIL_BITS, 8);
  glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);

  window = glfwCreateWindow(1280, 720, "Circle does Shadertoy", 
    NULL, NULL);
  if(!window) {
    printf("Cannot create GLFW window\n");
    exit(1);
  }

  glEnable(GL_DEBUG_OUTPUT);
  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
  glDebugMessageCallback(_debug_callback, this);

  glfwSetWindowUserPointer(window, this);
  glfwSetFramebufferSizeCallback(window, _framebuffer_callback);
  glfwSetMouseButtonCallback(window, _button_callback);

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  // Create an ImGui context.
  ImGui::CreateContext();
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 460");

  const float vertices[][2] { { -1, 1 }, { 1, 1 }, { -1, -1 }, { 1, -1 } };

  // Load into an array object.
  glCreateBuffers(1, &array_buffer);
  glNamedBufferStorage(array_buffer, sizeof(vertices), vertices, 0);

  // Declare a vertex array object and bind the array buffer.
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, array_buffer);

  // Bind to slot 0
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);

  // Allocate a shadertoy_uniform_t buffer.
  glCreateBuffers(1, &uniform_buffer);
  glNamedBufferStorage(uniform_buffer, sizeof(shadertoy_uniforms_t), nullptr,
    GL_DYNAMIC_STORAGE_BIT);

  // Choose the default shader.
  set_active_shader(shader_program_t { });

  uniforms.time = 0;
  uniforms.mouse = vec4(.5, .5, .5, .5);
}

void app_t::loop() {

  while(!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    // Set the shadertoy uniforms.
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    glViewport(0, 0, width, height);

    // Let ImGui have the mouse if the cursor is above its windows.
    ImGuiIO& io = ImGui::GetIO();
    bool has_mouse = !io.WantCaptureMouse &&
      GLFW_PRESS == glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    if(has_mouse) {
      // Update mouse constant if pressed.
      double xpos, ypos;
      glfwGetCursorPos(window, &xpos, &ypos);

      float x = floor(xpos) + .5f;
      float y = height - floor(ypos) - .5f;

      uniforms.mouse.x = x;
      uniforms.mouse.y = y;
      if(uniforms.mouse.z < 0) {
        // The button was up the previous frame.
        uniforms.mouse.z = x;
        uniforms.mouse.w = y;
      }

    } else if(uniforms.mouse.z >= 0) {
      // The button was down the previous frame.
      // uniforms.mouse.zw *= -uniforms.mouse.xy;
      uniforms.mouse.z = -uniforms.mouse.x;
      uniforms.mouse.w = -uniforms.mouse.y;
    }

    uniforms.resolution = vec2(width, height);
    double new_time = glfwGetTime();
    uniforms.time += speed * (new_time - time);
    time = new_time; 

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Configure the selected program...
    bool changed = configure();

    // Save the ImGui frame.
    ImGui::Render();

    if(render_cpu) {
      if(!cpu_compute || cpu_compute->threads.size() != num_threads) {
        // Create a new thread pool.
        cpu_compute = std::make_unique<cpu_compute_t>(num_threads, width,
          height);
        cpu_compute->program = program.get();
      }

      if(changed || cpu_compute->is_complete()) {
        // Either the shader settings have changed or we finished the frame.
        // Either way, update the settings and start again.
        cpu_compute->join();

        cpu_compute->fbo->update();
        cpu_compute->fbo->blit(cpu_compute->width, cpu_compute->height);

        cpu_compute->num_levels = num_levels;
        cpu_compute->interlace = interlace;
        cpu_compute->uniforms = uniforms;
        cpu_compute->pool_execute();

      } else if(asynchronous)
        cpu_compute->fbo->update();

      // Render what we have so far.
      cpu_compute->fbo->blit(cpu_compute->width, cpu_compute->height);

    } else {
      if(cpu_compute) {
        cpu_compute.reset();
      }

      // Bind the builtin shadertoy uniforms.
      glNamedBufferSubData(uniform_buffer, 0, sizeof(uniforms), &uniforms);
      glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniform_buffer);

      // Bind and execute the input program.
      glUseProgram(program->program);
      glBindBufferBase(GL_UNIFORM_BUFFER, 1, program->ubo);

      glBindVertexArray(vao);

      glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    // Render the ImGui frame over the application.
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Swap buffers.
    glfwSwapBuffers(window);
  }
}

void app_t::button_callback(int button, int action, int mods) {
  ImGuiIO& io = ImGui::GetIO();
  if(!io.WantCaptureMouse && GLFW_PRESS == action && 
    GLFW_MOUSE_BUTTON_LEFT == button) {

    // On the CPU, run the shader once on the click-on point.
    double x, y;
    glfwGetCursorPos(window, &x, &y);

    vec2 coord((float)x + .5f, uniforms.resolution.y - (float)y - .5f);
    shadertoy_uniforms_t u = uniforms;
    u.mouse = vec4(coord, coord);

    vec4 color = program->eval(coord, u, debug_on_click);

    printf("(%f, %f) -> (%f %f %f %f)\n", coord.x, coord.y,
      color.r, color.g, color.b, color.a);
  }
}

bool app_t::configure() {
  // Start by selecting the current program.
  // Produce a combo box with all shader options.
  ImGui::Begin("Shader parameters");

  ImGui::DragFloat("Time", &uniforms.time, .1);
  ImGui::SliderFloat("Speed", &speed, 0, 5);

  ImGui::Checkbox("Debug on click (run from GDB)", &debug_on_click);

  ImGui::Checkbox("Render on CPU", &render_cpu);

  bool changed = false;
  if(render_cpu) {
    changed |= ImGui::SliderInt("Thread pool size", &num_threads, 1, 32);
    changed |= ImGui::SliderInt("Detail levels", &num_levels, 1, 7);
    changed |= ImGui::Checkbox("Interlacing", &interlace);
    ImGui::Checkbox("Asynchronous", &asynchronous);
  }

  int current = (int)active_shader;
  const char* items[] { 
    @attribute(@enum_types(shader_program_t), imgui::title)... 
  }; 
  changed |= ImGui::Combo("Active shader", &current, items, items.length);

  if(current != (int)active_shader) {
    cpu_compute.reset();
    set_active_shader((shader_program_t)current);
  }

  program->configure(!render_cpu);
  ImGui::End();

  return changed;
}

void app_t::set_active_shader(shader_program_t shader) {
  // Generate a switch construct the requested shader program.
  switch(shader) {
    @meta for enum(shader_program_t e : shader_program_t) {
      case e: 
        program = std::make_unique<program_t<@enum_type(e)> >();
        glfwSetWindowTitle(window, @attribute(@enum_type(e), imgui::title));
        break;
    }
  }
  active_shader = shader;
}


////////////////////////////////////////////////////////////////////////////////


int main() {
  glfwInit();
  gl3wInit();
  
  app_t app;
  app.loop();

  return 0;
}
