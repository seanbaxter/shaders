#pragma once

// linear to sRGB approximation
// see http://chilliant.blogspot.com/2012/08/srgb-approximations-for-hlsl.html
inline vec3 linearTosRGB(vec3 color) {
  const float GAMMA = 2.2;
  const float INV_GAMMA = 1.0 / GAMMA;

  return pow(color, vec3(INV_GAMMA));
}

// sRGB to linear approximation
// see http://chilliant.blogspot.com/2012/08/srgb-approximations-for-hlsl.html
inline vec3 sRGBToLinear(vec3 srgbIn) {
  const float GAMMA = 2.2;
  return vec3(pow(srgbIn.xyz, vec3(GAMMA)));
}

inline vec4 sRGBToLinear(vec4 srgbIn) {
  return vec4(sRGBToLinear(srgbIn.xyz), srgbIn.w);
}

// Uncharted 2 tone map
// see: http://filmicworlds.com/blog/filmic-tonemapping-operators/
inline vec3 toneMapUncharted2Impl(vec3 color) {
  const float A = 0.15;
  const float B = 0.50;
  const float C = 0.10;
  const float D = 0.20;
  const float E = 0.02;
  const float F = 0.30;
  return ((color*(A*color+C*B)+D*E)/(color*(A*color+B)+D*F))-E/F;
}

inline vec3 toneMapUncharted(vec3 color) {
  const float W = 11.2;
  color = toneMapUncharted2Impl(2 * color);
  vec3 whiteScale = 1 / toneMapUncharted2Impl(W);
  return linearTosRGB(color * whiteScale);
}

// Hejl Richard tone map
// see: http://filmicworlds.com/blog/filmic-tonemapping-operators/
inline vec3 toneMapHejlRichard(vec3 color) {
  color = max(vec3(0.0), color - vec3(0.004));
  return (color * (6.2f * color + .5f)) / 
    (color * (6.2f * color + 1.7f) + 0.06f);
}

// ACES tone map
// see: https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
inline vec3 toneMapACES(vec3 color) {
  const float A = 2.51;
  const float B = 0.03;
  const float C = 2.43;
  const float D = 0.59;
  const float E = 0.14;
  return linearTosRGB(clamp((color * (A * color + B)) / (color * (C * color + D) + E), 0.f, 1.f));
}

inline vec3 toneMap(vec3 color, float exposure) {
  color *= exposure;
/*
#ifdef TONEMAP_UNCHARTED
    return toneMapUncharted(color);
#endif

#ifdef TONEMAP_HEJLRICHARD
    return toneMapHejlRichard(color);
#endif

#ifdef TONEMAP_ACES
    return toneMapACES(color);
#endif
*/
  return linearTosRGB(color);
}
