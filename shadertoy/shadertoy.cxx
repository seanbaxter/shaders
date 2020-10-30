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

  using title    [[attribute]] = const char*;
  using url      [[attribute]] = const char*;
}

// Return true if any option has changed.
template<typename options_t>
bool render_imgui(options_t& options, const char* child_name) {

  ImGui::BeginChild(child_name);

  using namespace imgui;
  if constexpr(@has_attribute(options_t, url))
    ImGui::Text(@attribute(options_t, url));

  bool changed = false;
  @meta for(int i = 0; i < @member_count(options_t); ++i) {{
    typedef @member_type(options_t, i) type_t;
    const char* name = @member_name(options_t, i);
    auto& value = @member_value(options, i);

    if constexpr(@member_has_attribute(options_t, i, color4)) {
      changed |= ImGui::ColorEdit4(name, &value.x);

    } else if constexpr(@member_has_attribute(options_t, i, color3)) {
      changed |= ImGui::ColorEdit3(name, &value.x);

    } else if constexpr(@member_has_attribute(options_t, i, range_float)) {
      auto minmax = @member_attribute(options_t, i, range_float);
      changed |= ImGui::SliderFloat(name, &value, minmax.min, minmax.max);

    } else if constexpr(@member_has_attribute(options_t, i, range_int)) {
      auto minmax = @member_attribute(options_t, i, range_int);
      changed |= ImGui::SliderInt(name, &value, minmax.min, minmax.max);

    } else if constexpr(std::is_same_v<type_t, bool>) {
      changed |= ImGui::Checkbox(name, &value);

    } else if constexpr(std::is_same_v<type_t, vec4>) {
      changed |= ImGui::DragFloat4(name, &value.x, .1f);

    } else if constexpr(std::is_same_v<type_t, vec3>) {
      changed |= ImGui::DragFloat3(name, &value.x, .1f);

    } else if constexpr(std::is_same_v<type_t, vec2>) {
      changed |= ImGui::DragFloat2(name, &value.x, .1f);
      
    } else if constexpr(std::is_same_v<type_t, float>) {
      changed |= ImGui::DragFloat(name, &value, .1f);
  
    } else if constexpr(std::is_same_v<type_t, int>) {
      changed |= ImGui::DragInt(name, &value);
    }
  }}
  ImGui::EndChild();

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

template<typename shader_t, int I>
[[using spirv: uniform, binding(1 + I)]]
shader_t shader_ubo;

template<typename shader_t, int I>
[[spirv::frag(lower_left)]]
void frag_main() {
  fragColor = shader_ubo<shader_t, I>.render(glfrag_FragCoord.xy, uniforms);
}

////////////////////////////////////////////////////////////////////////////////

inline vec2 rot(vec2 p, vec2 pivot, float a) {
  p -= pivot;
  p = vec2(
    p.x * cos(a) - p.y * sin(a), 
    p.x * sin(a) + p.y * cos(a)
  );
  p += pivot;

  return p;
}

inline vec2 rot(vec2 p, float a) {
  return rot(p, vec2(), a);
}


struct [[
  .imgui::title="Devil Egg",
  .imgui::url="https://www.shadertoy.com/view/3tXXRX"
]] devil_egg_t {

  vec3 hash32(vec2 p) {
    vec3 p3 = fract(vec3(p.xyx) * vec3(.1031, .1030, .0973));
    p3 += dot(p3, p3.yxz+19.19);
    return fract((p3.xxy+p3.yzz)*p3.zyx);
  }

  float opUnion(float d1, float d2) { return min(d1, d2); }
  float opSubtraction(float d1, float d2) { return max(-d1, d2); }

  float sdShape(vec2 p, float r) {
    return length(p + .5f * r) - r;
  }
 
  vec2 egg(vec2 sd, vec2 uv, float t, float r, float a) {
    if(r > 0) {
      vec2 uv2 = rot(uv, a);
      float yolk = sdShape(11 / sqrt(2.f) * uv2, r);
      float white = sdShape(5.2 / sqrt(2.f) * uv2, r);
      sd = vec2(opUnion(sd.x, white), opUnion(sd.y, yolk));
    }
    return sd;
  }
 
  float under(vec2 N, float t) {
    float sz = .001;
    float range = 1.8;
    vec2 p1 = vec2(sin(-1.66666f * t), cos(t)) * range;
    vec2 p2 = vec2(sin(t), sin(1.3333f * t)) * range;
    float b = min(length(p1 - N) - sz, length(p2 - N) - sz);
    return b;
  }

  vec2 smallEggField(vec2 sd, vec2 uv, vec2 uvbig, float t) {
    vec2 uvsmall = mod(uv + vec2(.3 * t, 0), Span) - .5f * Span;
    vec2 uvsmall_q = uv - uvsmall;
    
    // Find the dist to the big egg, quantizing big egg's coords to small 
    // egg coords.
    vec2 sdquant = egg(1e6, uvsmall_q, t, BigSize, under(uvsmall_q, t));
    return egg(sd, uvsmall, t, SmallSize * smoothstep(0.f, .8f, sdquant.x - .5f),
      under(10 * uvbig, t));
  }

  vec3 color(vec2 sd, float fact) {
    vec3 o = 1 - smoothstep(0.f, fact * vec3(.06, .03, .02), sd.x); 
    o += BackgroundColor;
    if(sd.x < 0)
      o -= sd.x * .6f;
    o = clamp(o, 0.f, 1.f);

    vec3 ayolk = 1 - smoothstep(0, fact * vec3(.2, .1, .2), sd.yyy);
    o = mix(o, YolkColor, ayolk);
    if(sd.y < 0)
      o -= sd.y * .1f;

    o = clamp(o, 0.f, 1.f);
    return o;
  }

  vec4 render(vec2 frag_coord, shadertoy_uniforms_t u) {
    constexpr float PI2 = 2 * M_PIf32;
    vec2 uv = frag_coord / u.resolution - .5f;
    vec2 N = uv;
    uv.x *= u.resolution.x / u.resolution.y;
    uv.y -= .1f;
    uv *= Zoom;

    float t = Speed * u.time;

    vec2 uvbig = uv;

    // Big egg.
    vec2 sd = egg(1.0e6, uv, t, BigSize, under(uvbig, t));

    // Small eggs.
    vec2 sdsmall = smallEggField(1.0e6, uv, uvbig, t);
    uv -= Span * .5f;
    sdsmall = smallEggField(sdsmall, uv, uvbig, t);

    vec3 o = mix(color(sd, 2), color(sdsmall, .2f), vec3(step(.1f, sd.x)));

    o = pow(o, .5f);
    o += (hash32(frag_coord + t) - .5f) * FilmGrain;
    o = clamp(o, 0.f, 1.f);
    o *= 1 - length(13 * pow(abs(N), DarkEdge));

    return vec4(o, 1);
  }

  [[.imgui::range_float {  .5,  5 }]] float Zoom = 1;
  [[.imgui::range_float {   0,  3 }]] float Speed = 1;
  [[.imgui::range_float {  .2,  2 }]] float BigSize = .8;
  [[.imgui::range_float { .05,  1 }]] float SmallSize = .09;
  [[.imgui::range_float { .05,  2 }]] float Span = .1;
  [[.imgui::range_float { .01, .5 }]] float FilmGrain = .1;
  [[.imgui::range_float {   1,  5 }]] float DarkEdge = 4;

  [[.imgui::color3]] vec3 YolkColor = vec3(.5, .5, 0);
  [[.imgui::color3]] vec3 BackgroundColor = vec3(.1, .5, .1);
};

struct [[
  .imgui::title="Neon Hypno Bands",
  .imgui::url="https://www.shadertoy.com/view/MdcGW4"]] 
hypno_bands_t {

  float rand(float n){
    return fract(cos(n * 89.42f) * 343.42f);
  }
  float roundx(float x, float p) {
      return floor((x + (p * .5f)) / p) * p;
  }
  float dtoa(float d, float amount) {
      return clamp(1 / (clamp(d, 1 / amount, 1.f) * amount), 0.f, 1.f);
  }
  float sdSegment1D(float uv, float a, float b) {
    return max(max(a - uv, 0.f), uv - b);
  }

  vec4 render(vec2 frag_coord, shadertoy_uniforms_t u) {
    constexpr float PI2 = 2 * M_PIf32;
    vec2 uv = frag_coord / u.resolution - .5f;
    uv.x *= u.resolution.x / u.resolution.y;
    uv *= Zoom;

    float t = u.time * Speed;

    if(Warp) {
      vec2 oldUV = uv;
      uv = pow(abs(uv), sin(t * vec2(WarpX, WarpY)) * .35f + 1.2f) * sign(oldUV);
    }

    float bandRadius = roundx(length(uv), BandSpacing);
    vec3 bandID = vec3(
      rand(bandRadius + 0),
      rand(bandRadius + 1),
      rand(bandRadius + 2)
    );

    float distToLine = sdSegment1D(
      length(uv), 
      bandRadius - (LineSize * .5f), 
      bandRadius + (LineSize * .5f)
    );
    float bandA = dtoa(distToLine, 400.f);// alpha around this band.
    
    float bandSpeed = .1f / max(0.05f, bandRadius);// outside = slower
    float r = -t + PI2 * bandID.x;
    r *= bandSpeed;
    uv *= rot(uv, r);

    float angle = mod(atan2(uv.x, uv.y), PI2);// angle, animated
    float arcLength = bandRadius * angle;
    
    float color = sign(mod(arcLength, 2 * SegmentLength) - SegmentLength);

    return vec4(vec3(bandID * color * bandA), 1);
  }

  [[.imgui::range_float { .5,  8 }]] float Zoom = 1.5;
  [[.imgui::range_float {  0,  5 }]] float Speed = 2;
  [[.imgui::range_float { .1, .5 }]] float BandSpacing = .05;
  [[.imgui::range_float {  0, .5 }]] float LineSize = .008;
  [[.imgui::range_float {  0,  6 }]] float SegmentLength = .5;
  [[.imgui::range_float {  0,  1 }]] float WarpX = .2;
  [[.imgui::range_float {  0,  1 }]] float WarpY = .7;
                                      bool Warp = true;
};


////////////////////////////////////////////////////////////////////////////////
// https://www.shadertoy.com/view/wsVyzw

struct [[
  .imgui::title="Star Amplitude Modulation", 
  .imgui::url="https://www.shadertoy.com/view/wsVyzw"
]] modulation_t {

  // Signed distance to a n-star polygon with external angle en.
  float sdStar(vec2 p, float r, int n, float m) {
    float an = M_PIf32 / n;
    float en = M_PIf32 / m;
    vec2 acs(cos(an), sin(an));
    vec2 ecs(cos(en), sin(en));

    // reduce to first sector.
    float bn = mod(atan2(p.x, p.y), 2 * an) - an;
    p = length(p) * vec2(cos(bn), abs(sin(bn)));

    // line sdf
    p -= r * acs;
    p += ecs * clamp(-dot(p, ecs), 0.0f, r * acs.y / ecs.y);
    return length(p) * sign(p.x);
  }

  float sdShape(vec2 uv, float t) {
    float angle = -t * StarRotationSpeed;
    return sdStar(rot(uv, angle), StarSize, StarPoints, StarWeight);
  }

  vec3 dtoa(float d, vec3 amount) {
    return 1 / clamp(d * amount, 1, amount);
  }

  // https://www.shadertoy.com/view/3t23WG
  // Distance to y(x) = a + b*cos(cx+d)
  float udCos(vec2 p, float a, float b, float c, float d) {
    p = c * (p - vec2(d, a));
    
    // Reduce to principal half cycle.
    p.x = mod(p.x, 2 * M_PIf32);
    if(p.x > M_PIf32)
      p.x = 2 * M_PIf32 - p.x;

    // Fine zero of derivative (minimize distance).
    float xa = 0, xb = 2 * M_PIf32;
    for(int i = 0; i < 7; ++i) { // bisection, 7 bits more or less.
      float  x = .5f * (xa + xb);
      float si = sin(x);
      float co = cos(x);
      float  y = x - p.x + b * c * si * (p.y - b * c * co);
      if(y < 0)
        xa = x;
      else
        xb = x;
    }

    float x = .5f * (xa + xb);
    for(int i = 0; i < 4; ++i) { // Newton-Raphson, 28 bits more or less.
      float si = sin(x);
      float co = cos(x);
      float  f = x - p.x + b * c * (p.y * si - b * c * si * co);
      float df = 1       + b * c * (p.y * co - b * c * (2 * co * co - 1));
      x = x - f / df;
    }

    // Compute distance.
    vec2 q(x, b * c * cos(x));
    return length(p - q) / c;
  }

  vec4 render(vec2 frag_coord, shadertoy_uniforms_t u) {
    vec2 N = frag_coord / u.resolution - .5f;
    vec2 uv = N;
    uv.x *= u.resolution.x / u.resolution.y;

    uv *= Zoom;
    float t = u.time * PhaseSpeed;

    vec2 uvsq = uv;
    float a = sdShape(uv, t);

    float sh = mix(100.f, 1000.f, Sharpness);

    float a2 = 1.5;
    for(int i = -3; i <= 3; ++i) {
      vec2 uvwave(uv.x, uv.y + i * WaveSpacing);
      float b = smoothstep(1.f, -1.f, a) * WaveAmp + WaveAmpOffset;
      a2 = min(a2, udCos(uvwave, 0.f, b, WaveFreq, t));
    }

    vec3 o = dtoa(mix(a2, a-LineWeight + 4, .03f), sh * Tint);
    if(!InvertColors)
      o = 1 - o;

    o *= 1 - dot(N, N * 2);
    return vec4(clamp(o, 0, 1), 1);
  }

  [[.imgui::range_float {   1,  10 }]] float Zoom = 5;
  [[.imgui::range_float {   0,  10 }]] float LineWeight = 4.3;
  [[.imgui::range_int   {   2,  10 }]] int   StarPoints = 5;
  [[.imgui::range_float {  .1,   3 }]] float Sharpness = .2;
  [[.imgui::range_float {  -5,   5 }]] float StarRotationSpeed = -.5;
  [[.imgui::range_float {   0,   5 }]] float StarSize = 2;
  [[.imgui::range_float {   2,  10 }]] float StarWeight = 6;
  [[.imgui::range_float {   0,   4 }]] float WaveSpacing = .8;
  [[.imgui::range_float {   0,   5 }]] float WaveAmp = 2;
  [[.imgui::range_float {   0,  50 }]] float WaveFreq = 25;
  [[.imgui::range_float {  -2,   2 }]] float PhaseSpeed = .33;
  [[.imgui::range_float { -.1,  .1 }]] float WaveAmpOffset = .01;
  [[.imgui::color3]]                     vec3 Tint = vec3(.15, .5, .8);
  bool InvertColors = false; 
};

////////////////////////////////////////////////////////////////////////////////
// Keep up little square

struct [[
  .imgui::title="Keep Up Little Square",
  .imgui::url="https://www.shadertoy.com/view/wlsXD2"
]] keep_up_square_t {

  vec3 hash32(vec2 p) {
    vec3 p3 = fract(vec3(p.xyx) * vec3(.1031, .1030, .0973));
    p3 += dot(p3, p3.yxz + 19.19f);
    return fract((p3.xxy + p3.yzz) * p3.zyx);
  }

  float dtoa(float d, float amount) {
    return 1 / clamp(d * amount, 1.f, amount);
  }

  float sdSquare(vec2 p, vec2 pos, vec2 origin, float a, float s) {
    vec2 d = abs((rot(p - pos, -a)) - origin) - s;
    return min(max(d.x, d.y), 0.f) + length(max(d, 0.f));
  }

  // returns { x:orig, y:height, z:position 0-1 within this cell }
  vec3 cell(float x) {
    float pos = fract(x / XScale);
    float orig = (x / XScale - pos) * XScale;
    float h = hash32(vec2(orig)).r;
    return vec3(orig, h * YScale, pos);
  }

  vec3 scene(vec2 N, vec2 uv, float t, float xpos, float xcam) {
    vec3 sqCellPrev = cell(xpos - XScale);
    
    // Ground.
    vec3 bigCellThis = cell(uv.x);
    vec3 bigCellNext = cell(uv.x + XScale);
    float bigHeightThis = mix(bigCellThis.y, bigCellNext.y, bigCellThis.z);
    float sd = uv.y - bigHeightThis;

    // Walking square; interpolate between positions.
    vec3 sqCellThis = cell(xpos);
    vec3 sqCellNext = cell(xpos + XScale);
    float aThis = atan2(sqCellPrev.y - sqCellThis.y, 
      sqCellPrev.x - sqCellThis.x);
    float aNext = atan2(sqCellThis.y - sqCellNext.y, 
      sqCellThis.x - sqCellNext.x) - M_PIf32 / 2;

    if(aNext > aThis + M_PIf32) aNext -= 2 * M_PIf32;
    if(aNext < aThis - M_PIf32) aNext += 2 * M_PIf32;

    float szThis = distance(sqCellPrev.xy, sqCellThis.xy);
    float szNext = distance(sqCellThis.xy, sqCellNext.xy);

    float param = pow(sqCellNext.z, Sluggishness);
    float asq = mix(aThis,  aNext,  param);
    float sz  = mix(szThis, szNext, param);

    float sdsq = sdSquare(uv, sqCellThis.xy, sz * vec2(-.5f, .5f), 
      asq + M_PIf32, .5f * sz);

    vec3 o { };
    vec2 uvtemp = uv;

    for(int i = 1; i <= 9; ++i) {
      uvtemp.x -= xpos;
      uvtemp *= vec2(2, 1.8);

      uvtemp.y -= .3;
      uvtemp.x += xpos + 1000;

      vec3 cellThis = cell(uvtemp.x);
      vec3 cellNext = cell(uvtemp.x + XScale);
      float heightThis = mix(cellThis.y, cellNext.y, cellThis.z);
      float sd = uvtemp.y - heightThis;
      o = max(o, dtoa(sd, 1000) * .2f / i);

      float amt = 25 + heightThis * 500;
      o = max(o, dtoa(abs(sd) + .01f, amt) * .4f / i);
    }

    o += .8f - uv.y * 1.1f;
    o.g *= o.r;
    o.r *= .6f;
    o.b += N.y;

    // square
    float alphasq = dtoa(sdsq, 1000);
    o = mix(o, SquareColor, alphasq);

    // ground
    float alphagr = dtoa(sd, 300);
    o = mix(o, GroundColor, alphagr);

    // snow
    float alphasn = dtoa(abs(sd + .01f), 25 + bigHeightThis * 500);
    o = mix(o, SnowColor, alphasn);

    return o;
  }

  vec4 render(vec2 frag_coord, shadertoy_uniforms_t u) {
    vec2 N = frag_coord / u.resolution - .5f;
    N.x *= u.resolution.x / u.resolution.y;
    vec2 uv = N;

    float t = u.time * Speed + 100;
    float xpos = t * XSpeed;
    float xcam = sin(9 * t) * CamShakiness;

    uv = vec2(xpos + xcam, .3f) + Zoom * N;

    // Calc scene twice and motion blur.
    vec3 sqCellPrev = cell(xpos - XScale);
    float bounce = abs(sin(sqCellPrev.z * 26)) * 
      pow(1 - sqCellPrev.z, .7f) * Bounce;

    vec3 o1 = scene(N, uv + vec2(0, bounce), t, xpos, xcam);
    vec3 o2 = scene(N, uv                  , t, xpos, xcam);

    vec3 o = .5f * (o1 + o2);
    o.b *= .9;
    o += (hash32(frag_coord + t) - .5f) * FilmGrain;

    o *= 1.1f - dot(N, N);
    o = clamp(o, 0, 1);

    return vec4(o, 1);
  }

  [[.imgui::range_float {  .1,  5 }]] float Zoom = 1.5;
  [[.imgui::range_float {   0,  1 }]] float Speed = .16;
  [[.imgui::range_float {  .1,  1 }]] float XScale = .3;
  [[.imgui::range_float {   0, .5 }]] float YScale = .2;
  [[.imgui::range_float {   0, .1 }]] float Bounce = .01;
  [[.imgui::range_float {  -3,  3 }]] float XSpeed = 1;
  [[.imgui::range_float {   0, .2 }]] float CamShakiness = .1;
  [[.imgui::range_float {   1, 10 }]] float Sluggishness = 7;
  [[.imgui::range_float { .01, .5 }]] float FilmGrain = .15;
  [[.imgui::color3]] vec3 SquareColor = vec3(0.9, 0.1, 0.1);
  [[.imgui::color3]] vec3 GroundColor = vec3(.3, .6, .1);
  [[.imgui::color3]] vec3 SnowColor   = vec3(0.8, 0.9, 1.0);
};

////////////////////////////////////////////////////////////////////////////////

struct [[
  .imgui::title="Metallic Paint Stir",
  .imgui::url="https://www.shadertoy.com/view/3sG3Rd"
]] paint_t {

  vec4 render(vec2 frag_coord, shadertoy_uniforms_t u) {
    constexpr float PI2 = 2 * M_PIf32;
    vec2 N = frag_coord / u.resolution - .5f;
    vec2 uv = N;
    uv.x *= u.resolution.x / u.resolution.y;
    uv *= Zoom;

    float t = u.time * Speed;

    // get radius and angle
    float l = sqrt(length(uv));
    float a = atan2(uv.x, uv.y) + sin(l * PI2) * PI2;

    // distort uv by length, animated wave over time
    float ex = mix(MinPow, MaxPow, .5f * sin(l*PI2+a+t*PI2) + .5f);
    uv = sign(uv)*pow(abs(uv), vec2(ex));
    
    float d = abs(fract(length(uv)-t) - .5f);// dist to ring centers
    float c = 1 / max(((2 - l) * 6) * d, .1f);// dist to grayscale amt
    vec4 o(c);
    vec3 col = vec3(
      // generate correlated colorants 0-1 from length, angle, exponent
      clamp(l * l * l, 0.f, 1.f), 
      .5f * sin(a) + .5f,
      (ex - MinPow) / (MaxPow - MinPow)
    );

    col = 1 - mix(vec3(col.r + col.g + col.b) / 3, col, Saturation);

    o.rgb *= col;
    o *= Fade - l;// fade edges (vignette)

    return o;
  }

  [[.imgui::range_float { .1, 10 }]] float Zoom = 4.3;
  [[.imgui::range_float {  0,  2 }]] float Speed = .2;
  [[.imgui::range_float {  0,  4 }]] float MinPow = .6;
  [[.imgui::range_float {  0,  4 }]] float MaxPow = 2;
  [[.imgui::range_float {  0,  4 }]] float Fade = 1.8;
  [[.imgui::range_float {  0,  1 }]] float Saturation = .5;
};

////////////////////////////////////////////////////////////////////////////////

struct [[
  .imgui::title="Mouse test",
  .imgui::url="None"
]] mouse_test_t {
  vec4 render(vec2 frag_coord, shadertoy_uniforms_t u) {
    vec3 color { };

    // Last down.
    vec3 bg = u.mouse.z >= 0 ? active_color : inactive_color; 
    vec2 mouse = abs(u.mouse.zw);
    if(abs(frag_coord.x - mouse.x) <= outer_width) color = bg;
    if(abs(frag_coord.y - mouse.y) <= outer_width) color = bg;

    // Last up.
    if(abs(frag_coord.x - u.mouse.x) <= drag_width) color += drag_color;
    if(abs(frag_coord.y - u.mouse.y) <= drag_width) color += drag_color;

    // Last down.
    return vec4(color, 1);
  }

  float drag_width = 2;
  [[.imgui::color3]] vec3 drag_color = vec3(1, 0, 0);

  float outer_width = 4;
  [[.imgui::color3]] vec3 active_color = vec3(0, 1, 0);
  [[.imgui::color3]] vec3 inactive_color = vec3(0, 0, 1);
};

enum typename class shader_program_t {
  DevilEgg = devil_egg_t,
  HypnoBands = hypno_bands_t,
  Modulation = modulation_t,
  Square = keep_up_square_t,
  Paint = paint_t,
  MouseTest = mouse_test_t,
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

template<typename shader_t, int I>
struct program_t : program_base_t {
  // Keep an instance of the shader parameters in memory to drive ImGui.
  shader_t shader;

  program_t();
  bool configure(bool update_ubo) override;
  vec4 eval(vec2 coord, shadertoy_uniforms_t u, bool signal) override;
};

template<typename shader_t, int I>
program_t<shader_t, I>::program_t() {
  // Create vertex and fragment shader handles.
  GLuint vs = glCreateShader(GL_VERTEX_SHADER);
  GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
  GLuint shaders[] { vs, fs };

  // Associate shader handlers with the translation unit's SPIRV data.
  glShaderBinary(2, shaders, GL_SHADER_BINARY_FORMAT_SPIR_V_ARB, 
    __spirv_data, __spirv_size);
  glSpecializeShader(vs, @spirv(vert_main), 0, nullptr, nullptr);
  glSpecializeShader(fs, @spirv(frag_main<shader_t, I>), 0, nullptr, nullptr);

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

template<typename shader_t, int I>
bool program_t<shader_t, I>::configure(bool update_ubo) {
  bool changed = render_imgui(shader, "child");

  if(update_ubo)
    glNamedBufferSubData(ubo, 0, sizeof(shader_t), &shader);

  return changed;
}

template<typename shader_t, int I>
vec4 program_t<shader_t, I>::eval(vec2 coord, shadertoy_uniforms_t u, 
  bool signal) {
  assert(0 < coord.x && coord.x < u.resolution.x);
  assert(0 < coord.y && coord.y < u.resolution.y);

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
  shader_program_t active_shader { };

  shadertoy_uniforms_t uniforms;

  // Keep an instance of the current program.
  std::unique_ptr<program_base_t> program;

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
    uniforms.time = glfwGetTime();

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
      glBindBufferBase(GL_UNIFORM_BUFFER, 1 + (int)active_shader, program->ubo);

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
        program = std::make_unique<program_t<@enum_type(e), (int)e> >();
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
