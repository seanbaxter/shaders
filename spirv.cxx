#include "implicit.hxx"
#include "../fe/grammar.hxx"

BEGIN_SEMA_NAMESPACE

using fe::range_t;

const char* spirv_pre = R"text(
#ifdef SPIRV_IMPLICIT_NAMESPACE
namespace SPIRV_IMPLICIT_NAMESPACE {
#endif

typedef float    vec2    __attribute__((vector_size(8)));
typedef float    vec3    __attribute__((vector_size(12)));
typedef float    vec4    __attribute__((vector_size(16)));
 
typedef double   dvec2   __attribute__((vector_size(16)));
typedef double   dvec3   __attribute__((vector_size(24)));
typedef double   dvec4   __attribute__((vector_size(32)));
 
typedef int      ivec2   __attribute__((vector_size(8)));
typedef int      ivec3   __attribute__((vector_size(12)));
typedef int      ivec4   __attribute__((vector_size(16)));
 
typedef unsigned uvec2   __attribute__((vector_size(8)));
typedef unsigned uvec3   __attribute__((vector_size(12)));
typedef unsigned uvec4   __attribute__((vector_size(16)));

typedef bool     bvec2    __attribute__((vector_size(2)));
typedef bool     bvec3    __attribute__((vector_size(3)));
typedef bool     bvec4    __attribute__((vector_size(4)));

typedef vec2     mat2x2  __attribute__((matrix_size(16)));
typedef vec2     mat3x2  __attribute__((matrix_size(24)));
typedef vec2     mat4x2  __attribute__((matrix_size(32)));
typedef vec3     mat2x3  __attribute__((matrix_size(24)));
typedef vec3     mat3x3  __attribute__((matrix_size(36)));
typedef vec3     mat4x3  __attribute__((matrix_size(48)));
typedef vec4     mat2x4  __attribute__((matrix_size(32)));
typedef vec4     mat3x4  __attribute__((matrix_size(48)));
typedef vec4     mat4x4  __attribute__((matrix_size(64)));
typedef mat2x2   mat2;
typedef mat3x3   mat3;
typedef mat4x4   mat4;
 
typedef dvec2    dmat2x2  __attribute__((matrix_size(32)));
typedef dvec2    dmat3x2  __attribute__((matrix_size(48)));
typedef dvec2    dmat4x2  __attribute__((matrix_size(64)));
typedef dvec3    dmat2x3  __attribute__((matrix_size(48)));
typedef dvec3    dmat3x3  __attribute__((matrix_size(72)));
typedef dvec3    dmat4x3  __attribute__((matrix_size(96)));
typedef dvec4    dmat2x4  __attribute__((matrix_size(64)));
typedef dvec4    dmat3x4  __attribute__((matrix_size(96)));
typedef dvec4    dmat4x4  __attribute__((matrix_size(128)));
typedef dmat2x2  dmat2;
typedef dmat3x3  dmat3;
typedef dmat4x4  dmat4;

////////////////////////////////////////////////////////////////////////////////
#define __CORRECT_ISO_CPP_MATH_H_PROTO
#define GLSL_PREFIX inline constexpr

// https://www.khronos.org/registry/OpenGL/specs/gl/GLSLangSpec.4.60.html#angle-and-trigonometry-functions
#define PI_F 3.14159265359f
#define PI_D 3.14159265359

GLSL_PREFIX float  [[spirv::GLSLstd450(11)]] radians(float  a) noexcept { return (PI_F / 180) * a; }
GLSL_PREFIX vec2   [[spirv::GLSLstd450(11)]] radians(vec2   a) noexcept { return (PI_F / 180) * a; }
GLSL_PREFIX vec3   [[spirv::GLSLstd450(11)]] radians(vec3   a) noexcept { return (PI_F / 180) * a; }
GLSL_PREFIX vec4   [[spirv::GLSLstd450(11)]] radians(vec4   a) noexcept { return (PI_F / 180) * a; }
GLSL_PREFIX double [[spirv::GLSLstd450(11)]] radians(double a) noexcept { return (PI_D / 180) * a; }
GLSL_PREFIX dvec2  [[spirv::GLSLstd450(11)]] radians(dvec2  a) noexcept { return (PI_D / 180) * a; }
GLSL_PREFIX dvec3  [[spirv::GLSLstd450(11)]] radians(dvec3  a) noexcept { return (PI_D / 180) * a; }
GLSL_PREFIX dvec4  [[spirv::GLSLstd450(11)]] radians(dvec4  a) noexcept { return (PI_D / 180) * a; }
 
GLSL_PREFIX float  [[spirv::GLSLstd450(12)]] degrees(float  a) noexcept { return (180 / PI_F) * a; }
GLSL_PREFIX vec2   [[spirv::GLSLstd450(12)]] degrees(vec2   a) noexcept { return (180 / PI_F) * a; }
GLSL_PREFIX vec3   [[spirv::GLSLstd450(12)]] degrees(vec3   a) noexcept { return (180 / PI_F) * a; }
GLSL_PREFIX vec4   [[spirv::GLSLstd450(12)]] degrees(vec4   a) noexcept { return (180 / PI_F) * a; }
GLSL_PREFIX double [[spirv::GLSLstd450(12)]] degrees(double a) noexcept { return (180 / PI_D) * a; }
GLSL_PREFIX dvec2  [[spirv::GLSLstd450(12)]] degrees(dvec2  a) noexcept { return (180 / PI_D) * a; }
GLSL_PREFIX dvec3  [[spirv::GLSLstd450(12)]] degrees(dvec3  a) noexcept { return (180 / PI_D) * a; }
GLSL_PREFIX dvec4  [[spirv::GLSLstd450(12)]] degrees(dvec4  a) noexcept { return (180 / PI_D) * a; }

#undef PI_F
#undef PI_D

GLSL_PREFIX float  [[spirv::GLSLstd450(13)]] sin(float a) noexcept { return                __builtin_sinf( a); }
GLSL_PREFIX vec2   [[spirv::GLSLstd450(13)]] sin(vec2  a) noexcept { return __vector_apply(__builtin_sinf, a); }
GLSL_PREFIX vec3   [[spirv::GLSLstd450(13)]] sin(vec3  a) noexcept { return __vector_apply(__builtin_sinf, a); }
GLSL_PREFIX vec4   [[spirv::GLSLstd450(13)]] sin(vec4  a) noexcept { return __vector_apply(__builtin_sinf, a); }
extern "C"  double [[spirv::GLSLstd450(13)]] sin(double ) noexcept;
GLSL_PREFIX dvec2  [[spirv::GLSLstd450(13)]] sin(dvec2 a) noexcept { return __vector_apply(__builtin_sin,  a); }
GLSL_PREFIX dvec3  [[spirv::GLSLstd450(13)]] sin(dvec3 a) noexcept { return __vector_apply(__builtin_sin,  a); }
GLSL_PREFIX dvec4  [[spirv::GLSLstd450(13)]] sin(dvec4 a) noexcept { return __vector_apply(__builtin_sin,  a); }
GLSL_PREFIX long double sin(long double a) noexcept { return __builtin_sinl(a); }

GLSL_PREFIX float  [[spirv::GLSLstd450(14)]] cos(float a) noexcept { return                __builtin_cosf( a); }
GLSL_PREFIX vec2   [[spirv::GLSLstd450(14)]] cos(vec2  a) noexcept { return __vector_apply(__builtin_cosf, a); }
GLSL_PREFIX vec3   [[spirv::GLSLstd450(14)]] cos(vec3  a) noexcept { return __vector_apply(__builtin_cosf, a); }
GLSL_PREFIX vec4   [[spirv::GLSLstd450(14)]] cos(vec4  a) noexcept { return __vector_apply(__builtin_cosf, a); }
extern "C"  double [[spirv::GLSLstd450(14)]] cos(double ) noexcept;
GLSL_PREFIX dvec2  [[spirv::GLSLstd450(14)]] cos(dvec2 a) noexcept { return __vector_apply(__builtin_cos,  a); }
GLSL_PREFIX dvec3  [[spirv::GLSLstd450(14)]] cos(dvec3 a) noexcept { return __vector_apply(__builtin_cos,  a); }
GLSL_PREFIX dvec4  [[spirv::GLSLstd450(14)]] cos(dvec4 a) noexcept { return __vector_apply(__builtin_cos,  a); }
GLSL_PREFIX long double cos(long double a) noexcept { return __builtin_cosl(a); }

GLSL_PREFIX float  [[spirv::GLSLstd450(15)]] tan(float a) noexcept { return                __builtin_tanf( a); }
GLSL_PREFIX vec2   [[spirv::GLSLstd450(15)]] tan(vec2  a) noexcept { return __vector_apply(__builtin_tanf, a); }
GLSL_PREFIX vec3   [[spirv::GLSLstd450(15)]] tan(vec3  a) noexcept { return __vector_apply(__builtin_tanf, a); }
GLSL_PREFIX vec4   [[spirv::GLSLstd450(15)]] tan(vec4  a) noexcept { return __vector_apply(__builtin_tanf, a); }
extern "C"  double [[spirv::GLSLstd450(15)]] tan(double ) noexcept;
GLSL_PREFIX dvec2  [[spirv::GLSLstd450(15)]] tan(dvec2 a) noexcept { return __vector_apply(__builtin_tan,  a); }
GLSL_PREFIX dvec3  [[spirv::GLSLstd450(15)]] tan(dvec3 a) noexcept { return __vector_apply(__builtin_tan,  a); }
GLSL_PREFIX dvec4  [[spirv::GLSLstd450(15)]] tan(dvec4 a) noexcept { return __vector_apply(__builtin_tan,  a); }
GLSL_PREFIX long double tan(long double a) noexcept { return __builtin_tanl(a); }

GLSL_PREFIX float  [[spirv::GLSLstd450(16)]] asin(float a) noexcept { return                __builtin_asinf( a); }
GLSL_PREFIX vec2   [[spirv::GLSLstd450(16)]] asin(vec2  a) noexcept { return __vector_apply(__builtin_asinf, a); }
GLSL_PREFIX vec3   [[spirv::GLSLstd450(16)]] asin(vec3  a) noexcept { return __vector_apply(__builtin_asinf, a); }
GLSL_PREFIX vec4   [[spirv::GLSLstd450(16)]] asin(vec4  a) noexcept { return __vector_apply(__builtin_asinf, a); }
extern "C"  double [[spirv::GLSLstd450(16)]] asin(double ) noexcept;
GLSL_PREFIX dvec2  [[spirv::GLSLstd450(16)]] asin(dvec2 a) noexcept { return __vector_apply(__builtin_asin,  a); }
GLSL_PREFIX dvec3  [[spirv::GLSLstd450(16)]] asin(dvec3 a) noexcept { return __vector_apply(__builtin_asin,  a); }
GLSL_PREFIX dvec4  [[spirv::GLSLstd450(16)]] asin(dvec4 a) noexcept { return __vector_apply(__builtin_asin,  a); }
GLSL_PREFIX long double aasin(long double a) noexcept { return __builtin_asinl(a); }

GLSL_PREFIX float  [[spirv::GLSLstd450(17)]] acos(float a) noexcept { return                __builtin_acosf( a); }
GLSL_PREFIX vec2   [[spirv::GLSLstd450(17)]] acos(vec2  a) noexcept { return __vector_apply(__builtin_acosf, a); }
GLSL_PREFIX vec3   [[spirv::GLSLstd450(17)]] acos(vec3  a) noexcept { return __vector_apply(__builtin_acosf, a); }
GLSL_PREFIX vec4   [[spirv::GLSLstd450(17)]] acos(vec4  a) noexcept { return __vector_apply(__builtin_acosf, a); }
extern "C"  double [[spirv::GLSLstd450(17)]] acos(double ) noexcept;
GLSL_PREFIX dvec2  [[spirv::GLSLstd450(17)]] acos(dvec2 a) noexcept { return __vector_apply(__builtin_acos,  a); }
GLSL_PREFIX dvec3  [[spirv::GLSLstd450(17)]] acos(dvec3 a) noexcept { return __vector_apply(__builtin_acos,  a); }
GLSL_PREFIX dvec4  [[spirv::GLSLstd450(17)]] acos(dvec4 a) noexcept { return __vector_apply(__builtin_acos,  a); }
GLSL_PREFIX long double acos(long double a) noexcept { return __builtin_acosl(a); }

GLSL_PREFIX float  [[spirv::GLSLstd450(18)]] atan(float a) noexcept { return                __builtin_atanf( a); }
GLSL_PREFIX vec2   [[spirv::GLSLstd450(18)]] atan(vec2  a) noexcept { return __vector_apply(__builtin_atanf, a); }
GLSL_PREFIX vec3   [[spirv::GLSLstd450(18)]] atan(vec3  a) noexcept { return __vector_apply(__builtin_atanf, a); }
GLSL_PREFIX vec4   [[spirv::GLSLstd450(18)]] atan(vec4  a) noexcept { return __vector_apply(__builtin_atanf, a); }
extern "C"  double [[spirv::GLSLstd450(18)]] atan(double ) noexcept;
GLSL_PREFIX dvec2  [[spirv::GLSLstd450(18)]] atan(dvec2 a) noexcept { return __vector_apply(__builtin_atan,  a); }
GLSL_PREFIX dvec3  [[spirv::GLSLstd450(18)]] atan(dvec3 a) noexcept { return __vector_apply(__builtin_atan,  a); }
GLSL_PREFIX dvec4  [[spirv::GLSLstd450(18)]] atan(dvec4 a) noexcept { return __vector_apply(__builtin_atan,  a); }
GLSL_PREFIX long double atan(long double a) noexcept { return __builtin_atanl(a); }

GLSL_PREFIX float  [[spirv::GLSLstd450(25)]] atan2(float a, float b) noexcept { return                __builtin_atan2f( a, b); }
GLSL_PREFIX vec2   [[spirv::GLSLstd450(25)]] atan2(vec2  a, vec2  b) noexcept { return __vector_apply(__builtin_atan2f, a, b); }
GLSL_PREFIX vec3   [[spirv::GLSLstd450(25)]] atan2(vec3  a, vec3  b) noexcept { return __vector_apply(__builtin_atan2f, a, b); }
GLSL_PREFIX vec4   [[spirv::GLSLstd450(25)]] atan2(vec4  a, vec4  b) noexcept { return __vector_apply(__builtin_atan2f, a, b); }
extern "C"  double [[spirv::GLSLstd450(25)]] atan2(double , double ) noexcept;
GLSL_PREFIX dvec2  [[spirv::GLSLstd450(25)]] atan2(dvec2 a, dvec2 b) noexcept { return __vector_apply(__builtin_atan2,  a, b); }
GLSL_PREFIX dvec3  [[spirv::GLSLstd450(25)]] atan2(dvec3 a, dvec3 b) noexcept { return __vector_apply(__builtin_atan2,  a, b); }
GLSL_PREFIX dvec4  [[spirv::GLSLstd450(25)]] atan2(dvec4 a, dvec4 b) noexcept { return __vector_apply(__builtin_atan2,  a, b); }
GLSL_PREFIX long double atan2(long double a, long double b) noexcept { return __builtin_atan2l(a, b); }

GLSL_PREFIX float  [[spirv::GLSLstd450(19)]] sinh(float a) noexcept { return                __builtin_sinhf( a); }
GLSL_PREFIX vec2   [[spirv::GLSLstd450(19)]] sinh(vec2  a) noexcept { return __vector_apply(__builtin_sinhf, a); }
GLSL_PREFIX vec3   [[spirv::GLSLstd450(19)]] sinh(vec3  a) noexcept { return __vector_apply(__builtin_sinhf, a); }
GLSL_PREFIX vec4   [[spirv::GLSLstd450(19)]] sinh(vec4  a) noexcept { return __vector_apply(__builtin_sinhf, a); }
extern "C"  double [[spirv::GLSLstd450(19)]] sinh(double ) noexcept;
GLSL_PREFIX dvec2  [[spirv::GLSLstd450(19)]] sinh(dvec2 a) noexcept { return __vector_apply(__builtin_sinh,  a); }
GLSL_PREFIX dvec3  [[spirv::GLSLstd450(19)]] sinh(dvec3 a) noexcept { return __vector_apply(__builtin_sinh,  a); }
GLSL_PREFIX dvec4  [[spirv::GLSLstd450(19)]] sinh(dvec4 a) noexcept { return __vector_apply(__builtin_sinh,  a); }
GLSL_PREFIX long double sinh(long double a) noexcept { return __builtin_sinhl(a); }

GLSL_PREFIX float  [[spirv::GLSLstd450(20)]] cosh(float a) noexcept { return                __builtin_coshf( a); }
GLSL_PREFIX vec2   [[spirv::GLSLstd450(20)]] cosh(vec2  a) noexcept { return __vector_apply(__builtin_coshf, a); }
GLSL_PREFIX vec3   [[spirv::GLSLstd450(20)]] cosh(vec3  a) noexcept { return __vector_apply(__builtin_coshf, a); }
GLSL_PREFIX vec4   [[spirv::GLSLstd450(20)]] cosh(vec4  a) noexcept { return __vector_apply(__builtin_coshf, a); }
extern "C"  double [[spirv::GLSLstd450(20)]] cosh(double ) noexcept;
GLSL_PREFIX dvec2  [[spirv::GLSLstd450(20)]] cosh(dvec2 a) noexcept { return __vector_apply(__builtin_cosh,  a); }
GLSL_PREFIX dvec3  [[spirv::GLSLstd450(20)]] cosh(dvec3 a) noexcept { return __vector_apply(__builtin_cosh,  a); }
GLSL_PREFIX dvec4  [[spirv::GLSLstd450(20)]] cosh(dvec4 a) noexcept { return __vector_apply(__builtin_cosh,  a); }
GLSL_PREFIX long double cosh(long double a) noexcept { return __builtin_coshl(a); }

GLSL_PREFIX float  [[spirv::GLSLstd450(21)]] tanh(float a) noexcept { return                __builtin_tanhf( a); }
GLSL_PREFIX vec2   [[spirv::GLSLstd450(21)]] tanh(vec2  a) noexcept { return __vector_apply(__builtin_tanhf, a); }
GLSL_PREFIX vec3   [[spirv::GLSLstd450(21)]] tanh(vec3  a) noexcept { return __vector_apply(__builtin_tanhf, a); }
GLSL_PREFIX vec4   [[spirv::GLSLstd450(21)]] tanh(vec4  a) noexcept { return __vector_apply(__builtin_tanhf, a); }
extern "C"  double [[spirv::GLSLstd450(21)]] tanh(double ) noexcept;
GLSL_PREFIX dvec2  [[spirv::GLSLstd450(21)]] tanh(dvec2 a) noexcept { return __vector_apply(__builtin_tanh,  a); }
GLSL_PREFIX dvec3  [[spirv::GLSLstd450(21)]] tanh(dvec3 a) noexcept { return __vector_apply(__builtin_tanh,  a); }
GLSL_PREFIX dvec4  [[spirv::GLSLstd450(21)]] tanh(dvec4 a) noexcept { return __vector_apply(__builtin_tanh,  a); }
GLSL_PREFIX long double tanh(long double a) noexcept { return __builtin_tanhl(a); }

GLSL_PREFIX float  [[spirv::GLSLstd450(22)]] asinh(float a) noexcept { return                __builtin_asinhf( a); }
GLSL_PREFIX vec2   [[spirv::GLSLstd450(22)]] asinh(vec2  a) noexcept { return __vector_apply(__builtin_asinhf, a); }
GLSL_PREFIX vec3   [[spirv::GLSLstd450(22)]] asinh(vec3  a) noexcept { return __vector_apply(__builtin_asinhf, a); }
GLSL_PREFIX vec4   [[spirv::GLSLstd450(22)]] asinh(vec4  a) noexcept { return __vector_apply(__builtin_asinhf, a); }
extern "C"  double [[spirv::GLSLstd450(22)]] asinh(double ) noexcept;
GLSL_PREFIX dvec2  [[spirv::GLSLstd450(22)]] asinh(dvec2 a) noexcept { return __vector_apply(__builtin_asinh,  a); }
GLSL_PREFIX dvec3  [[spirv::GLSLstd450(22)]] asinh(dvec3 a) noexcept { return __vector_apply(__builtin_asinh,  a); }
GLSL_PREFIX dvec4  [[spirv::GLSLstd450(22)]] asinh(dvec4 a) noexcept { return __vector_apply(__builtin_asinh,  a); }
GLSL_PREFIX long double asinh(long double a) noexcept { return __builtin_asinhl(a); }

GLSL_PREFIX float  [[spirv::GLSLstd450(23)]] acosh(float a) noexcept { return                __builtin_acoshf( a); }
GLSL_PREFIX vec2   [[spirv::GLSLstd450(23)]] acosh(vec2  a) noexcept { return __vector_apply(__builtin_acoshf, a); }
GLSL_PREFIX vec3   [[spirv::GLSLstd450(23)]] acosh(vec3  a) noexcept { return __vector_apply(__builtin_acoshf, a); }
GLSL_PREFIX vec4   [[spirv::GLSLstd450(23)]] acosh(vec4  a) noexcept { return __vector_apply(__builtin_acoshf, a); }
extern "C"  double [[spirv::GLSLstd450(23)]] acosh(double ) noexcept;
GLSL_PREFIX dvec2  [[spirv::GLSLstd450(23)]] acosh(dvec2 a) noexcept { return __vector_apply(__builtin_acosh,  a); }
GLSL_PREFIX dvec3  [[spirv::GLSLstd450(23)]] acosh(dvec3 a) noexcept { return __vector_apply(__builtin_acosh,  a); }
GLSL_PREFIX dvec4  [[spirv::GLSLstd450(23)]] acosh(dvec4 a) noexcept { return __vector_apply(__builtin_acosh,  a); }
GLSL_PREFIX long double acosh(long double a) noexcept { return __builtin_acoshl(a); }

GLSL_PREFIX float  [[spirv::GLSLstd450(24)]] atanh(float a) noexcept { return                __builtin_atanhf( a); }
GLSL_PREFIX vec2   [[spirv::GLSLstd450(24)]] atanh(vec2  a) noexcept { return __vector_apply(__builtin_atanhf, a); }
GLSL_PREFIX vec3   [[spirv::GLSLstd450(24)]] atanh(vec3  a) noexcept { return __vector_apply(__builtin_atanhf, a); }
GLSL_PREFIX vec4   [[spirv::GLSLstd450(24)]] atanh(vec4  a) noexcept { return __vector_apply(__builtin_atanhf, a); }
extern "C"  double [[spirv::GLSLstd450(24)]] atanh(double ) noexcept;
GLSL_PREFIX dvec2  [[spirv::GLSLstd450(24)]] atanh(dvec2 a) noexcept { return __vector_apply(__builtin_atanh,  a); }
GLSL_PREFIX dvec3  [[spirv::GLSLstd450(24)]] atanh(dvec3 a) noexcept { return __vector_apply(__builtin_atanh,  a); }
GLSL_PREFIX dvec4  [[spirv::GLSLstd450(24)]] atanh(dvec4 a) noexcept { return __vector_apply(__builtin_atanh,  a); }
GLSL_PREFIX long double atanh(long double a) noexcept { return __builtin_atanhl(a); }

// https://www.khronos.org/registry/OpenGL/specs/gl/GLSLangSpec.4.60.html#exponential-functions

GLSL_PREFIX float  [[spirv::GLSLstd450(26)]] pow(float a, float b) noexcept { return                __builtin_powf( a, b); }
GLSL_PREFIX vec2   [[spirv::GLSLstd450(26)]] pow(vec2  a, vec2  b) noexcept { return __vector_apply(__builtin_powf, a, b); }
GLSL_PREFIX vec3   [[spirv::GLSLstd450(26)]] pow(vec3  a, vec3  b) noexcept { return __vector_apply(__builtin_powf, a, b); }
GLSL_PREFIX vec4   [[spirv::GLSLstd450(26)]] pow(vec4  a, vec4  b) noexcept { return __vector_apply(__builtin_powf, a, b); }
extern "C"  double [[spirv::GLSLstd450(26)]] pow(double , double ) noexcept;
GLSL_PREFIX dvec2  [[spirv::GLSLstd450(26)]] pow(dvec2 a, dvec2 b) noexcept { return __vector_apply(__builtin_pow,  a, b); }
GLSL_PREFIX dvec3  [[spirv::GLSLstd450(26)]] pow(dvec3 a, dvec3 b) noexcept { return __vector_apply(__builtin_pow,  a, b); }
GLSL_PREFIX dvec4  [[spirv::GLSLstd450(26)]] pow(dvec4 a, dvec4 b) noexcept { return __vector_apply(__builtin_pow,  a, b); }
GLSL_PREFIX long double pow(long double a, long double b) noexcept { return __builtin_powl(a, b); }

GLSL_PREFIX float  [[spirv::GLSLstd450(27)]] exp(float a) noexcept { return                __builtin_expf( a); }
GLSL_PREFIX vec2   [[spirv::GLSLstd450(27)]] exp(vec2  a) noexcept { return __vector_apply(__builtin_expf, a); }
GLSL_PREFIX vec3   [[spirv::GLSLstd450(27)]] exp(vec3  a) noexcept { return __vector_apply(__builtin_expf, a); }
GLSL_PREFIX vec4   [[spirv::GLSLstd450(27)]] exp(vec4  a) noexcept { return __vector_apply(__builtin_expf, a); }
extern "C"  double [[spirv::GLSLstd450(27)]] exp(double ) noexcept;
GLSL_PREFIX dvec2  [[spirv::GLSLstd450(27)]] exp(dvec2 a) noexcept { return __vector_apply(__builtin_exp,  a); }
GLSL_PREFIX dvec3  [[spirv::GLSLstd450(27)]] exp(dvec3 a) noexcept { return __vector_apply(__builtin_exp,  a); }
GLSL_PREFIX dvec4  [[spirv::GLSLstd450(27)]] exp(dvec4 a) noexcept { return __vector_apply(__builtin_exp,  a); }
GLSL_PREFIX long double exp(long double a) noexcept { return __builtin_expl(a); }

GLSL_PREFIX float  [[spirv::GLSLstd450(28)]] log(float a) noexcept { return                __builtin_logf( a); }
GLSL_PREFIX vec2   [[spirv::GLSLstd450(28)]] log(vec2  a) noexcept { return __vector_apply(__builtin_logf, a); }
GLSL_PREFIX vec3   [[spirv::GLSLstd450(28)]] log(vec3  a) noexcept { return __vector_apply(__builtin_logf, a); }
GLSL_PREFIX vec4   [[spirv::GLSLstd450(28)]] log(vec4  a) noexcept { return __vector_apply(__builtin_logf, a); }
extern "C"  double [[spirv::GLSLstd450(28)]] log(double ) noexcept;
GLSL_PREFIX dvec2  [[spirv::GLSLstd450(28)]] log(dvec2 a) noexcept { return __vector_apply(__builtin_log,  a); }
GLSL_PREFIX dvec3  [[spirv::GLSLstd450(28)]] log(dvec3 a) noexcept { return __vector_apply(__builtin_log,  a); }
GLSL_PREFIX dvec4  [[spirv::GLSLstd450(28)]] log(dvec4 a) noexcept { return __vector_apply(__builtin_log,  a); }
GLSL_PREFIX long double log(long double a) noexcept { return __builtin_logl(a); }

GLSL_PREFIX float  [[spirv::GLSLstd450(29)]] exp2(float a) noexcept { return                __builtin_exp2f( a); }
GLSL_PREFIX vec2   [[spirv::GLSLstd450(29)]] exp2(vec2  a) noexcept { return __vector_apply(__builtin_exp2f, a); }
GLSL_PREFIX vec3   [[spirv::GLSLstd450(29)]] exp2(vec3  a) noexcept { return __vector_apply(__builtin_exp2f, a); }
GLSL_PREFIX vec4   [[spirv::GLSLstd450(29)]] exp2(vec4  a) noexcept { return __vector_apply(__builtin_exp2f, a); }
extern "C"  double [[spirv::GLSLstd450(29)]] exp2(double ) noexcept;
GLSL_PREFIX dvec2  [[spirv::GLSLstd450(29)]] exp2(dvec2 a) noexcept { return __vector_apply(__builtin_exp2,  a); }
GLSL_PREFIX dvec3  [[spirv::GLSLstd450(29)]] exp2(dvec3 a) noexcept { return __vector_apply(__builtin_exp2,  a); }
GLSL_PREFIX dvec4  [[spirv::GLSLstd450(29)]] exp2(dvec4 a) noexcept { return __vector_apply(__builtin_exp2,  a); }
GLSL_PREFIX long double exp2(long double a) noexcept { return __builtin_exp2l(a); }

GLSL_PREFIX float  [[spirv::GLSLstd450(30)]] log2(float a) noexcept { return                __builtin_log2f( a); }
GLSL_PREFIX vec2   [[spirv::GLSLstd450(30)]] log2(vec2  a) noexcept { return __vector_apply(__builtin_log2f, a); }
GLSL_PREFIX vec3   [[spirv::GLSLstd450(30)]] log2(vec3  a) noexcept { return __vector_apply(__builtin_log2f, a); }
GLSL_PREFIX vec4   [[spirv::GLSLstd450(30)]] log2(vec4  a) noexcept { return __vector_apply(__builtin_log2f, a); }
extern "C"  double [[spirv::GLSLstd450(30)]] log2(double ) noexcept;
GLSL_PREFIX dvec2  [[spirv::GLSLstd450(30)]] log2(dvec2 a) noexcept { return __vector_apply(__builtin_log2,  a); }
GLSL_PREFIX dvec3  [[spirv::GLSLstd450(30)]] log2(dvec3 a) noexcept { return __vector_apply(__builtin_log2,  a); }
GLSL_PREFIX dvec4  [[spirv::GLSLstd450(30)]] log2(dvec4 a) noexcept { return __vector_apply(__builtin_log2,  a); }
GLSL_PREFIX long double log2(long double a) noexcept { return __builtin_log2l(a); }

GLSL_PREFIX float  [[spirv::GLSLstd450(31)]] sqrt(float a) noexcept { return                __builtin_sqrtf( a); }
GLSL_PREFIX vec2   [[spirv::GLSLstd450(31)]] sqrt(vec2  a) noexcept { return __vector_apply(__builtin_sqrtf, a); }
GLSL_PREFIX vec3   [[spirv::GLSLstd450(31)]] sqrt(vec3  a) noexcept { return __vector_apply(__builtin_sqrtf, a); }
GLSL_PREFIX vec4   [[spirv::GLSLstd450(31)]] sqrt(vec4  a) noexcept { return __vector_apply(__builtin_sqrtf, a); }
extern "C" double  [[spirv::GLSLstd450(31)]] sqrt(double ) noexcept;
GLSL_PREFIX dvec2  [[spirv::GLSLstd450(31)]] sqrt(dvec2 a) noexcept { return __vector_apply(__builtin_sqrt,  a); }
GLSL_PREFIX dvec3  [[spirv::GLSLstd450(31)]] sqrt(dvec3 a) noexcept { return __vector_apply(__builtin_sqrt,  a); }
GLSL_PREFIX dvec4  [[spirv::GLSLstd450(31)]] sqrt(dvec4 a) noexcept { return __vector_apply(__builtin_sqrt,  a); }
GLSL_PREFIX long double sqrt(long double a) noexcept { return __builtin_sqrtl(a); }

GLSL_PREFIX float  [[spirv::GLSLstd450(32)]] inversesqrt(float  a) noexcept { return                __builtin_rsqrtf( a); }
GLSL_PREFIX vec2   [[spirv::GLSLstd450(32)]] inversesqrt(vec2   a) noexcept { return __vector_apply(__builtin_rsqrtf, a); }
GLSL_PREFIX vec3   [[spirv::GLSLstd450(32)]] inversesqrt(vec3   a) noexcept { return __vector_apply(__builtin_rsqrtf, a); }
GLSL_PREFIX vec4   [[spirv::GLSLstd450(32)]] inversesqrt(vec4   a) noexcept { return __vector_apply(__builtin_rsqrtf, a); }
GLSL_PREFIX double [[spirv::GLSLstd450(32)]] inversesqrt(double a) noexcept { return                __builtin_rsqrt ( a); }
GLSL_PREFIX dvec2  [[spirv::GLSLstd450(32)]] inversesqrt(dvec2  a) noexcept { return __vector_apply(__builtin_rsqrt,  a); }
GLSL_PREFIX dvec3  [[spirv::GLSLstd450(32)]] inversesqrt(dvec3  a) noexcept { return __vector_apply(__builtin_rsqrt,  a); }
GLSL_PREFIX dvec4  [[spirv::GLSLstd450(32)]] inversesqrt(dvec4  a) noexcept { return __vector_apply(__builtin_rsqrt,  a); }

// 8.3 Common Functions

GLSL_PREFIX float  [[spirv::GLSLstd450(4)]] abs(float  a) noexcept { return                __builtin_fabsf( a); }
GLSL_PREFIX vec2   [[spirv::GLSLstd450(4)]] abs(vec2   a) noexcept { return __vector_apply(__builtin_fabsf, a); }
GLSL_PREFIX vec3   [[spirv::GLSLstd450(4)]] abs(vec3   a) noexcept { return __vector_apply(__builtin_fabsf, a); }
GLSL_PREFIX vec4   [[spirv::GLSLstd450(4)]] abs(vec4   a) noexcept { return __vector_apply(__builtin_fabsf, a); }
GLSL_PREFIX double [[spirv::GLSLstd450(4)]] abs(double a) noexcept { return                __builtin_fabs(  a); }
GLSL_PREFIX dvec2  [[spirv::GLSLstd450(4)]] abs(dvec2  a) noexcept { return __vector_apply(__builtin_fabs,  a); }
GLSL_PREFIX dvec3  [[spirv::GLSLstd450(4)]] abs(dvec3  a) noexcept { return __vector_apply(__builtin_fabs,  a); }
GLSL_PREFIX dvec4  [[spirv::GLSLstd450(4)]] abs(dvec4  a) noexcept { return __vector_apply(__builtin_fabs,  a); }
GLSL_PREFIX ivec2  [[spirv::GLSLstd450(5)]] abs(ivec2  a) noexcept { return __vector_apply(__builtin_abs,   a); }
GLSL_PREFIX ivec3  [[spirv::GLSLstd450(5)]] abs(ivec3  a) noexcept { return __vector_apply(__builtin_abs,   a); }
GLSL_PREFIX ivec4  [[spirv::GLSLstd450(5)]] abs(ivec4  a) noexcept { return __vector_apply(__builtin_abs,   a); }

GLSL_PREFIX float  [[spirv::GLSLstd450(6)]] sign(float  a) noexcept { return a >= 0 ? 1.f : -1.f; }
GLSL_PREFIX vec2   [[spirv::GLSLstd450(6)]] sign(vec2   a) noexcept { return a >= 0 ? 1.f : -1.f; }
GLSL_PREFIX vec3   [[spirv::GLSLstd450(6)]] sign(vec3   a) noexcept { return a >= 0 ? 1.f : -1.f; }
GLSL_PREFIX vec4   [[spirv::GLSLstd450(6)]] sign(vec4   a) noexcept { return a >= 0 ? 1.f : -1.f; }
GLSL_PREFIX double [[spirv::GLSLstd450(6)]] sign(double a) noexcept { return a >= 0 ? 1.0 : -1.0; }
GLSL_PREFIX dvec2  [[spirv::GLSLstd450(6)]] sign(dvec2  a) noexcept { return a >= 0 ? 1.0 : -1.0; }
GLSL_PREFIX dvec3  [[spirv::GLSLstd450(6)]] sign(dvec3  a) noexcept { return a >= 0 ? 1.0 : -1.0; }
GLSL_PREFIX dvec4  [[spirv::GLSLstd450(6)]] sign(dvec4  a) noexcept { return a >= 0 ? 1.0 : -1.0; }
GLSL_PREFIX int    [[spirv::GLSLstd450(7)]] sign(int    a) noexcept { return a >= 0 ? 1   : -1;   }
GLSL_PREFIX ivec2  [[spirv::GLSLstd450(7)]] sign(ivec2  a) noexcept { return a >= 0 ? 1   : -1;   }
GLSL_PREFIX ivec3  [[spirv::GLSLstd450(7)]] sign(ivec3  a) noexcept { return a >= 0 ? 1   : -1;   }
GLSL_PREFIX ivec4  [[spirv::GLSLstd450(7)]] sign(ivec4  a) noexcept { return a >= 0 ? 1   : -1;   }

GLSL_PREFIX float  [[spirv::GLSLstd450(8)]] floor(float  a) noexcept { return                __builtin_floorf( a); }
GLSL_PREFIX vec2   [[spirv::GLSLstd450(8)]] floor(vec2   a) noexcept { return __vector_apply(__builtin_floorf, a); }
GLSL_PREFIX vec3   [[spirv::GLSLstd450(8)]] floor(vec3   a) noexcept { return __vector_apply(__builtin_floorf, a); }
GLSL_PREFIX vec4   [[spirv::GLSLstd450(8)]] floor(vec4   a) noexcept { return __vector_apply(__builtin_floorf, a); }
extern "C"  double [[spirv::GLSLstd450(8)]] floor(double  ) noexcept;
GLSL_PREFIX dvec2  [[spirv::GLSLstd450(8)]] floor(dvec2  a) noexcept { return __vector_apply(__builtin_floor,  a); }
GLSL_PREFIX dvec3  [[spirv::GLSLstd450(8)]] floor(dvec3  a) noexcept { return __vector_apply(__builtin_floor,  a); }
GLSL_PREFIX dvec4  [[spirv::GLSLstd450(8)]] floor(dvec4  a) noexcept { return __vector_apply(__builtin_floor,  a); }

GLSL_PREFIX float  [[spirv::GLSLstd450(3)]] trunc(float  a) noexcept { return                __builtin_truncf( a); }
GLSL_PREFIX vec2   [[spirv::GLSLstd450(3)]] trunc(vec2   a) noexcept { return __vector_apply(__builtin_truncf, a); }
GLSL_PREFIX vec3   [[spirv::GLSLstd450(3)]] trunc(vec3   a) noexcept { return __vector_apply(__builtin_truncf, a); }
GLSL_PREFIX vec4   [[spirv::GLSLstd450(3)]] trunc(vec4   a) noexcept { return __vector_apply(__builtin_truncf, a); }
extern "C"  double [[spirv::GLSLstd450(3)]] trunc(double a) noexcept;
GLSL_PREFIX dvec2  [[spirv::GLSLstd450(3)]] trunc(dvec2  a) noexcept { return __vector_apply(__builtin_trunc,  a); }
GLSL_PREFIX dvec3  [[spirv::GLSLstd450(3)]] trunc(dvec3  a) noexcept { return __vector_apply(__builtin_trunc,  a); }
GLSL_PREFIX dvec4  [[spirv::GLSLstd450(3)]] trunc(dvec4  a) noexcept { return __vector_apply(__builtin_trunc,  a); }

GLSL_PREFIX float  [[spirv::GLSLstd450(1)]] round(float  a) noexcept { return                __builtin_roundf( a); }
GLSL_PREFIX vec2   [[spirv::GLSLstd450(1)]] round(vec2   a) noexcept { return __vector_apply(__builtin_roundf, a); }
GLSL_PREFIX vec3   [[spirv::GLSLstd450(1)]] round(vec3   a) noexcept { return __vector_apply(__builtin_roundf, a); }
GLSL_PREFIX vec4   [[spirv::GLSLstd450(1)]] round(vec4   a) noexcept { return __vector_apply(__builtin_roundf, a); }
extern "C"  double [[spirv::GLSLstd450(1)]] round(double  ) noexcept;
GLSL_PREFIX dvec2  [[spirv::GLSLstd450(1)]] round(dvec2  a) noexcept { return __vector_apply(__builtin_round,  a); }
GLSL_PREFIX dvec3  [[spirv::GLSLstd450(1)]] round(dvec3  a) noexcept { return __vector_apply(__builtin_round,  a); }
GLSL_PREFIX dvec4  [[spirv::GLSLstd450(1)]] round(dvec4  a) noexcept { return __vector_apply(__builtin_round,  a); }

GLSL_PREFIX float  [[spirv::opcode(141)]] mod(float  x, float  y) noexcept { return x - y * floor(x / y); }
GLSL_PREFIX vec2   [[spirv::opcode(141)]] mod(vec2   x, vec2   y) noexcept { return x - y * floor(x / y); }
GLSL_PREFIX vec3   [[spirv::opcode(141)]] mod(vec3   x, vec3   y) noexcept { return x - y * floor(x / y); }
GLSL_PREFIX vec4   [[spirv::opcode(141)]] mod(vec4   x, vec4   y) noexcept { return x - y * floor(x / y); }
GLSL_PREFIX double [[spirv::opcode(141)]] mod(double x, double y) noexcept { return x - y * floor(x / y); }
GLSL_PREFIX dvec2  [[spirv::opcode(141)]] mod(dvec2  x, dvec2  y) noexcept { return x - y * floor(x / y); }
GLSL_PREFIX dvec3  [[spirv::opcode(141)]] mod(dvec3  x, dvec3  y) noexcept { return x - y * floor(x / y); }
GLSL_PREFIX dvec4  [[spirv::opcode(141)]] mod(dvec4  x, dvec4  y) noexcept { return x - y * floor(x / y); }

GLSL_PREFIX float  [[spirv::GLSLstd450(9)]] ceil(float  a) noexcept { return                __builtin_ceilf( a); }
GLSL_PREFIX vec2   [[spirv::GLSLstd450(9)]] ceil(vec2   a) noexcept { return __vector_apply(__builtin_ceilf, a); }
GLSL_PREFIX vec3   [[spirv::GLSLstd450(9)]] ceil(vec3   a) noexcept { return __vector_apply(__builtin_ceilf, a); }
GLSL_PREFIX vec4   [[spirv::GLSLstd450(9)]] ceil(vec4   a) noexcept { return __vector_apply(__builtin_ceilf, a); }
extern "C"  double [[spirv::GLSLstd450(9)]] ceil(double  ) noexcept;
GLSL_PREFIX dvec2  [[spirv::GLSLstd450(9)]] ceil(dvec2  a) noexcept { return __vector_apply(__builtin_ceil,  a); }
GLSL_PREFIX dvec3  [[spirv::GLSLstd450(9)]] ceil(dvec3  a) noexcept { return __vector_apply(__builtin_ceil,  a); }
GLSL_PREFIX dvec4  [[spirv::GLSLstd450(9)]] ceil(dvec4  a) noexcept { return __vector_apply(__builtin_ceil,  a); }

GLSL_PREFIX float  [[spirv::GLSLstd450(10)]] fract(float  a) noexcept { return a - floor(a); }
GLSL_PREFIX vec2   [[spirv::GLSLstd450(10)]] fract(vec2   a) noexcept { return a - floor(a); }
GLSL_PREFIX vec3   [[spirv::GLSLstd450(10)]] fract(vec3   a) noexcept { return a - floor(a); }
GLSL_PREFIX vec4   [[spirv::GLSLstd450(10)]] fract(vec4   a) noexcept { return a - floor(a); }
GLSL_PREFIX double [[spirv::GLSLstd450(10)]] fract(double a) noexcept { return a - floor(a); }
GLSL_PREFIX dvec2  [[spirv::GLSLstd450(10)]] fract(dvec2  a) noexcept { return a - floor(a); }
GLSL_PREFIX dvec3  [[spirv::GLSLstd450(10)]] fract(dvec3  a) noexcept { return a - floor(a); }
GLSL_PREFIX dvec4  [[spirv::GLSLstd450(10)]] fract(dvec4  a) noexcept { return a - floor(a); }

GLSL_PREFIX float  [[spirv::GLSLstd450(37)]] min(float  a, float  b) noexcept { return                __builtin_fminf( a, b); }
GLSL_PREFIX vec2   [[spirv::GLSLstd450(37)]] min(vec2   a, vec2   b) noexcept { return __vector_apply(__builtin_fminf, a, b); }
GLSL_PREFIX vec3   [[spirv::GLSLstd450(37)]] min(vec3   a, vec3   b) noexcept { return __vector_apply(__builtin_fminf, a, b); }
GLSL_PREFIX vec4   [[spirv::GLSLstd450(37)]] min(vec4   a, vec4   b) noexcept { return __vector_apply(__builtin_fminf, a, b); }
GLSL_PREFIX double [[spirv::GLSLstd450(37)]] min(double a, double b) noexcept { return                __builtin_fmin(  a, b); }
GLSL_PREFIX dvec2  [[spirv::GLSLstd450(37)]] min(dvec2  a, dvec2  b) noexcept { return __vector_apply(__builtin_fmin,  a, b); }
GLSL_PREFIX dvec3  [[spirv::GLSLstd450(37)]] min(dvec3  a, dvec3  b) noexcept { return __vector_apply(__builtin_fmin,  a, b); }
GLSL_PREFIX dvec4  [[spirv::GLSLstd450(37)]] min(dvec4  a, dvec4  b) noexcept { return __vector_apply(__builtin_fmin,  a, b); }
GLSL_PREFIX int    [[spirv::GLSLstd450(39)]] min(int    a, int    b) noexcept { return                __builtin_min(   a, b); }
GLSL_PREFIX ivec2  [[spirv::GLSLstd450(39)]] min(ivec2  a, ivec2  b) noexcept { return __vector_apply(__builtin_min,   a, b); }
GLSL_PREFIX ivec3  [[spirv::GLSLstd450(39)]] min(ivec3  a, ivec3  b) noexcept { return __vector_apply(__builtin_min,   a, b); }
GLSL_PREFIX ivec4  [[spirv::GLSLstd450(39)]] min(ivec4  a, ivec4  b) noexcept { return __vector_apply(__builtin_min,   a, b); }
GLSL_PREFIX uint   [[spirv::GLSLstd450(38)]] min(uint   a, uint   b) noexcept { return                __builtin_umin(  a, b); }
GLSL_PREFIX uvec2  [[spirv::GLSLstd450(38)]] min(uvec2  a, uvec2  b) noexcept { return __vector_apply(__builtin_umin,  a, b); }
GLSL_PREFIX uvec3  [[spirv::GLSLstd450(38)]] min(uvec3  a, uvec3  b) noexcept { return __vector_apply(__builtin_umin,  a, b); }
GLSL_PREFIX uvec4  [[spirv::GLSLstd450(38)]] min(uvec4  a, uvec4  b) noexcept { return __vector_apply(__builtin_umin,  a, b); }

GLSL_PREFIX float  [[spirv::GLSLstd450(40)]] max(float  a, float  b) noexcept { return                __builtin_fmaxf( a, b); }
GLSL_PREFIX vec2   [[spirv::GLSLstd450(40)]] max(vec2   a, vec2   b) noexcept { return __vector_apply(__builtin_fmaxf, a, b); }
GLSL_PREFIX vec3   [[spirv::GLSLstd450(40)]] max(vec3   a, vec3   b) noexcept { return __vector_apply(__builtin_fmaxf, a, b); }
GLSL_PREFIX vec4   [[spirv::GLSLstd450(40)]] max(vec4   a, vec4   b) noexcept { return __vector_apply(__builtin_fmaxf, a, b); }
GLSL_PREFIX double [[spirv::GLSLstd450(40)]] max(double a, double b) noexcept { return                __builtin_fmax(  a, b); }
GLSL_PREFIX dvec2  [[spirv::GLSLstd450(40)]] max(dvec2  a, dvec2  b) noexcept { return __vector_apply(__builtin_fmax,  a, b); }
GLSL_PREFIX dvec3  [[spirv::GLSLstd450(40)]] max(dvec3  a, dvec3  b) noexcept { return __vector_apply(__builtin_fmax,  a, b); }
GLSL_PREFIX dvec4  [[spirv::GLSLstd450(40)]] max(dvec4  a, dvec4  b) noexcept { return __vector_apply(__builtin_fmax,  a, b); }
GLSL_PREFIX int    [[spirv::GLSLstd450(42)]] max(int    a, int    b) noexcept { return                __builtin_max(   a, b); }
GLSL_PREFIX ivec2  [[spirv::GLSLstd450(42)]] max(ivec2  a, ivec2  b) noexcept { return __vector_apply(__builtin_max,   a, b); }
GLSL_PREFIX ivec3  [[spirv::GLSLstd450(42)]] max(ivec3  a, ivec3  b) noexcept { return __vector_apply(__builtin_max,   a, b); }
GLSL_PREFIX ivec4  [[spirv::GLSLstd450(42)]] max(ivec4  a, ivec4  b) noexcept { return __vector_apply(__builtin_max,   a, b); }
GLSL_PREFIX uint   [[spirv::GLSLstd450(41)]] max(uint   a, uint   b) noexcept { return                __builtin_umax(  a, b); }
GLSL_PREFIX uvec2  [[spirv::GLSLstd450(41)]] max(uvec2  a, uvec2  b) noexcept { return __vector_apply(__builtin_umax,  a, b); }
GLSL_PREFIX uvec3  [[spirv::GLSLstd450(41)]] max(uvec3  a, uvec3  b) noexcept { return __vector_apply(__builtin_umax,  a, b); }
GLSL_PREFIX uvec4  [[spirv::GLSLstd450(41)]] max(uvec4  a, uvec4  b) noexcept { return __vector_apply(__builtin_umax,  a, b); }

GLSL_PREFIX float  [[spirv::GLSLstd450(43)]] clamp(float  x, float  x0, float  x1) noexcept { return min(max(x, x0), x1); }
GLSL_PREFIX vec2   [[spirv::GLSLstd450(43)]] clamp(vec2   x, vec2   x0, vec2   x1) noexcept { return min(max(x, x0), x1); }
GLSL_PREFIX vec3   [[spirv::GLSLstd450(43)]] clamp(vec3   x, vec3   x0, vec3   x1) noexcept { return min(max(x, x0), x1); }
GLSL_PREFIX vec4   [[spirv::GLSLstd450(43)]] clamp(vec4   x, vec4   x0, vec4   x1) noexcept { return min(max(x, x0), x1); }
GLSL_PREFIX double [[spirv::GLSLstd450(43)]] clamp(double x, double x0, double x1) noexcept { return min(max(x, x0), x1); }
GLSL_PREFIX dvec2  [[spirv::GLSLstd450(43)]] clamp(dvec2  x, dvec2  x0, dvec2  x1) noexcept { return min(max(x, x0), x1); }
GLSL_PREFIX dvec3  [[spirv::GLSLstd450(43)]] clamp(dvec3  x, dvec3  x0, dvec3  x1) noexcept { return min(max(x, x0), x1); }
GLSL_PREFIX dvec4  [[spirv::GLSLstd450(43)]] clamp(dvec4  x, dvec4  x0, dvec4  x1) noexcept { return min(max(x, x0), x1); }
GLSL_PREFIX int    [[spirv::GLSLstd450(45)]] clamp(int    x, int    x0, int    x1) noexcept { return min(max(x, x0), x1); }
GLSL_PREFIX ivec2  [[spirv::GLSLstd450(45)]] clamp(ivec2  x, ivec2  x0, ivec2  x1) noexcept { return min(max(x, x0), x1); }
GLSL_PREFIX ivec3  [[spirv::GLSLstd450(45)]] clamp(ivec3  x, ivec3  x0, ivec3  x1) noexcept { return min(max(x, x0), x1); }
GLSL_PREFIX ivec4  [[spirv::GLSLstd450(45)]] clamp(ivec4  x, ivec4  x0, ivec4  x1) noexcept { return min(max(x, x0), x1); }
GLSL_PREFIX uint   [[spirv::GLSLstd450(44)]] clamp(uint   x, uint   x0, uint   x1) noexcept { return min(max(x, x0), x1); }
GLSL_PREFIX uvec2  [[spirv::GLSLstd450(44)]] clamp(uvec2  x, uvec2  x0, uvec2  x1) noexcept { return min(max(x, x0), x1); }
GLSL_PREFIX uvec3  [[spirv::GLSLstd450(44)]] clamp(uvec3  x, uvec3  x0, uvec3  x1) noexcept { return min(max(x, x0), x1); }
GLSL_PREFIX uvec4  [[spirv::GLSLstd450(44)]] clamp(uvec4  x, uvec4  x0, uvec4  x1) noexcept { return min(max(x, x0), x1); }

GLSL_PREFIX float  [[spirv::GLSLstd450(46)]] mix(float  x, float  y, float  a) noexcept { return x * (1 - a) + y * a; }
GLSL_PREFIX vec2   [[spirv::GLSLstd450(46)]] mix(vec2   x, vec2   y, vec2   a) noexcept { return x * (1 - a) + y * a; }
GLSL_PREFIX vec3   [[spirv::GLSLstd450(46)]] mix(vec3   x, vec3   y, vec3   a) noexcept { return x * (1 - a) + y * a; }
GLSL_PREFIX vec4   [[spirv::GLSLstd450(46)]] mix(vec4   x, vec4   y, vec4   a) noexcept { return x * (1 - a) + y * a; }
GLSL_PREFIX double [[spirv::GLSLstd450(46)]] mix(double x, double y, double a) noexcept { return x * (1 - a) + y * a; }
GLSL_PREFIX dvec2  [[spirv::GLSLstd450(46)]] mix(dvec2  x, dvec2  y, dvec2  a) noexcept { return x * (1 - a) + y * a; }
GLSL_PREFIX dvec3  [[spirv::GLSLstd450(46)]] mix(dvec3  x, dvec3  y, dvec3  a) noexcept { return x * (1 - a) + y * a; }
GLSL_PREFIX dvec4  [[spirv::GLSLstd450(46)]] mix(dvec4  x, dvec4  y, dvec4  a) noexcept { return x * (1 - a) + y * a; }

GLSL_PREFIX float  [[spirv::GLSLstd450(48)]] step(float  edge, float  x) noexcept { return x >= edge ? 1.f : 0.f; }
GLSL_PREFIX vec2   [[spirv::GLSLstd450(48)]] step(vec2   edge, vec2   x) noexcept { return x >= edge ? 1.f : 0.f; }
GLSL_PREFIX vec3   [[spirv::GLSLstd450(48)]] step(vec3   edge, vec3   x) noexcept { return x >= edge ? 1.f : 0.f; }
GLSL_PREFIX vec4   [[spirv::GLSLstd450(48)]] step(vec4   edge, vec4   x) noexcept { return x >= edge ? 1.f : 0.f; }
GLSL_PREFIX double [[spirv::GLSLstd450(48)]] step(double edge, double x) noexcept { return x >= edge ? 1.0 : 0.0; }
GLSL_PREFIX dvec2  [[spirv::GLSLstd450(48)]] step(dvec2  edge, dvec2  x) noexcept { return x >= edge ? 1.0 : 0.0; }
GLSL_PREFIX dvec3  [[spirv::GLSLstd450(48)]] step(dvec3  edge, dvec3  x) noexcept { return x >= edge ? 1.0 : 0.0; }
GLSL_PREFIX dvec4  [[spirv::GLSLstd450(48)]] step(dvec4  edge, dvec4  x) noexcept { return x >= edge ? 1.0 : 0.0; }

GLSL_PREFIX float  [[spirv::GLSLstd450(49)]] smoothstep(float  edge0, float  edge1, float  x) noexcept { float  t = clamp((x - edge0) / (edge1 - edge0), 0.f, 1.f); return t * t * (3 - 2 * t); }
GLSL_PREFIX vec2   [[spirv::GLSLstd450(49)]] smoothstep(vec2   edge0, vec2   edge1, vec2   x) noexcept { vec2   t = clamp((x - edge0) / (edge1 - edge0), 0.f, 1.f); return t * t * (3 - 2 * t); }
GLSL_PREFIX vec3   [[spirv::GLSLstd450(49)]] smoothstep(vec3   edge0, vec3   edge1, vec3   x) noexcept { vec3   t = clamp((x - edge0) / (edge1 - edge0), 0.f, 1.f); return t * t * (3 - 2 * t); }
GLSL_PREFIX vec4   [[spirv::GLSLstd450(49)]] smoothstep(vec4   edge0, vec4   edge1, vec4   x) noexcept { vec4   t = clamp((x - edge0) / (edge1 - edge0), 0.f, 1.f); return t * t * (3 - 2 * t); }
GLSL_PREFIX double [[spirv::GLSLstd450(49)]] smoothstep(double edge0, double edge1, double x) noexcept { double t = clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0); return t * t * (3 - 2 * t); }
GLSL_PREFIX dvec2  [[spirv::GLSLstd450(49)]] smoothstep(dvec2  edge0, dvec2  edge1, dvec2  x) noexcept { dvec2  t = clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0); return t * t * (3 - 2 * t); }
GLSL_PREFIX dvec3  [[spirv::GLSLstd450(49)]] smoothstep(dvec3  edge0, dvec3  edge1, dvec3  x) noexcept { dvec3  t = clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0); return t * t * (3 - 2 * t); }
GLSL_PREFIX dvec4  [[spirv::GLSLstd450(49)]] smoothstep(dvec4  edge0, dvec4  edge1, dvec4  x) noexcept { dvec4  t = clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0); return t * t * (3 - 2 * t); }

GLSL_PREFIX bool   [[spirv::opcode(156)]] isnan(float  a) noexcept { return                __builtin_isnanf( a); }
GLSL_PREFIX bvec2  [[spirv::opcode(156)]] isnan(vec2   a) noexcept { return __vector_apply(__builtin_isnanf, a); }
GLSL_PREFIX bvec3  [[spirv::opcode(156)]] isnan(vec3   a) noexcept { return __vector_apply(__builtin_isnanf, a); }
GLSL_PREFIX bvec4  [[spirv::opcode(156)]] isnan(vec4   a) noexcept { return __vector_apply(__builtin_isnanf, a); }
extern "C"  bool   [[spirv::opcode(156)]] isnan(double a) noexcept;
GLSL_PREFIX bvec2  [[spirv::opcode(156)]] isnan(dvec2  a) noexcept { return __vector_apply(__builtin_isnan,  a); }
GLSL_PREFIX bvec3  [[spirv::opcode(156)]] isnan(dvec3  a) noexcept { return __vector_apply(__builtin_isnan,  a); }
GLSL_PREFIX bvec4  [[spirv::opcode(156)]] isnan(dvec4  a) noexcept { return __vector_apply(__builtin_isnan,  a); }

GLSL_PREFIX bool   [[spirv::opcode(157)]] isinf(float  a) noexcept { return                __builtin_isinff( a); }
GLSL_PREFIX bvec2  [[spirv::opcode(157)]] isinf(vec2   a) noexcept { return __vector_apply(__builtin_isinff, a); }
GLSL_PREFIX bvec3  [[spirv::opcode(157)]] isinf(vec3   a) noexcept { return __vector_apply(__builtin_isinff, a); }
GLSL_PREFIX bvec4  [[spirv::opcode(157)]] isinf(vec4   a) noexcept { return __vector_apply(__builtin_isinff, a); }
extern "C"  bool   [[spirv::opcode(157)]] isinf(double a) noexcept;
GLSL_PREFIX bvec2  [[spirv::opcode(157)]] isinf(dvec2  a) noexcept { return __vector_apply(__builtin_isinf,  a); }
GLSL_PREFIX bvec3  [[spirv::opcode(157)]] isinf(dvec3  a) noexcept { return __vector_apply(__builtin_isinf,  a); }
GLSL_PREFIX bvec4  [[spirv::opcode(157)]] isinf(dvec4  a) noexcept { return __vector_apply(__builtin_isinf,  a); }

// https://www.khronos.org/registry/OpenGL/specs/gl/GLSLangSpec.4.60.html#floating-point-pack-and-unpack-functions
GLSL_PREFIX uint  [[spirv::GLSLstd450(54)]]   packSnorm4x8(vec4 v) noexcept;
GLSL_PREFIX uint  [[spirv::GLSLstd450(55)]]   packUnorm4x8(vec4 v) noexcept;
GLSL_PREFIX uint  [[spirv::GLSLstd450(56)]]   packSnorm2x16(vec2 v) noexcept;
GLSL_PREFIX uint  [[spirv::GLSLstd450(57)]]   packUnorm2x16(vec2 v) noexcept;
GLSL_PREFIX uint  [[spirv::GLSLstd450(58)]]   packHalf2x16(vec2 v) noexcept;
GLSL_PREFIX vec2  [[spirv::GLSLstd450(59)]]   packDouble2x32(uvec2 v) noexcept;
GLSL_PREFIX vec2  [[spirv::GLSLstd450(60)]] unpackSnorm2x16(uint p) noexcept;
GLSL_PREFIX vec2  [[spirv::GLSLstd450(61)]] unpackUnorm2x16(uint p) noexcept;
GLSL_PREFIX vec2  [[spirv::GLSLstd450(62)]] unpackHalf2x16(uint p) noexcept;
GLSL_PREFIX vec4  [[spirv::GLSLstd450(63)]] unpackSnorm4x8(uint p) noexcept;
GLSL_PREFIX vec4  [[spirv::GLSLstd450(64)]] unpackUnorm4x8(uint p) noexcept;
GLSL_PREFIX uvec2 [[spirv::GLSLstd450(65)]] unpackDouble2x32(double v) noexcept;

// https://www.khronos.org/registry/OpenGL/specs/gl/GLSLangSpec.4.60.html#geometric-functions
GLSL_PREFIX float  [[spirv::GLSLstd450(66)]] length(vec2  a) noexcept { return sqrt(__vector_dot(a, a)); }
GLSL_PREFIX float  [[spirv::GLSLstd450(66)]] length(vec3  a) noexcept { return sqrt(__vector_dot(a, a)); }
GLSL_PREFIX float  [[spirv::GLSLstd450(66)]] length(vec4  a) noexcept { return sqrt(__vector_dot(a, a)); }
GLSL_PREFIX double [[spirv::GLSLstd450(66)]] length(dvec2 a) noexcept { return sqrt(__vector_dot(a, a)); }
GLSL_PREFIX double [[spirv::GLSLstd450(66)]] length(dvec3 a) noexcept { return sqrt(__vector_dot(a, a)); }
GLSL_PREFIX double [[spirv::GLSLstd450(66)]] length(dvec4 a) noexcept { return sqrt(__vector_dot(a, a)); }

GLSL_PREFIX float  [[spirv::GLSLstd450(67)]] distance(vec2  a, vec2  b) noexcept { a -= b; return sqrt(__vector_dot(a, a)); }
GLSL_PREFIX float  [[spirv::GLSLstd450(67)]] distance(vec3  a, vec3  b) noexcept { a -= b; return sqrt(__vector_dot(a, a)); }
GLSL_PREFIX float  [[spirv::GLSLstd450(67)]] distance(vec4  a, vec4  b) noexcept { a -= b; return sqrt(__vector_dot(a, a)); }
GLSL_PREFIX double [[spirv::GLSLstd450(67)]] distance(dvec2 a, dvec2 b) noexcept { a -= b; return sqrt(__vector_dot(a, a)); }
GLSL_PREFIX double [[spirv::GLSLstd450(67)]] distance(dvec3 a, dvec3 b) noexcept { a -= b; return sqrt(__vector_dot(a, a)); }
GLSL_PREFIX double [[spirv::GLSLstd450(67)]] distance(dvec4 a, dvec4 b) noexcept { a -= b; return sqrt(__vector_dot(a, a)); }

GLSL_PREFIX float  [[spirv::opcode(148)]] dot(vec2  a, vec2  b) noexcept { return __vector_dot(a, b); }
GLSL_PREFIX float  [[spirv::opcode(148)]] dot(vec3  a, vec3  b) noexcept { return __vector_dot(a, b); }
GLSL_PREFIX float  [[spirv::opcode(148)]] dot(vec4  a, vec4  b) noexcept { return __vector_dot(a, b); }
GLSL_PREFIX double [[spirv::opcode(148)]] dot(dvec2 a, dvec2 b) noexcept { return __vector_dot(a, b); }
GLSL_PREFIX double [[spirv::opcode(148)]] dot(dvec3 a, dvec3 b) noexcept { return __vector_dot(a, b); }
GLSL_PREFIX double [[spirv::opcode(148)]] dot(dvec4 a, dvec4 b) noexcept { return __vector_dot(a, b); }

GLSL_PREFIX vec3  [[spirv::GLSLstd450(68)]] cross(vec3  a, vec3  b) noexcept { return a.yzx * b.zxy - b.yzx * a.zxy; }
GLSL_PREFIX dvec3 [[spirv::GLSLstd450(68)]] cross(dvec3 a, dvec3 b) noexcept { return a.yzx * b.zxy - b.yzx * a.zxy; }

GLSL_PREFIX vec2  [[spirv::GLSLstd450(69)]] normalize(vec2  a) noexcept { return inversesqrt(__vector_dot(a, a)) * a; }
GLSL_PREFIX vec3  [[spirv::GLSLstd450(69)]] normalize(vec3  a) noexcept { return inversesqrt(__vector_dot(a, a)) * a; }
GLSL_PREFIX vec4  [[spirv::GLSLstd450(69)]] normalize(vec4  a) noexcept { return inversesqrt(__vector_dot(a, a)) * a; }
GLSL_PREFIX dvec2 [[spirv::GLSLstd450(69)]] normalize(dvec2 a) noexcept { return inversesqrt(__vector_dot(a, a)) * a; }
GLSL_PREFIX dvec3 [[spirv::GLSLstd450(69)]] normalize(dvec3 a) noexcept { return inversesqrt(__vector_dot(a, a)) * a; }
GLSL_PREFIX dvec4 [[spirv::GLSLstd450(69)]] normalize(dvec4 a) noexcept { return inversesqrt(__vector_dot(a, a)) * a; }

GLSL_PREFIX vec3  [[spirv::GLSLstd450(70)]] faceforward(vec3  N, vec3  I, vec3  Nref) noexcept { return dot(Nref, I) < 0 ? N : -N; }
GLSL_PREFIX dvec3 [[spirv::GLSLstd450(70)]] faceforward(dvec3 N, dvec3 I, dvec3 Nref) noexcept { return dot(Nref, I) < 0 ? N : -N; }

GLSL_PREFIX vec3  [[spirv::GLSLstd450(71)]] reflect(vec3   I, vec3   N) noexcept { return I - 2 * dot(N, I) * N; }
GLSL_PREFIX dvec3 [[spirv::GLSLstd450(71)]] reflect(dvec3  I, dvec3  N) noexcept { return I - 2 * dot(N, I) * N; }

GLSL_PREFIX vec3  [[spirv::GLSLstd450(72)]] refract(vec3   I, vec3   N, float  eta) noexcept { 
  float dotNI = dot(N, I); 
  float k = 1 - eta * eta * (1 - dotNI); 
  return k < 0 ? 0 : eta * I - (eta * dotNI + sqrt(k)) * N; 
}
GLSL_PREFIX dvec3 [[spirv::GLSLstd450(72)]] refract(dvec3  I, dvec3  N, double eta) noexcept { 
  double dotNI = dot(N, I); 
  double k = 1 - eta * eta * (1 - dotNI); 
  return k < 0 ? 0 : eta * I - (eta * dotNI + sqrt(k)) * N; 
}

// https://www.khronos.org/registry/OpenGL/specs/gl/GLSLangSpec.4.60.html#matrix-functions

GLSL_PREFIX mat2   [[spirv::opcode(84)]] transpose(mat2   m) noexcept { return __matrix_transpose(m); }
GLSL_PREFIX mat3   [[spirv::opcode(84)]] transpose(mat3   m) noexcept { return __matrix_transpose(m); }
GLSL_PREFIX mat4   [[spirv::opcode(84)]] transpose(mat4   m) noexcept { return __matrix_transpose(m); }
GLSL_PREFIX mat2x3 [[spirv::opcode(84)]] transpose(mat3x2 m) noexcept { return __matrix_transpose(m); }
GLSL_PREFIX mat3x2 [[spirv::opcode(84)]] transpose(mat2x3 m) noexcept { return __matrix_transpose(m); }
GLSL_PREFIX mat2x4 [[spirv::opcode(84)]] transpose(mat4x2 m) noexcept { return __matrix_transpose(m); }
GLSL_PREFIX mat4x2 [[spirv::opcode(84)]] transpose(mat2x4 m) noexcept { return __matrix_transpose(m); }
GLSL_PREFIX mat3x4 [[spirv::opcode(84)]] transpose(mat4x3 m) noexcept { return __matrix_transpose(m); }
GLSL_PREFIX mat4x3 [[spirv::opcode(84)]] transpose(mat3x4 m) noexcept { return __matrix_transpose(m); }

// For now these are only forward declarations. They'll compile for shaders,
// but fail to link for host code.
GLSL_PREFIX float  [[spirv::GLSLstd450(33)]] determinant(mat2  m) noexcept;
GLSL_PREFIX float  [[spirv::GLSLstd450(33)]] determinant(mat3  m) noexcept;
GLSL_PREFIX float  [[spirv::GLSLstd450(33)]] determinant(mat4  m) noexcept;
GLSL_PREFIX double [[spirv::GLSLstd450(33)]] determinant(dmat2 m) noexcept;
GLSL_PREFIX double [[spirv::GLSLstd450(33)]] determinant(dmat3 m) noexcept;
GLSL_PREFIX double [[spirv::GLSLstd450(33)]] determinant(dmat4 m) noexcept;

GLSL_PREFIX mat2  [[spirv::GLSLstd450(34)]] inverse(mat2  m) noexcept;
GLSL_PREFIX mat3  [[spirv::GLSLstd450(34)]] inverse(mat3  m) noexcept;
GLSL_PREFIX mat4  [[spirv::GLSLstd450(34)]] inverse(mat4  m) noexcept;
GLSL_PREFIX dmat2 [[spirv::GLSLstd450(34)]] inverse(dmat2 m) noexcept;
GLSL_PREFIX dmat3 [[spirv::GLSLstd450(34)]] inverse(dmat3 m) noexcept;
GLSL_PREFIX dmat4 [[spirv::GLSLstd450(34)]] inverse(dmat4 m) noexcept;

GLSL_PREFIX bool   [[spirv::opcode(154)]] any(bvec2 x) noexcept { return __vector_any(x); }
GLSL_PREFIX bool   [[spirv::opcode(154)]] any(bvec3 x) noexcept { return __vector_any(x); }
GLSL_PREFIX bool   [[spirv::opcode(154)]] any(bvec4 x) noexcept { return __vector_any(x); }
GLSL_PREFIX bool   [[spirv::opcode(155)]] all(bvec2 x) noexcept { return __vector_all(x); }
GLSL_PREFIX bool   [[spirv::opcode(155)]] all(bvec3 x) noexcept { return __vector_all(x); }
GLSL_PREFIX bool   [[spirv::opcode(155)]] all(bvec4 x) noexcept { return __vector_all(x); }

// 8.8 Integer Functions
GLSL_PREFIX int    [[spirv::opcode(205)]] bitCount(int   x) noexcept { return                __builtin_popcount( (uint) x); }
GLSL_PREFIX ivec2  [[spirv::opcode(205)]] bitCount(ivec2 x) noexcept { return __vector_apply(__builtin_popcount, (uvec2)x); }
GLSL_PREFIX ivec3  [[spirv::opcode(205)]] bitCount(ivec3 x) noexcept { return __vector_apply(__builtin_popcount, (uvec3)x); }
GLSL_PREFIX ivec4  [[spirv::opcode(205)]] bitCount(ivec4 x) noexcept { return __vector_apply(__builtin_popcount, (uvec4)x); }
GLSL_PREFIX int    [[spirv::opcode(205)]] bitCount(uint  x) noexcept { return                __builtin_popcount( x); }
GLSL_PREFIX ivec2  [[spirv::opcode(205)]] bitCount(uvec2 x) noexcept { return __vector_apply(__builtin_popcount, x); }
GLSL_PREFIX ivec3  [[spirv::opcode(205)]] bitCount(uvec3 x) noexcept { return __vector_apply(__builtin_popcount, x); }
GLSL_PREFIX ivec4  [[spirv::opcode(205)]] bitCount(uvec4 x) noexcept { return __vector_apply(__builtin_popcount, x); }

GLSL_PREFIX [[spirv::builtin]] int   floatBitsToInt(float x) noexcept { return *reinterpret_cast<const int*>(&x);      }
GLSL_PREFIX [[spirv::builtin]] ivec2 floatBitsToInt(vec2  x) noexcept { return __vector_apply(floatBitsToInt(0.f), x); }
GLSL_PREFIX [[spirv::builtin]] ivec3 floatBitsToInt(vec3  x) noexcept { return __vector_apply(floatBitsToInt(0.f), x); }
GLSL_PREFIX [[spirv::builtin]] ivec4 floatBitsToInt(vec4  x) noexcept { return __vector_apply(floatBitsToInt(0.f), x); }

GLSL_PREFIX [[spirv::builtin]] uint  floatBitsToUint(float x) noexcept { return *reinterpret_cast<const uint*>(&x);      }
GLSL_PREFIX [[spirv::builtin]] uvec2 floatBitsToUint(vec2  x) noexcept { return __vector_apply(floatBitsToUint(0.f), x); }
GLSL_PREFIX [[spirv::builtin]] uvec3 floatBitsToUint(vec3  x) noexcept { return __vector_apply(floatBitsToUint(0.f), x); }
GLSL_PREFIX [[spirv::builtin]] uvec4 floatBitsToUint(vec4  x) noexcept { return __vector_apply(floatBitsToUint(0.f), x); }

GLSL_PREFIX [[spirv::builtin]] float intBitsToFloat(int   x) noexcept { return *reinterpret_cast<const float*>(&x);  }
GLSL_PREFIX [[spirv::builtin]] vec2  intBitsToFloat(ivec2 x) noexcept { return __vector_apply(intBitsToFloat(0), x); }
GLSL_PREFIX [[spirv::builtin]] vec3  intBitsToFloat(ivec3 x) noexcept { return __vector_apply(intBitsToFloat(0), x); }
GLSL_PREFIX [[spirv::builtin]] vec4  intBitsToFloat(ivec4 x) noexcept { return __vector_apply(intBitsToFloat(0), x); }

GLSL_PREFIX [[spirv::builtin]] float uintBitsToFloat(uint  x) noexcept { return *reinterpret_cast<const float*>(&x);   }
GLSL_PREFIX [[spirv::builtin]] vec2  uintBitsToFloat(uvec2 x) noexcept { return __vector_apply(uintBitsToFloat(0u), x); }
GLSL_PREFIX [[spirv::builtin]] vec3  uintBitsToFloat(uvec3 x) noexcept { return __vector_apply(uintBitsToFloat(0u), x); }
GLSL_PREFIX [[spirv::builtin]] vec4  uintBitsToFloat(uvec4 x) noexcept { return __vector_apply(uintBitsToFloat(0u), x); }

GLSL_PREFIX float  [[spirv::GLSLstd450(51)]] frexp(float  x, int*   y) noexcept { return                __builtin_frexpf( x, y); }
GLSL_PREFIX vec2   [[spirv::GLSLstd450(51)]] frexp(vec2   x, ivec2* y) noexcept { return __vector_apply(__builtin_frexpf, x, y); }
GLSL_PREFIX vec3   [[spirv::GLSLstd450(51)]] frexp(vec3   x, ivec3* y) noexcept { return __vector_apply(__builtin_frexpf, x, y); }
GLSL_PREFIX vec4   [[spirv::GLSLstd450(51)]] frexp(vec4   x, ivec4* y) noexcept { return __vector_apply(__builtin_frexpf, x, y); }
extern "C"  double [[spirv::GLSLstd450(51)]] frexp(double x, int*   y) noexcept;
GLSL_PREFIX dvec2  [[spirv::GLSLstd450(51)]] frexp(dvec2  x, ivec2* y) noexcept { return __vector_apply(__builtin_frexp,  x, y); }
GLSL_PREFIX dvec3  [[spirv::GLSLstd450(51)]] frexp(dvec3  x, ivec3* y) noexcept { return __vector_apply(__builtin_frexp,  x, y); }
GLSL_PREFIX dvec4  [[spirv::GLSLstd450(51)]] frexp(dvec4  x, ivec4* y) noexcept { return __vector_apply(__builtin_frexp,  x, y); }

GLSL_PREFIX float  [[spirv::GLSLstd450(53)]] ldexp(float  x, int   y) noexcept { return                __builtin_ldexpf( x, y); }
GLSL_PREFIX vec2   [[spirv::GLSLstd450(53)]] ldexp(vec2   x, ivec2 y) noexcept { return __vector_apply(__builtin_ldexpf, x, y); }
GLSL_PREFIX vec3   [[spirv::GLSLstd450(53)]] ldexp(vec3   x, ivec3 y) noexcept { return __vector_apply(__builtin_ldexpf, x, y); }
GLSL_PREFIX vec4   [[spirv::GLSLstd450(53)]] ldexp(vec4   x, ivec4 y) noexcept { return __vector_apply(__builtin_ldexpf, x, y); }
extern "C"  double [[spirv::GLSLstd450(53)]] ldexp(double x, int   y) noexcept;
GLSL_PREFIX dvec2  [[spirv::GLSLstd450(53)]] ldexp(dvec2  x, ivec2 y) noexcept { return __vector_apply(__builtin_ldexp,  x, y); }
GLSL_PREFIX dvec3  [[spirv::GLSLstd450(53)]] ldexp(dvec3  x, ivec3 y) noexcept { return __vector_apply(__builtin_ldexp,  x, y); }
GLSL_PREFIX dvec4  [[spirv::GLSLstd450(53)]] ldexp(dvec4  x, ivec4 y) noexcept { return __vector_apply(__builtin_ldexp,  x, y); }



////////////////////////////////////////////////////////////////////////////////

enum class gl_layout_t : unsigned {
  std140,
  std430,
  scalar,
};

// https://www.khronos.org/registry/OpenGL/specs/gl/GLSLangSpec.4.60.html#built-in-variables
struct [[spirv::block]] gl_PerVertex {
  [[spirv::builtin(0)]] vec4  Position;
  [[spirv::builtin(1)]] float PointSize;
  [[spirv::builtin(3)]] float ClipDistance[4];
  [[spirv::builtin(4)]] float CullDistance[4];
};

// Vertex variables.
[[spirv::builtin(5)   ]] int          glvert_VertexID;      // GL only
[[spirv::builtin(42)  ]] int          glvert_VertexIndex;   // Vulkan only
[[spirv::builtin(6)   ]] int          glvert_InstanceID;    // GL only
[[spirv::builtin(43)  ]] int          glvert_InstanceIndex; // Vulkan only
[[spirv::builtin(4426)]] int          glvert_DrawID;
[[spirv::builtin(4424)]] int          glvert_BaseVertex;
[[spirv::builtin(4425)]] int          glvert_BaseInstance;

[[spirv::out]] gl_PerVertex glvert_Output;

// Tessellation control variables.
[[spirv::builtin(14)]] int          gltesc_PatchVerticesIn;
[[spirv::builtin(7) ]] int          gltesc_PrimitiveID;
[[spirv::builtin(8) ]] int          gltesc_InvocationID;

[[spirv::builtin(11), spirv::out, spirv::patch]] float gltesc_LevelOuter[4]; // output
[[spirv::builtin(12), spirv::out, spirv::patch]] float gltesc_LevelInner[2]; // output

[[spirv::out]] gl_PerVertex gltesc_Output[16];
[[spirv::in ]] gl_PerVertex gltesc_Input[32];

// OpControlBarrier
[[spirv::builtin]] void         gltesc_barrier();

// Tessellation evaluation variables.
[[spirv::in     ]] gl_PerVertex gltese_Input[32];
[[spirv::builtin(14)]] int          gltese_PatchVerticesIn;
[[spirv::builtin(7) ]] int          gltese_PrimitiveID;
[[spirv::builtin(13)]] vec3         gltese_TessCoord;
[[spirv::builtin(11)]] float        gltese_LevelOuter[4];
[[spirv::builtin(12)]] float        gltese_LevelInner[2];

[[spirv::out]] gl_PerVertex gltese_Output;        // output

enum class gltese_primitive_t : unsigned {
  triangles,
  quads,
  isolines,
};

enum class gltese_spacing_t : unsigned {
  equal,
  fractional_even,
  fractional_odd,
};

enum class gltese_ordering_t : unsigned {
 cw,
 ccw, 
};

// Geometry variables.
[[spirv::in]] gl_PerVertex glgeom_Input[6];
[[spirv::out]] gl_PerVertex glgeom_Output;

[[spirv::builtin(7)             ]] int          glgeom_PrimitiveIDIn;
[[spirv::builtin(8)             ]] int          glgeom_InvocationID;
[[spirv::builtin(7),  spirv::out]] int          glgeom_PrimitiveID;   // output
[[spirv::builtin(9),  spirv::out]] int          glgeom_Layer;         // output
[[spirv::builtin(10), spirv::out]] int          glgeom_ViewportIndex; // output
[[spirv::builtin]] void         glgeom_EmitVertex() noexcept;
[[spirv::builtin]] void         glgeom_EndPrimitive() noexcept;
[[spirv::builtin]] void         glgeom_EmitStreamVertex(int stream) noexcept;
[[spirv::builtin]] void         glgeom_EndStreamPrimitive(int stream) noexcept;

enum class glgeom_input_t : unsigned {
  points,
  lines,
  lines_adjacency,
  triangles,
  triangles_adjacency,
};

enum class glgeom_output_t : unsigned {
  points,
  line_strip,
  triangle_strip,
};

// Fragment variables.
[[spirv::builtin(15)]] vec4  glfrag_FragCoord;
[[spirv::builtin(17)]] bool  glfrag_FrontFacing;
[[spirv::builtin(3) ]] float glfrag_ClipDistance[4];
[[spirv::builtin(4) ]] float glfrag_CullDistance[4];
[[spirv::builtin(16)]] vec2  glfrag_PointCoord;
[[spirv::builtin(7) ]] int   glfrag_PrimitiveID;
[[spirv::builtin(18)]] int   glfrag_SampleID;
[[spirv::builtin(19)]] vec2  glfrag_SamplePosition;
[[spirv::builtin(20)]] int   glfrag_SampleMaskIn[1];
[[spirv::builtin(9) ]] int   glfrag_Layer;
[[spirv::builtin(10)]] int   glfrag_ViewportIndex;
[[spirv::builtin(23)]] bool  glfrag_HelperInvocation;

[[spirv::builtin(22), spirv::out]] float glfrag_FragDepth;
[[spirv::builtin(2),  spirv::out]] int   glfrag_SampleMask[1];

[[spirv::builtin]] void         glfrag_discard() noexcept;

[[spirv::opcode(207)]] float        glfrag_dFdx(float x) noexcept;
[[spirv::opcode(207)]] vec2         glfrag_dFdx(vec2 x)  noexcept;
[[spirv::opcode(207)]] vec3         glfrag_dFdx(vec3 x)  noexcept;
[[spirv::opcode(207)]] vec4         glfrag_dFdx(vec4 x)  noexcept;

[[spirv::opcode(208)]] float        glfrag_dFdy(float x) noexcept;
[[spirv::opcode(208)]] vec2         glfrag_dFdy(vec2 x)  noexcept;
[[spirv::opcode(208)]] vec3         glfrag_dFdy(vec3 x)  noexcept;
[[spirv::opcode(208)]] vec4         glfrag_dFdy(vec4 x)  noexcept;

[[spirv::opcode(209)]] float        glfrag_fwidth(float x) noexcept;
[[spirv::opcode(209)]] vec2         glfrag_fwidth(vec2 x)  noexcept;
[[spirv::opcode(209)]] vec3         glfrag_fwidth(vec3 x)  noexcept;
[[spirv::opcode(209)]] vec4         glfrag_fwidth(vec4 x)  noexcept;

[[spirv::opcode(213)]] float        glfrag_dFdxCoarse(float x) noexcept;
[[spirv::opcode(213)]] vec2         glfrag_dFdxCoarse(vec2 x)  noexcept;
[[spirv::opcode(213)]] vec3         glfrag_dFdxCoarse(vec3 x)  noexcept;
[[spirv::opcode(213)]] vec4         glfrag_dFdxCoarse(vec4 x)  noexcept;

[[spirv::opcode(214)]] float        glfrag_dFdyCoarse(float x) noexcept;
[[spirv::opcode(214)]] vec2         glfrag_dFdyCoarse(vec2 x)  noexcept;
[[spirv::opcode(214)]] vec3         glfrag_dFdyCoarse(vec3 x)  noexcept;
[[spirv::opcode(214)]] vec4         glfrag_dFdyCoarse(vec4 x)  noexcept;

[[spirv::opcode(215)]] float        glfrag_fwidthCoarse(float x) noexcept;
[[spirv::opcode(215)]] vec2         glfrag_fwidthCoarse(vec2 x)  noexcept;
[[spirv::opcode(215)]] vec3         glfrag_fwidthCoarse(vec3 x)  noexcept;
[[spirv::opcode(215)]] vec4         glfrag_fwidthCoarse(vec4 x)  noexcept;

[[spirv::opcode(210)]] float        glfrag_dFdxFine(float x) noexcept;
[[spirv::opcode(210)]] vec2         glfrag_dFdxFine(vec2 x)  noexcept;
[[spirv::opcode(210)]] vec3         glfrag_dFdxFine(vec3 x)  noexcept;
[[spirv::opcode(210)]] vec4         glfrag_dFdxFine(vec4 x)  noexcept;

[[spirv::opcode(211)]] float        glfrag_dFdyFine(float x) noexcept;
[[spirv::opcode(211)]] vec2         glfrag_dFdyFine(vec2 x)  noexcept;
[[spirv::opcode(211)]] vec3         glfrag_dFdyFine(vec3 x)  noexcept;
[[spirv::opcode(211)]] vec4         glfrag_dFdyFine(vec4 x)  noexcept;

[[spirv::opcode(212)]] float        glfrag_fwidthFine(float x) noexcept;
[[spirv::opcode(212)]] vec2         glfrag_fwidthFine(vec2 x)  noexcept;
[[spirv::opcode(212)]] vec3         glfrag_fwidthFine(vec3 x)  noexcept;
[[spirv::opcode(212)]] vec4         glfrag_fwidthFine(vec4 x)  noexcept;

enum class glfrag_origin_t : unsigned {
  lower_left,   // OpenGL style
  upper_left,   // Vulkan style
};

// Compute shader variables.
[[spirv::builtin(24)]] uvec3    glcomp_NumWorkGroups;
[[spirv::builtin(25)]] uvec3    glcomp_WorkGroupSize;
[[spirv::builtin(26)]] uvec3    glcomp_WorkGroupID;
[[spirv::builtin(27)]] uvec3    glcomp_LocalInvocationID;
[[spirv::builtin(28)]] uvec3    glcomp_GlobalInvocationID;
[[spirv::builtin(29)]] unsigned glcomp_LocalInvocationIndex;

[[spirv::builtin(24)]] uvec3    gridDim;    // glcomp_NumWorkGroups
[[spirv::builtin(25)]] uvec3    blockDim;   // glcomp_WorkGroupSize
[[spirv::builtin(26)]] uvec3    blockIdx;   // glcomp_WorkGroupID
[[spirv::builtin(27)]] uvec3    threadIdx;  // glcomp_LocalInvocationID

// NOTE: __synchreads is aliased to glcomp_barrier.
[[spirv::builtin]] void         glcomp_barrier();

////////////////////////////////////////////////////////////////////////////////
// GLSL_NV_mesh_shader

[[spirv::out]]
gl_PerVertex glmesh_Output[126];

// Task shader variables.
[[spirv::builtin(5274), spirv::out]] uint glmesh_TaskCount;

// Mesh shader variables.
enum class glmesh_output_t : unsigned {
  points,
  lines,
  triangles,
};

[[spirv::builtin(5280)]] uint glmesh_ViewCount;
[[spirv::builtin(5281)]] uint glmesh_ViewIndices[];

[[spirv::builtin(5275), spirv::out]] uint glmesh_PrimitiveCount;
[[spirv::builtin(5275), spirv::out]] uint glmesh_PrimitiveIndices[];

[[spirv::opcode(5299)]] uint glmesh_writePackedPrimitiveIndices4x8(
  uint indexOffset, uint packedIndices);

// Floating-point opaque types.
enum class [[spirv::builtin]] sampler1D              : long long;
enum class [[spirv::builtin]] sampler1DShadow        : long long;
enum class [[spirv::builtin]] sampler1DArray         : long long;
enum class [[spirv::builtin]] sampler1DArrayShadow   : long long;
 
enum class [[spirv::builtin]] sampler2D              : long long;
enum class [[spirv::builtin]] sampler2DShadow        : long long;
enum class [[spirv::builtin]] sampler2DArray         : long long;
enum class [[spirv::builtin]] sampler2DArrayShadow   : long long;
enum class [[spirv::builtin]] sampler2DMS            : long long;
enum class [[spirv::builtin]] sampler2DMSArray       : long long;
enum class [[spirv::builtin]] sampler2DRect          : long long;
enum class [[spirv::builtin]] sampler2DRectShadow    : long long;
 
enum class [[spirv::builtin]] sampler3D              : long long;
 
enum class [[spirv::builtin]] samplerCube            : long long;
enum class [[spirv::builtin]] samplerCubeShadow      : long long;
enum class [[spirv::builtin]] samplerCubeArray       : long long;
enum class [[spirv::builtin]] samplerCubeArrayShadow : long long;
 
enum class [[spirv::builtin]] samplerBuffer          : long long;

// Signed integer opaque types.
enum class [[spirv::builtin]] isampler1D             : long long;
enum class [[spirv::builtin]] isampler1DArray        : long long;

enum class [[spirv::builtin]] isampler2D             : long long;
enum class [[spirv::builtin]] isampler2DArray        : long long;
enum class [[spirv::builtin]] isampler2DMS           : long long;
enum class [[spirv::builtin]] isampler2DMSArray      : long long;
enum class [[spirv::builtin]] isampler2DRect         : long long;

enum class [[spirv::builtin]] isampler3D             : long long;
enum class [[spirv::builtin]] isamplerCube           : long long;
enum class [[spirv::builtin]] isamplerCubeArray      : long long;

enum class [[spirv::builtin]] isamplerBuffer         : long long;

// Unsigned integer opaque types.
enum class [[spirv::builtin]] usampler1D             : long long;
enum class [[spirv::builtin]] usampler1DArray        : long long;

enum class [[spirv::builtin]] usampler2D             : long long;
enum class [[spirv::builtin]] usampler2DArray        : long long;
enum class [[spirv::builtin]] usampler2DMS           : long long;
enum class [[spirv::builtin]] usampler2DMSArray      : long long;
enum class [[spirv::builtin]] usampler2DRect         : long long;

enum class [[spirv::builtin]] usampler3D             : long long;
enum class [[spirv::builtin]] utexture3D             : long long;
enum class [[spirv::builtin]] uimage3D               : long long;

enum class [[spirv::builtin]] usamplerCube           : long long;
enum class [[spirv::builtin]] usamplerCubeArray      : long long;

enum class [[spirv::builtin]] usamplerBuffer         : long long;

////////////////////////////////////////////////////////////////////////////////
// 8.9.1. Texture Query Functions

// textureSize
[[spirv::builtin]] int   textureSize(sampler1D sampler, int lod) noexcept;
[[spirv::builtin]] ivec2 textureSize(sampler2D sampler, int lod) noexcept;
[[spirv::builtin]] ivec3 textureSize(sampler3D sampler, int lod) noexcept;
[[spirv::builtin]] ivec2 textureSize(samplerCube sampler, int lod) noexcept;
[[spirv::builtin]] int   textureSize(isampler1D sampler, int lod) noexcept;
[[spirv::builtin]] ivec2 textureSize(isampler2D sampler, int lod) noexcept;
[[spirv::builtin]] ivec3 textureSize(isampler3D sampler, int lod) noexcept;
[[spirv::builtin]] ivec2 textureSize(isamplerCube sampler, int lod) noexcept;
[[spirv::builtin]] int   textureSize(usampler1D sampler, int lod) noexcept;
[[spirv::builtin]] ivec2 textureSize(usampler2D sampler, int lod) noexcept;
[[spirv::builtin]] ivec3 textureSize(usampler3D sampler, int lod) noexcept;
[[spirv::builtin]] ivec2 textureSize(usamplerCube sampler, int lod) noexcept;
[[spirv::builtin]] int   textureSize(sampler1DShadow sampler, int lod) noexcept;
[[spirv::builtin]] ivec2 textureSize(sampler2DShadow sampler, int lod) noexcept;
[[spirv::builtin]] ivec2 textureSize(samplerCubeShadow sampler, int lod) noexcept;
[[spirv::builtin]] ivec3 textureSize(samplerCubeArray sampler, int lod) noexcept;
[[spirv::builtin]] ivec3 textureSize(isamplerCubeArray sampler, int lod) noexcept;
[[spirv::builtin]] ivec3 textureSize(usamplerCubeArray sampler, int lod) noexcept;
[[spirv::builtin]] ivec3 textureSize(samplerCubeArrayShadow sampler, int lod) noexcept;
[[spirv::builtin]] ivec2 textureSize(sampler2DRect sampler) noexcept;
[[spirv::builtin]] ivec2 textureSize(isampler2DRect sampler) noexcept;
[[spirv::builtin]] ivec2 textureSize(usampler2DRect sampler) noexcept;
[[spirv::builtin]] ivec2 textureSize(sampler2DRectShadow sampler) noexcept;
[[spirv::builtin]] ivec2 textureSize(sampler1DArray sampler, int lod) noexcept;
[[spirv::builtin]] ivec2 textureSize(isampler1DArray sampler, int lod) noexcept;
[[spirv::builtin]] ivec2 textureSize(usampler1DArray sampler, int lod) noexcept;
[[spirv::builtin]] ivec2 textureSize(sampler1DArrayShadow sampler, int lod) noexcept;
[[spirv::builtin]] ivec3 textureSize(sampler2DArray sampler, int lod) noexcept;
[[spirv::builtin]] ivec3 textureSize(isampler2DArray sampler, int lod) noexcept;
[[spirv::builtin]] ivec3 textureSize(usampler2DArray sampler, int lod) noexcept;
[[spirv::builtin]] ivec3 textureSize(sampler2DArrayShadow sampler, int lod) noexcept;
[[spirv::builtin]] int   textureSize(samplerBuffer sampler) noexcept;
[[spirv::builtin]] int   textureSize(isamplerBuffer sampler) noexcept;
[[spirv::builtin]] int   textureSize(usamplerBuffer sampler) noexcept;
[[spirv::builtin]] ivec2 textureSize(sampler2DMS sampler) noexcept;
[[spirv::builtin]] ivec2 textureSize(isampler2DMS sampler) noexcept;
[[spirv::builtin]] ivec2 textureSize(usampler2DMS sampler) noexcept;
[[spirv::builtin]] ivec3 textureSize(sampler2DMSArray sampler) noexcept;
[[spirv::builtin]] ivec3 textureSize(isampler2DMSArray sampler) noexcept;
[[spirv::builtin]] ivec3 textureSize(usampler2DMSArray sampler) noexcept;

// textureQueryLod
[[spirv::builtin]] vec2 textureQueryLod(sampler1D sampler,              float P) noexcept;
[[spirv::builtin]] vec2 textureQueryLod(isampler1D sampler,             float P) noexcept;
[[spirv::builtin]] vec2 textureQueryLod(usampler1D sampler,             float P) noexcept;

[[spirv::builtin]] vec2 textureQueryLod(sampler2D sampler,              vec2 P) noexcept;
[[spirv::builtin]] vec2 textureQueryLod(isampler2D sampler,             vec2 P) noexcept;
[[spirv::builtin]] vec2 textureQueryLod(usampler2D sampler,             vec2 P) noexcept;

[[spirv::builtin]] vec2 textureQueryLod(sampler3D sampler,              vec3 P) noexcept;
[[spirv::builtin]] vec2 textureQueryLod(isampler3D sampler,             vec3 P) noexcept;
[[spirv::builtin]] vec2 textureQueryLod(usampler3D sampler,             vec3 P) noexcept;

[[spirv::builtin]] vec2 textureQueryLod(samplerCube sampler,            vec3 P) noexcept;
[[spirv::builtin]] vec2 textureQueryLod(isamplerCube sampler,           vec3 P) noexcept;
[[spirv::builtin]] vec2 textureQueryLod(usamplerCube sampler,           vec3 P) noexcept;

[[spirv::builtin]] vec2 textureQueryLod(sampler1DArray sampler,         float P) noexcept;
[[spirv::builtin]] vec2 textureQueryLod(isampler1DArray sampler,        float P) noexcept;
[[spirv::builtin]] vec2 textureQueryLod(usampler1DArray sampler,        float P) noexcept;

[[spirv::builtin]] vec2 textureQueryLod(sampler2DArray sampler,         vec2 P) noexcept;
[[spirv::builtin]] vec2 textureQueryLod(isampler2DArray sampler,        vec2 P) noexcept;
[[spirv::builtin]] vec2 textureQueryLod(usampler2DArray sampler,        vec2 P) noexcept;

[[spirv::builtin]] vec2 textureQueryLod(samplerCubeArray sampler,       vec3 P) noexcept;
[[spirv::builtin]] vec2 textureQueryLod(isamplerCubeArray sampler,      vec3 P) noexcept;
[[spirv::builtin]] vec2 textureQueryLod(usamplerCubeArray sampler,      vec3 P) noexcept;

[[spirv::builtin]] vec2 textureQueryLod(sampler1DShadow sampler,        float P) noexcept;
[[spirv::builtin]] vec2 textureQueryLod(sampler2DShadow sampler,        vec2 P) noexcept;

[[spirv::builtin]] vec2 textureQueryLod(sampler1DArrayShadow sampler,   float P) noexcept;
[[spirv::builtin]] vec2 textureQueryLod(sampler2DArrayShadow sampler,   vec2 P) noexcept;
[[spirv::builtin]] vec2 textureQueryLod(samplerCubeArrayShadow sampler, vec3 P) noexcept;

// textureQueryLevels;
[[spirv::builtin]] int textureQueryLevels(sampler1D sampler) noexcept;
[[spirv::builtin]] int textureQueryLevels(isampler1D sampler) noexcept;
[[spirv::builtin]] int textureQueryLevels(usampler1D sampler) noexcept;
[[spirv::builtin]] int textureQueryLevels(sampler2D sampler) noexcept;
[[spirv::builtin]] int textureQueryLevels(isampler2D sampler) noexcept;
[[spirv::builtin]] int textureQueryLevels(usampler2D sampler) noexcept;
[[spirv::builtin]] int textureQueryLevels(sampler3D sampler) noexcept;
[[spirv::builtin]] int textureQueryLevels(isampler3D sampler) noexcept;
[[spirv::builtin]] int textureQueryLevels(usampler3D sampler) noexcept;
[[spirv::builtin]] int textureQueryLevels(samplerCube sampler) noexcept;
[[spirv::builtin]] int textureQueryLevels(isamplerCube sampler) noexcept;
[[spirv::builtin]] int textureQueryLevels(usamplerCube sampler) noexcept;
[[spirv::builtin]] int textureQueryLevels(sampler1DArray sampler) noexcept;
[[spirv::builtin]] int textureQueryLevels(isampler1DArray sampler) noexcept;
[[spirv::builtin]] int textureQueryLevels(usampler1DArray sampler) noexcept;
[[spirv::builtin]] int textureQueryLevels(sampler2DArray sampler) noexcept;
[[spirv::builtin]] int textureQueryLevels(isampler2DArray sampler) noexcept;
[[spirv::builtin]] int textureQueryLevels(usampler2DArray sampler) noexcept;
[[spirv::builtin]] int textureQueryLevels(samplerCubeArray sampler) noexcept;
[[spirv::builtin]] int textureQueryLevels(isamplerCubeArray sampler) noexcept;
[[spirv::builtin]] int textureQueryLevels(usamplerCubeArray sampler) noexcept;
[[spirv::builtin]] int textureQueryLevels(sampler1DShadow sampler) noexcept;
[[spirv::builtin]] int textureQueryLevels(sampler2DShadow sampler) noexcept;
[[spirv::builtin]] int textureQueryLevels(samplerCubeShadow sampler) noexcept;
[[spirv::builtin]] int textureQueryLevels(sampler1DArrayShadow sampler) noexcept;
[[spirv::builtin]] int textureQueryLevels(sampler2DArrayShadow sampler) noexcept;
[[spirv::builtin]] int textureQueryLevels(samplerCubeArrayShadow sampler) noexcept;

// textureSamples;
[[spirv::builtin]] int textureSamples(sampler2DMS sampler) noexcept;
[[spirv::builtin]] int textureSamples(isampler2DMS sampler) noexcept;
[[spirv::builtin]] int textureSamples(usampler2DMS sampler) noexcept;
[[spirv::builtin]] int textureSamples(sampler2DMSArray sampler) noexcept;
[[spirv::builtin]] int textureSamples(isampler2DMSArray sampler) noexcept;
[[spirv::builtin]] int textureSamples(usampler2DMSArray sampler) noexcept;

////////////////////////////////////////////////////////////////////////////////
// 8.9.2. Texel Lookup Function noexcept;
// texture
[[spirv::builtin]] vec4  texture(sampler1D sampler,         float P) noexcept;
[[spirv::builtin]] ivec4 texture(isampler1D sampler,        float P) noexcept;
[[spirv::builtin]] uvec4 texture(usampler1D sampler,        float P) noexcept;
[[spirv::builtin]] vec4  texture(sampler1D sampler,         float P, float bias) noexcept;
[[spirv::builtin]] ivec4 texture(isampler1D sampler,        float P, float bias) noexcept;
[[spirv::builtin]] uvec4 texture(usampler1D sampler,        float P, float bias) noexcept;

[[spirv::builtin]] vec4  texture(sampler2D sampler,         vec2 P) noexcept;
[[spirv::builtin]] ivec4 texture(isampler2D sampler,        vec2 P) noexcept;
[[spirv::builtin]] uvec4 texture(usampler2D sampler,        vec2 P) noexcept;
[[spirv::builtin]] vec4  texture(sampler2D sampler,         vec2 P, float bias) noexcept;
[[spirv::builtin]] ivec4 texture(isampler2D sampler,        vec2 P, float bias) noexcept;
[[spirv::builtin]] uvec4 texture(usampler2D sampler,        vec2 P, float bias) noexcept;

[[spirv::builtin]] vec4  texture(sampler3D sampler,         vec3 P) noexcept;
[[spirv::builtin]] ivec4 texture(isampler3D sampler,        vec3 P) noexcept;
[[spirv::builtin]] uvec4 texture(usampler3D sampler,        vec3 P) noexcept;
[[spirv::builtin]] vec4  texture(sampler3D sampler,         vec3 P, float bias) noexcept;
[[spirv::builtin]] ivec4 texture(isampler3D sampler,        vec3 P, float bias) noexcept;
[[spirv::builtin]] uvec4 texture(usampler3D sampler,        vec3 P, float bias) noexcept;

[[spirv::builtin]] vec4  texture(samplerCube sampler,       vec3 P) noexcept;
[[spirv::builtin]] ivec4 texture(isamplerCube sampler,      vec3 P) noexcept;
[[spirv::builtin]] uvec4 texture(usamplerCube sampler,      vec3 P) noexcept;
[[spirv::builtin]] vec4  texture(samplerCube sampler,       vec3 P, float bias) noexcept;
[[spirv::builtin]] ivec4 texture(isamplerCube sampler,      vec3 P, float bias) noexcept;
[[spirv::builtin]] uvec4 texture(usamplerCube sampler,      vec3 P, float bias) noexcept;
[[spirv::builtin]] float texture(sampler1DShadow sampler,   vec3 P) noexcept;
[[spirv::builtin]] float texture(sampler1DShadow sampler,   vec3 P, float bias) noexcept;
[[spirv::builtin]] float texture(sampler2DShadow sampler,   vec3 P) noexcept;
[[spirv::builtin]] float texture(sampler2DShadow sampler,   vec3 P, float bias) noexcept;
[[spirv::builtin]] float texture(samplerCubeShadow sampler, vec3 P) noexcept;
[[spirv::builtin]] float texture(samplerCubeShadow sampler, vec3 P, float bias) noexcept;
[[spirv::builtin]] vec4  texture(sampler1DArray sampler,    vec2 P) noexcept;
[[spirv::builtin]] ivec4 texture(isampler1DArray sampler,   vec2 P) noexcept;
[[spirv::builtin]] uvec4 texture(usampler1DArray sampler,   vec2 P) noexcept;
[[spirv::builtin]] vec4  texture(sampler1DArray sampler,    vec2 P, float bias) noexcept;
[[spirv::builtin]] ivec4 texture(isampler1DArray sampler,   vec2 P, float bias) noexcept;
[[spirv::builtin]] uvec4 texture(usampler1DArray sampler,   vec2 P, float bias) noexcept;
[[spirv::builtin]] vec4  texture(sampler2DArray sampler,    vec3 P) noexcept;
[[spirv::builtin]] ivec4 texture(isampler2DArray sampler,   vec3 P) noexcept;
[[spirv::builtin]] uvec4 texture(usampler2DArray sampler,   vec3 P) noexcept;
[[spirv::builtin]] vec4  texture(sampler2DArray sampler,    vec3 P, float bias) noexcept;
[[spirv::builtin]] ivec4 texture(isampler2DArray sampler,   vec3 P, float bias) noexcept;
[[spirv::builtin]] uvec4 texture(usampler2DArray sampler,   vec3 P, float bias) noexcept;
[[spirv::builtin]] vec4  texture(samplerCubeArray sampler,  vec4 P) noexcept;
[[spirv::builtin]] ivec4 texture(isamplerCubeArray sampler, vec4 P) noexcept;
[[spirv::builtin]] uvec4 texture(usamplerCubeArray sampler, vec4 P) noexcept;
[[spirv::builtin]] vec4  texture(samplerCubeArray sampler,  vec4 P, float bias) noexcept;
[[spirv::builtin]] ivec4 texture(isamplerCubeArray sampler, vec4 P, float bias) noexcept;
[[spirv::builtin]] uvec4 texture(usamplerCubeArray sampler, vec4 P, float bias) noexcept;
[[spirv::builtin]] float texture(sampler1DArrayShadow sampler, vec3 P) noexcept;
[[spirv::builtin]] float texture(sampler1DArrayShadow sampler, vec3 P, float bias) noexcept;
[[spirv::builtin]] float texture(sampler2DArrayShadow sampler, vec4 P) noexcept;
[[spirv::builtin]] float texture(sampler2DArrayShadow sampler, vec4 P, float bias) noexcept;
[[spirv::builtin]] vec4  texture(sampler2DRect sampler,  vec2 P) noexcept;
[[spirv::builtin]] ivec4 texture(isampler2DRect sampler, vec2 P) noexcept;
[[spirv::builtin]] uvec4 texture(usampler2DRect sampler, vec2 P) noexcept;
[[spirv::builtin]] float texture(sampler2DRectShadow sampler, vec3 P) noexcept;
[[spirv::builtin]] float texture(samplerCubeArrayShadow sampler, vec4 P, float compare) noexcept;

// textureProj
[[spirv::builtin]] vec4  textureProj(sampler1D sampler,  vec2 P) noexcept;
[[spirv::builtin]] ivec4 textureProj(isampler1D sampler, vec2 P) noexcept;
[[spirv::builtin]] uvec4 textureProj(usampler1D sampler, vec2 P) noexcept;
[[spirv::builtin]] vec4  textureProj(sampler1D sampler,  vec2 P, float bias) noexcept;
[[spirv::builtin]] ivec4 textureProj(isampler1D sampler, vec2 P, float bias) noexcept;
[[spirv::builtin]] uvec4 textureProj(usampler1D sampler, vec2 P, float bias) noexcept;
[[spirv::builtin]] vec4  textureProj(sampler1D sampler,  vec4 P) noexcept;
[[spirv::builtin]] ivec4 textureProj(isampler1D sampler, vec4 P) noexcept;
[[spirv::builtin]] uvec4 textureProj(usampler1D sampler, vec4 P) noexcept;
[[spirv::builtin]] vec4  textureProj(sampler1D sampler,  vec4 P, float bias) noexcept;
[[spirv::builtin]] ivec4 textureProj(isampler1D sampler, vec4 P, float bias) noexcept;
[[spirv::builtin]] uvec4 textureProj(usampler1D sampler, vec4 P, float bias) noexcept;
[[spirv::builtin]] vec4  textureProj(sampler2D sampler,  vec3 P) noexcept;
[[spirv::builtin]] ivec4 textureProj(isampler2D sampler, vec3 P) noexcept;
[[spirv::builtin]] uvec4 textureProj(usampler2D sampler, vec3 P) noexcept;
[[spirv::builtin]] vec4  textureProj(sampler2D sampler,  vec3 P, float bias) noexcept;
[[spirv::builtin]] ivec4 textureProj(isampler2D sampler, vec3 P, float bias) noexcept;
[[spirv::builtin]] uvec4 textureProj(usampler2D sampler, vec3 P, float bias) noexcept;
[[spirv::builtin]] vec4  textureProj(sampler2D sampler,  vec4 P) noexcept;
[[spirv::builtin]] ivec4 textureProj(isampler2D sampler, vec4 P) noexcept;
[[spirv::builtin]] uvec4 textureProj(usampler2D sampler, vec4 P) noexcept;
[[spirv::builtin]] vec4  textureProj(sampler2D sampler,  vec4 P, float bias) noexcept;
[[spirv::builtin]] ivec4 textureProj(isampler2D sampler, vec4 P, float bias) noexcept;
[[spirv::builtin]] uvec4 textureProj(usampler2D sampler, vec4 P, float bias) noexcept;
[[spirv::builtin]] vec4  textureProj(sampler3D sampler,  vec4 P) noexcept;
[[spirv::builtin]] ivec4 textureProj(isampler3D sampler, vec4 P) noexcept;
[[spirv::builtin]] uvec4 textureProj(usampler3D sampler, vec4 P) noexcept;
[[spirv::builtin]] vec4  textureProj(sampler3D sampler,  vec4 P, float bias) noexcept;
[[spirv::builtin]] ivec4 textureProj(isampler3D sampler, vec4 P, float bias) noexcept;
[[spirv::builtin]] uvec4 textureProj(usampler3D sampler, vec4 P, float bias) noexcept;
[[spirv::builtin]] float textureProj(sampler1DShadow sampler, vec4 P) noexcept;
[[spirv::builtin]] float textureProj(sampler1DShadow sampler, vec4 P, float bias) noexcept;
[[spirv::builtin]] float textureProj(sampler2DShadow sampler, vec4 P) noexcept;
[[spirv::builtin]] float textureProj(sampler2DShadow sampler, vec4 P, float bias) noexcept;
[[spirv::builtin]] vec4  textureProj(sampler2DRect sampler,  vec3 P) noexcept;
[[spirv::builtin]] ivec4 textureProj(isampler2DRect sampler, vec3 P) noexcept;
[[spirv::builtin]] uvec4 textureProj(usampler2DRect sampler, vec3 P) noexcept;
[[spirv::builtin]] vec4  textureProj(sampler2DRect sampler,  vec4 P) noexcept;
[[spirv::builtin]] ivec4 textureProj(isampler2DRect sampler, vec4 P) noexcept;
[[spirv::builtin]] uvec4 textureProj(usampler2DRect sampler, vec4 P) noexcept;
[[spirv::builtin]] float textureProj(sampler2DRectShadow sampler, vec4 P) noexcept;

// textureLod
[[spirv::builtin]] vec4  textureLod(sampler1D sampler,            float P, float lod) noexcept;
[[spirv::builtin]] ivec4 textureLod(isampler1D sampler,           float P, float lod) noexcept;
[[spirv::builtin]] uvec4 textureLod(usampler1D sampler,           float P, float lod) noexcept;
[[spirv::builtin]] vec4  textureLod(sampler2D sampler,            vec2 P,  float lod) noexcept;
[[spirv::builtin]] ivec4 textureLod(isampler2D sampler,           vec2 P,  float lod) noexcept;
[[spirv::builtin]] uvec4 textureLod(usampler2D sampler,           vec2 P,  float lod) noexcept;
[[spirv::builtin]] vec4  textureLod(sampler3D sampler,            vec3 P,  float lod) noexcept;
[[spirv::builtin]] ivec4 textureLod(isampler3D sampler,           vec3 P,  float lod) noexcept;
[[spirv::builtin]] uvec4 textureLod(usampler3D sampler,           vec3 P,  float lod) noexcept;
[[spirv::builtin]] vec4  textureLod(samplerCube sampler,          vec3 P,  float lod) noexcept;
[[spirv::builtin]] ivec4 textureLod(isamplerCube sampler,         vec3 P,  float lod) noexcept;
[[spirv::builtin]] uvec4 textureLod(usamplerCube sampler,         vec3 P,  float lod) noexcept;
[[spirv::builtin]] float textureLod(sampler1DShadow sampler,      vec3 P,  float lod) noexcept;
[[spirv::builtin]] float textureLod(sampler2DShadow sampler,      vec3 P,  float lod) noexcept;
[[spirv::builtin]] vec4  textureLod(sampler1DArray sampler,       vec2 P,  float lod) noexcept;
[[spirv::builtin]] ivec4 textureLod(isampler1DArray sampler,      vec2 P,  float lod) noexcept;
[[spirv::builtin]] uvec4 textureLod(usampler1DArray sampler,      vec2 P,  float lod) noexcept;
[[spirv::builtin]] float textureLod(sampler1DArrayShadow sampler, vec3 P,  float lod) noexcept;
[[spirv::builtin]] vec4  textureLod(sampler2DArray sampler,       vec3 P,  float lod) noexcept;
[[spirv::builtin]] ivec4 textureLod(isampler2DArray sampler,      vec3 P,  float lod) noexcept;
[[spirv::builtin]] uvec4 textureLod(usampler2DArray sampler,      vec3 P,  float lod) noexcept;
[[spirv::builtin]] vec4  textureLod(samplerCubeArray sampler,     vec4 P,  float lod) noexcept;
[[spirv::builtin]] ivec4 textureLod(isamplerCubeArray sampler,    vec4 P,  float lod) noexcept;
[[spirv::builtin]] uvec4 textureLod(usamplerCubeArray sampler,    vec4 P,  float lod) noexcept;

// textureOffset
[[spirv::builtin]] vec4  textureOffset(sampler1D sampler,  float P, int offset) noexcept;
[[spirv::builtin]] ivec4 textureOffset(isampler1D sampler, float P, int offset) noexcept;
[[spirv::builtin]] uvec4 textureOffset(usampler1D sampler, float P, int offset) noexcept;
[[spirv::builtin]] vec4  textureOffset(sampler1D sampler,  float P, int offset, float bias) noexcept;
[[spirv::builtin]] ivec4 textureOffset(isampler1D sampler, float P, int offset, float bias) noexcept;
[[spirv::builtin]] uvec4 textureOffset(usampler1D sampler, float P, int offset, float bias) noexcept;
[[spirv::builtin]] vec4  textureOffset(sampler2D sampler,  vec2 P, ivec2 offset) noexcept;
[[spirv::builtin]] ivec4 textureOffset(isampler2D sampler, vec2 P, ivec2 offset) noexcept;
[[spirv::builtin]] uvec4 textureOffset(usampler2D sampler, vec2 P, ivec2 offset) noexcept;
[[spirv::builtin]] vec4  textureOffset(sampler2D sampler,  vec2 P, ivec2 offset, float bias) noexcept;
[[spirv::builtin]] ivec4 textureOffset(isampler2D sampler, vec2 P, ivec2 offset, float bias) noexcept;
[[spirv::builtin]] uvec4 textureOffset(usampler2D sampler, vec2 P, ivec2 offset, float bias) noexcept;
[[spirv::builtin]] vec4  textureOffset(sampler3D sampler,  vec3 P, ivec3 offset) noexcept;
[[spirv::builtin]] ivec4 textureOffset(isampler3D sampler, vec3 P, ivec3 offset) noexcept;
[[spirv::builtin]] uvec4 textureOffset(usampler3D sampler, vec3 P, ivec3 offset) noexcept;
[[spirv::builtin]] vec4  textureOffset(sampler3D sampler,  vec3 P, ivec3 offset, float bias) noexcept;
[[spirv::builtin]] ivec4 textureOffset(isampler3D sampler, vec3 P, ivec3 offset, float bias) noexcept;
[[spirv::builtin]] uvec4 textureOffset(usampler3D sampler, vec3 P, ivec3 offset, float bias) noexcept;
[[spirv::builtin]] float textureOffset(sampler1DShadow sampler, vec3 P, int offset) noexcept;
[[spirv::builtin]] float textureOffset(sampler1DShadow sampler, vec3 P, int offset, float bias) noexcept;
[[spirv::builtin]] float textureOffset(sampler2DShadow sampler, vec3 P, ivec2 offset) noexcept;
[[spirv::builtin]] float textureOffset(sampler2DShadow sampler, vec3 P, ivec2 offset, float bias) noexcept;
[[spirv::builtin]] vec4 textureOffset(sampler2DRect sampler,   vec2 P, ivec2 offset) noexcept;
[[spirv::builtin]] ivec4 textureOffset(isampler2DRect sampler, vec2 P, ivec2 offset) noexcept;
[[spirv::builtin]] uvec4 textureOffset(usampler2DRect sampler, vec2 P, ivec2 offset) noexcept;
[[spirv::builtin]] float textureOffset(sampler2DRectShadow sampler, vec3 P, ivec2 offset) noexcept;
[[spirv::builtin]] vec4  textureOffset(sampler1DArray sampler,  vec2 P, int offset) noexcept;
[[spirv::builtin]] ivec4 textureOffset(isampler1DArray sampler, vec2 P, int offset) noexcept;
[[spirv::builtin]] uvec4 textureOffset(usampler1DArray sampler, vec2 P, int offset) noexcept;
[[spirv::builtin]] vec4  textureOffset(sampler1DArray sampler,  vec2 P, int offset, float bias) noexcept;
[[spirv::builtin]] ivec4 textureOffset(isampler1DArray sampler, vec2 P, int offset, float bias) noexcept;
[[spirv::builtin]] uvec4 textureOffset(usampler1DArray sampler, vec2 P, int offset, float bias) noexcept;
[[spirv::builtin]] vec4  textureOffset(sampler2DArray sampler,  vec3 P, ivec2 offset) noexcept;
[[spirv::builtin]] ivec4 textureOffset(isampler2DArray sampler, vec3 P, ivec2 offset) noexcept;
[[spirv::builtin]] uvec4 textureOffset(usampler2DArray sampler, vec3 P, ivec2 offset) noexcept;
[[spirv::builtin]] vec4  textureOffset(sampler2DArray sampler,  vec3 P, ivec2 offset, float bias) noexcept;
[[spirv::builtin]] ivec4 textureOffset(isampler2DArray sampler, vec3 P, ivec2 offset, float bias) noexcept;
[[spirv::builtin]] uvec4 textureOffset(usampler2DArray sampler, vec3 P, ivec2 offset, float bias) noexcept;
[[spirv::builtin]] float textureOffset(sampler1DArrayShadow sampler, vec3 P, int offset) noexcept;
[[spirv::builtin]] float textureOffset(sampler1DArrayShadow sampler, vec3 P, int offset, float bias) noexcept;
[[spirv::builtin]] float textureOffset(sampler2DArrayShadow sampler, vec4 P, ivec2 offset) noexcept;

// texelFetch
[[spirv::builtin]] vec4  texelFetch(sampler1D sampler,  int P, int lod) noexcept;
[[spirv::builtin]] ivec4 texelFetch(isampler1D sampler, int P, int lod) noexcept;
[[spirv::builtin]] uvec4 texelFetch(usampler1D sampler, int P, int lod) noexcept;
[[spirv::builtin]] vec4  texelFetch(sampler2D sampler,  ivec2 P, int lod) noexcept;
[[spirv::builtin]] ivec4 texelFetch(isampler2D sampler, ivec2 P, int lod) noexcept;
[[spirv::builtin]] uvec4 texelFetch(usampler2D sampler, ivec2 P, int lod) noexcept;
[[spirv::builtin]] vec4  texelFetch(sampler3D sampler,  ivec3 P, int lod) noexcept;
[[spirv::builtin]] ivec4 texelFetch(isampler3D sampler, ivec3 P, int lod) noexcept;
[[spirv::builtin]] uvec4 texelFetch(usampler3D sampler, ivec3 P, int lod) noexcept;
[[spirv::builtin]] vec4  texelFetch(sampler2DRect sampler,  ivec2 P) noexcept;
[[spirv::builtin]] ivec4 texelFetch(isampler2DRect sampler, ivec2 P) noexcept;
[[spirv::builtin]] uvec4 texelFetch(usampler2DRect sampler, ivec2 P) noexcept;
[[spirv::builtin]] vec4  texelFetch(sampler1DArray sampler,  ivec2 P, int lod) noexcept;
[[spirv::builtin]] ivec4 texelFetch(isampler1DArray sampler, ivec2 P, int lod) noexcept;
[[spirv::builtin]] uvec4 texelFetch(usampler1DArray sampler, ivec2 P, int lod) noexcept;
[[spirv::builtin]] vec4  texelFetch(sampler2DArray sampler,  ivec3 P, int lod) noexcept;
[[spirv::builtin]] ivec4 texelFetch(isampler2DArray sampler, ivec3 P, int lod) noexcept;
[[spirv::builtin]] uvec4 texelFetch(usampler2DArray sampler, ivec3 P, int lod) noexcept;
[[spirv::builtin]] vec4  texelFetch(samplerBuffer sampler,  int P) noexcept;
[[spirv::builtin]] ivec4 texelFetch(isamplerBuffer sampler, int P) noexcept;
[[spirv::builtin]] uvec4 texelFetch(usamplerBuffer sampler, int P) noexcept;
[[spirv::builtin]] vec4  texelFetch(sampler2DMS sampler,  ivec2 P, int sample) noexcept;
[[spirv::builtin]] ivec4 texelFetch(isampler2DMS sampler, ivec2 P, int sample) noexcept;
[[spirv::builtin]] uvec4 texelFetch(usampler2DMS sampler, ivec2 P, int sample) noexcept;
[[spirv::builtin]] vec4  texelFetch(sampler2DMSArray sampler,  ivec3 P, int sample) noexcept;
[[spirv::builtin]] ivec4 texelFetch(isampler2DMSArray sampler, ivec3 P, int sample) noexcept;
[[spirv::builtin]] uvec4 texelFetch(usampler2DMSArray sampler, ivec3 P, int sample) noexcept;

// texelFetchOffset
[[spirv::builtin]] vec4  texelFetchOffset(sampler1D sampler,  int P, int lod, int offset) noexcept;
[[spirv::builtin]] ivec4 texelFetchOffset(isampler1D sampler, int P, int lod, int offset) noexcept;
[[spirv::builtin]] uvec4 texelFetchOffset(usampler1D sampler, int P, int lod, int offset) noexcept;
[[spirv::builtin]] vec4  texelFetchOffset(sampler2D sampler,  ivec2 P, int lod, ivec2 offset) noexcept;
[[spirv::builtin]] ivec4 texelFetchOffset(isampler2D sampler, ivec2 P, int lod, ivec2 offset) noexcept;
[[spirv::builtin]] uvec4 texelFetchOffset(usampler2D sampler, ivec2 P, int lod, ivec2 offset) noexcept;
[[spirv::builtin]] vec4  texelFetchOffset(sampler3D sampler,  ivec3 P, int lod, ivec3 offset) noexcept;
[[spirv::builtin]] ivec4 texelFetchOffset(isampler3D sampler, ivec3 P, int lod, ivec3 offset) noexcept;
[[spirv::builtin]] uvec4 texelFetchOffset(usampler3D sampler, ivec3 P, int lod, ivec3 offset) noexcept;
[[spirv::builtin]] vec4  texelFetchOffset(sampler2DRect sampler,  ivec2 P, ivec2 offset) noexcept;
[[spirv::builtin]] ivec4 texelFetchOffset(isampler2DRect sampler, ivec2 P, ivec2 offset) noexcept;
[[spirv::builtin]] uvec4 texelFetchOffset(usampler2DRect sampler, ivec2 P, ivec2 offset) noexcept;
[[spirv::builtin]] vec4  texelFetchOffset(sampler1DArray sampler,  ivec2 P, int lod, int offset) noexcept;
[[spirv::builtin]] ivec4 texelFetchOffset(isampler1DArray sampler, ivec2 P, int lod, int offset) noexcept;
[[spirv::builtin]] uvec4 texelFetchOffset(usampler1DArray sampler, ivec2 P, int lod, int offset) noexcept;
[[spirv::builtin]] vec4  texelFetchOffset(sampler2DArray sampler,  ivec3 P, int lod, ivec2 offset) noexcept;
[[spirv::builtin]] ivec4 texelFetchOffset(isampler2DArray sampler, ivec3 P, int lod, ivec2 offset) noexcept;
[[spirv::builtin]] uvec4 texelFetchOffset(usampler2DArray sampler, ivec3 P, int lod, ivec2 offset) noexcept;

// textureProjOffset
[[spirv::builtin]] vec4  textureProjOffset(sampler1D sampler,  vec2 P, int offset) noexcept;
[[spirv::builtin]] ivec4 textureProjOffset(isampler1D sampler, vec2 P, int offset) noexcept;
[[spirv::builtin]] uvec4 textureProjOffset(usampler1D sampler, vec2 P, int offset) noexcept;
[[spirv::builtin]] vec4  textureProjOffset(sampler1D sampler,  vec2 P, int offset, float bias) noexcept;
[[spirv::builtin]] ivec4 textureProjOffset(isampler1D sampler, vec2 P, int offset, float bias) noexcept;
[[spirv::builtin]] uvec4 textureProjOffset(usampler1D sampler, vec2 P, int offset, float bias) noexcept;
[[spirv::builtin]] vec4  textureProjOffset(sampler1D sampler,  vec4 P, int offset) noexcept;
[[spirv::builtin]] ivec4 textureProjOffset(isampler1D sampler, vec4 P, int offset) noexcept;
[[spirv::builtin]] uvec4 textureProjOffset(usampler1D sampler, vec4 P, int offset) noexcept;
[[spirv::builtin]] vec4  textureProjOffset(sampler1D sampler,  vec4 P, int offset, float bias) noexcept;
[[spirv::builtin]] ivec4 textureProjOffset(isampler1D sampler, vec4 P, int offset, float bias) noexcept;
[[spirv::builtin]] uvec4 textureProjOffset(usampler1D sampler, vec4 P, int offset, float bias) noexcept;
[[spirv::builtin]] vec4  textureProjOffset(sampler2D sampler,  vec3 P, ivec2 offset) noexcept;
[[spirv::builtin]] ivec4 textureProjOffset(isampler2D sampler, vec3 P, ivec2 offset) noexcept;
[[spirv::builtin]] uvec4 textureProjOffset(usampler2D sampler, vec3 P, ivec2 offset) noexcept;
[[spirv::builtin]] vec4  textureProjOffset(sampler2D sampler,  vec3 P, ivec2 offset, float bias) noexcept;
[[spirv::builtin]] ivec4 textureProjOffset(isampler2D sampler, vec3 P, ivec2 offset, float bias) noexcept;
[[spirv::builtin]] uvec4 textureProjOffset(usampler2D sampler, vec3 P, ivec2 offset, float bias) noexcept;
[[spirv::builtin]] vec4  textureProjOffset(sampler2D sampler,  vec4 P, ivec2 offset) noexcept;
[[spirv::builtin]] ivec4 textureProjOffset(isampler2D sampler, vec4 P, ivec2 offset) noexcept;
[[spirv::builtin]] uvec4 textureProjOffset(usampler2D sampler, vec4 P, ivec2 offset) noexcept;
[[spirv::builtin]] vec4  textureProjOffset(sampler2D sampler,  vec4 P, ivec2 offset, float bias) noexcept;
[[spirv::builtin]] ivec4 textureProjOffset(isampler2D sampler, vec4 P, ivec2 offset, float bias) noexcept;
[[spirv::builtin]] uvec4 textureProjOffset(usampler2D sampler, vec4 P, ivec2 offset, float bias) noexcept;
[[spirv::builtin]] vec4  textureProjOffset(sampler3D sampler,  vec4 P, ivec3 offset) noexcept;
[[spirv::builtin]] ivec4 textureProjOffset(isampler3D sampler, vec4 P, ivec3 offset) noexcept;
[[spirv::builtin]] uvec4 textureProjOffset(usampler3D sampler, vec4 P, ivec3 offset) noexcept;
[[spirv::builtin]] vec4  textureProjOffset(sampler3D sampler,  vec4 P, ivec3 offset, float bias) noexcept;
[[spirv::builtin]] ivec4 textureProjOffset(isampler3D sampler, vec4 P, ivec3 offset, float bias) noexcept;
[[spirv::builtin]] uvec4 textureProjOffset(usampler3D sampler, vec4 P, ivec3 offset, float bias) noexcept;
[[spirv::builtin]] float textureProjOffset(sampler1DShadow sampler, vec4 P, int offset) noexcept;
[[spirv::builtin]] float textureProjOffset(sampler1DShadow sampler, vec4 P, int offset, float bias) noexcept;
[[spirv::builtin]] float textureProjOffset(sampler2DShadow sampler, vec4 P, ivec2 offset) noexcept;
[[spirv::builtin]] float textureProjOffset(sampler2DShadow sampler, vec4 P, ivec2 offset, float bias) noexcept;
[[spirv::builtin]] vec4  textureProjOffset(sampler2DRect sampler,  vec3 P, ivec2 offset) noexcept;
[[spirv::builtin]] ivec4 textureProjOffset(isampler2DRect sampler, vec3 P, ivec2 offset) noexcept;
[[spirv::builtin]] uvec4 textureProjOffset(usampler2DRect sampler, vec3 P, ivec2 offset) noexcept;
[[spirv::builtin]] vec4  textureProjOffset(sampler2DRect sampler,  vec4 P, ivec2 offset) noexcept;
[[spirv::builtin]] ivec4 textureProjOffset(isampler2DRect sampler, vec4 P, ivec2 offset) noexcept;
[[spirv::builtin]] uvec4 textureProjOffset(usampler2DRect sampler, vec4 P, ivec2 offset) noexcept;
[[spirv::builtin]] float textureProjOffset(sampler2DRectShadow sampler, vec4 P, ivec2 offset) noexcept;

// textureLodOffset
[[spirv::builtin]] vec4  textureLodOffset(sampler1D sampler,            float P, float lod, int offset) noexcept;
[[spirv::builtin]] ivec4 textureLodOffset(isampler1D sampler,           float P, float lod, int offset) noexcept;
[[spirv::builtin]] uvec4 textureLodOffset(usampler1D sampler,           float P, float lod, int offset) noexcept;
[[spirv::builtin]] vec4  textureLodOffset(sampler2D sampler,            vec2 P,  float lod, ivec2 offset) noexcept;
[[spirv::builtin]] ivec4 textureLodOffset(isampler2D sampler,           vec2 P,  float lod, ivec2 offset) noexcept;
[[spirv::builtin]] uvec4 textureLodOffset(usampler2D sampler,           vec2 P,  float lod, ivec2 offset) noexcept;
[[spirv::builtin]] vec4  textureLodOffset(sampler3D sampler,            vec3 P,  float lod, ivec3 offset) noexcept;
[[spirv::builtin]] ivec4 textureLodOffset(isampler3D sampler,           vec3 P,  float lod, ivec3 offset) noexcept;
[[spirv::builtin]] uvec4 textureLodOffset(usampler3D sampler,           vec3 P,  float lod, ivec3 offset) noexcept;
[[spirv::builtin]] float textureLodOffset(sampler1DShadow sampler,      vec3 P,  float lod, int offset) noexcept;
[[spirv::builtin]] float textureLodOffset(sampler2DShadow sampler,      vec3 P,  float lod, ivec2 offset) noexcept;
[[spirv::builtin]] vec4  textureLodOffset(sampler1DArray sampler,       vec2 P,  float lod, int offset) noexcept;
[[spirv::builtin]] ivec4 textureLodOffset(isampler1DArray sampler,      vec2 P,  float lod, int offset) noexcept;
[[spirv::builtin]] uvec4 textureLodOffset(usampler1DArray sampler,      vec2 P,  float lod, int offset) noexcept;
[[spirv::builtin]] float textureLodOffset(sampler1DArrayShadow sampler, vec3 P,  float lod, int offset) noexcept;
[[spirv::builtin]] vec4  textureLodOffset(sampler2DArray sampler,       vec3 P,  float lod, ivec2 offset) noexcept;
[[spirv::builtin]] ivec4 textureLodOffset(isampler2DArray sampler,      vec3 P,  float lod, ivec2 offset) noexcept;
[[spirv::builtin]] uvec4 textureLodOffset(usampler2DArray sampler,      vec3 P,  float lod, ivec2 offset) noexcept;

// textureProjLod
[[spirv::builtin]] vec4  textureProjLod(sampler1D sampler,       vec2 P, float lod) noexcept;
[[spirv::builtin]] ivec4 textureProjLod(isampler1D sampler,      vec2 P, float lod) noexcept;
[[spirv::builtin]] uvec4 textureProjLod(usampler1D sampler,      vec2 P, float lod) noexcept;
[[spirv::builtin]] vec4  textureProjLod(sampler1D sampler,       vec4 P, float lod) noexcept;
[[spirv::builtin]] ivec4 textureProjLod(isampler1D sampler,      vec4 P, float lod) noexcept;
[[spirv::builtin]] uvec4 textureProjLod(usampler1D sampler,      vec4 P, float lod) noexcept;
[[spirv::builtin]] vec4  textureProjLod(sampler2D sampler,       vec3 P, float lod) noexcept;
[[spirv::builtin]] ivec4 textureProjLod(isampler2D sampler,      vec3 P, float lod) noexcept;
[[spirv::builtin]] uvec4 textureProjLod(usampler2D sampler,      vec3 P, float lod) noexcept;
[[spirv::builtin]] vec4  textureProjLod(sampler2D sampler,       vec4 P, float lod) noexcept;
[[spirv::builtin]] ivec4 textureProjLod(isampler2D sampler,      vec4 P, float lod) noexcept;
[[spirv::builtin]] uvec4 textureProjLod(usampler2D sampler,      vec4 P, float lod) noexcept;
[[spirv::builtin]] vec4  textureProjLod(sampler3D sampler,       vec4 P, float lod) noexcept;
[[spirv::builtin]] ivec4 textureProjLod(isampler3D sampler,      vec4 P, float lod) noexcept;
[[spirv::builtin]] uvec4 textureProjLod(usampler3D sampler,      vec4 P, float lod) noexcept;
[[spirv::builtin]] float textureProjLod(sampler1DShadow sampler, vec4 P, float lod) noexcept;
[[spirv::builtin]] float textureProjLod(sampler2DShadow sampler, vec4 P, float lod) noexcept;

// textureProjLodOffset
[[spirv::builtin]] vec4  textureProjLodOffset(sampler1D sampler,       vec2 P, float lod, int offset) noexcept;
[[spirv::builtin]] ivec4 textureProjLodOffset(isampler1D sampler,      vec2 P, float lod, int offset) noexcept;
[[spirv::builtin]] uvec4 textureProjLodOffset(usampler1D sampler,      vec2 P, float lod, int offset) noexcept;
[[spirv::builtin]] vec4  textureProjLodOffset(sampler1D sampler,       vec4 P, float lod, int offset) noexcept;
[[spirv::builtin]] ivec4 textureProjLodOffset(isampler1D sampler,      vec4 P, float lod, int offset) noexcept;
[[spirv::builtin]] uvec4 textureProjLodOffset(usampler1D sampler,      vec4 P, float lod, int offset) noexcept;
[[spirv::builtin]] vec4  textureProjLodOffset(sampler2D sampler,       vec3 P, float lod, ivec2 offset) noexcept;
[[spirv::builtin]] ivec4 textureProjLodOffset(isampler2D sampler,      vec3 P, float lod, ivec2 offset) noexcept;
[[spirv::builtin]] uvec4 textureProjLodOffset(usampler2D sampler,      vec3 P, float lod, ivec2 offset) noexcept;
[[spirv::builtin]] vec4  textureProjLodOffset(sampler2D sampler,       vec4 P, float lod, ivec2 offset) noexcept;
[[spirv::builtin]] ivec4 textureProjLodOffset(isampler2D sampler,      vec4 P, float lod, ivec2 offset) noexcept;
[[spirv::builtin]] uvec4 textureProjLodOffset(usampler2D sampler,      vec4 P, float lod, ivec2 offset) noexcept;
[[spirv::builtin]] vec4  textureProjLodOffset(sampler3D sampler,       vec4 P, float lod, ivec3 offset) noexcept;
[[spirv::builtin]] ivec4 textureProjLodOffset(isampler3D sampler,      vec4 P, float lod, ivec3 offset) noexcept;
[[spirv::builtin]] uvec4 textureProjLodOffset(usampler3D sampler,      vec4 P, float lod, ivec3 offset) noexcept;
[[spirv::builtin]] float textureProjLodOffset(sampler1DShadow sampler, vec4 P, float lod, int offset) noexcept;
[[spirv::builtin]] float textureProjLodOffset(sampler2DShadow sampler, vec4 P, float lod, ivec2 offset) noexcept;

// textureGrad
[[spirv::builtin]] vec4  textureGrad(sampler1D sampler,      float P, float dPdx, float dPdy) noexcept;
[[spirv::builtin]] ivec4 textureGrad(isampler1D sampler,     float P, float dPdx, float dPdy) noexcept;
[[spirv::builtin]] uvec4 textureGrad(usampler1D sampler,     float P, float dPdx, float dPdy) noexcept;
[[spirv::builtin]] vec4  textureGrad(sampler2D sampler,      vec2 P,  vec2 dPdx,  vec2 dPdy) noexcept;
[[spirv::builtin]] ivec4 textureGrad(isampler2D sampler,     vec2 P,  vec2 dPdx,  vec2 dPdy) noexcept;
[[spirv::builtin]] uvec4 textureGrad(usampler2D sampler,     vec2 P,  vec2 dPdx,  vec2 dPdy) noexcept;
[[spirv::builtin]] vec4  textureGrad(sampler3D sampler,      vec3 P,  vec3 dPdx,  vec3 dPdy) noexcept;
[[spirv::builtin]] ivec4 textureGrad(isampler3D sampler,     vec3 P,  vec3 dPdx,  vec3 dPdy) noexcept;
[[spirv::builtin]] uvec4 textureGrad(usampler3D sampler,     vec3 P,  vec3 dPdx,  vec3 dPdy) noexcept;
[[spirv::builtin]] vec4  textureGrad(samplerCube sampler,    vec3 P,  vec3 dPdx,  vec3 dPdy) noexcept;
[[spirv::builtin]] ivec4 textureGrad(isamplerCube sampler,   vec3 P,  vec3 dPdx,  vec3 dPdy) noexcept;
[[spirv::builtin]] uvec4 textureGrad(usamplerCube sampler,   vec3 P,  vec3 dPdx,  vec3 dPdy) noexcept;
[[spirv::builtin]] vec4  textureGrad(sampler2DRect sampler,  vec2 P,  vec2 dPdx,  vec2 dPdy) noexcept;
[[spirv::builtin]] ivec4 textureGrad(isampler2DRect sampler, vec2 P,  vec2 dPdx,  vec2 dPdy) noexcept;
[[spirv::builtin]] uvec4 textureGrad(usampler2DRect sampler, vec2 P,  vec2 dPdx,  vec2 dPdy) noexcept;
[[spirv::builtin]] float textureGrad(sampler2DRectShadow sampler, vec3 P, vec2 dPdx, vec2 dPdy) noexcept;
[[spirv::builtin]] float textureGrad(sampler1DShadow sampler, vec3 P, float dPdx, float dPdy) noexcept;
[[spirv::builtin]] vec4  textureGrad(sampler1DArray sampler,  vec2 P, float dPdx, float dPdy) noexcept;
[[spirv::builtin]] ivec4 textureGrad(isampler1DArray sampler, vec2 P, float dPdx, float dPdy) noexcept;
[[spirv::builtin]] uvec4 textureGrad(usampler1DArray sampler, vec2 P, float dPdx, float dPdy) noexcept;
[[spirv::builtin]] vec4  textureGrad(sampler2DArray sampler,  vec3 P, vec2 dPdx, vec2 dPdy) noexcept;
[[spirv::builtin]] ivec4 textureGrad(isampler2DArray sampler, vec3 P, vec2 dPdx, vec2 dPdy) noexcept;
[[spirv::builtin]] uvec4 textureGrad(usampler2DArray sampler, vec3 P, vec2 dPdx, vec2 dPdy) noexcept;
[[spirv::builtin]] float textureGrad(sampler1DArrayShadow sampler, vec3 P, float dPdx, float dPdy) noexcept;
[[spirv::builtin]] float textureGrad(sampler2DShadow sampler, vec3 P, vec2 dPdx, vec2 dPdy) noexcept;
[[spirv::builtin]] float textureGrad(samplerCubeShadow sampler, vec4 P, vec3 dPdx, vec3 dPdy) noexcept;
[[spirv::builtin]] float textureGrad(sampler2DArrayShadow sampler, vec4 P, vec2 dPdx, vec2 dPdy) noexcept;
[[spirv::builtin]] vec4  textureGrad(samplerCubeArray sampler,  vec4 P, vec3 dPdx, vec3 dPdy) noexcept;
[[spirv::builtin]] ivec4 textureGrad(isamplerCubeArray sampler, vec4 P, vec3 dPdx, vec3 dPdy) noexcept;
[[spirv::builtin]] uvec4 textureGrad(usamplerCubeArray sampler, vec4 P, vec3 dPdx, vec3 dPdy) noexcept;

// textureGradOffset
[[spirv::builtin]] vec4  textureGradOffset(sampler1D sampler,            float P, float dPdx, float dPdy, int offset) noexcept;
[[spirv::builtin]] ivec4 textureGradOffset(isampler1D sampler,           float P, float dPdx, float dPdy, int offset) noexcept;
[[spirv::builtin]] uvec4 textureGradOffset(usampler1D sampler,           float P, float dPdx, float dPdy, int offset) noexcept;

[[spirv::builtin]] vec4  textureGradOffset(sampler2D sampler,            vec2 P,  vec2 dPdx,  vec2 dPdy,  ivec2 offset) noexcept;
[[spirv::builtin]] ivec4 textureGradOffset(isampler2D sampler,           vec2 P,  vec2 dPdx,  vec2 dPdy,  ivec2 offset) noexcept;
[[spirv::builtin]] uvec4 textureGradOffset(usampler2D sampler,           vec2 P,  vec2 dPdx,  vec2 dPdy,  ivec2 offset) noexcept;

[[spirv::builtin]] vec4  textureGradOffset(sampler3D sampler,            vec3 P,  vec3 dPdx,  vec3 dPdy,  ivec3 offset) noexcept;
[[spirv::builtin]] ivec4 textureGradOffset(isampler3D sampler,           vec3 P,  vec3 dPdx,  vec3 dPdy,  ivec3 offset) noexcept;
[[spirv::builtin]] uvec4 textureGradOffset(usampler3D sampler,           vec3 P,  vec3 dPdx,  vec3 dPdy,  ivec3 offset) noexcept;

[[spirv::builtin]] vec4  textureGradOffset(sampler2DRect sampler,        vec2 P,  vec2 dPdx,  vec2 dPdy,  ivec2 offset) noexcept;
[[spirv::builtin]] ivec4 textureGradOffset(isampler2DRect sampler,       vec2 P,  vec2 dPdx,  vec2 dPdy,  ivec2 offset) noexcept;
[[spirv::builtin]] uvec4 textureGradOffset(usampler2DRect sampler,       vec2 P,  vec2 dPdx,  vec2 dPdy,  ivec2 offset) noexcept;
[[spirv::builtin]] float textureGradOffset(sampler2DRectShadow sampler,  vec3 P, vec2 dPdx,   vec2 dPdy,  ivec2 offset) noexcept;
[[spirv::builtin]] float textureGradOffset(sampler1DShadow sampler,      vec3 P, float dPdx,  float dPdy, int offset) noexcept;
[[spirv::builtin]] float textureGradOffset(sampler2DShadow sampler,      vec3 P, vec2 dPdx,   vec2 dPdy,  ivec2 offset) noexcept;

[[spirv::builtin]] vec4  textureGradOffset(sampler1DArray sampler,       vec2 P, float dPdx,  float dPdy, int offset) noexcept;
[[spirv::builtin]] ivec4 textureGradOffset(isampler1DArray sampler,      vec2 P, float dPdx,  float dPdy, int offset) noexcept;
[[spirv::builtin]] uvec4 textureGradOffset(usampler1DArray sampler,      vec2 P, float dPdx,  float dPdy, int offset) noexcept;

[[spirv::builtin]] vec4  textureGradOffset(sampler2DArray sampler,       vec3 P, vec2 dPdx,   vec2 dPdy,  ivec2 offset) noexcept;
[[spirv::builtin]] ivec4 textureGradOffset(isampler2DArray sampler,      vec3 P, vec2 dPdx,   vec2 dPdy,  ivec2 offset) noexcept;
[[spirv::builtin]] uvec4 textureGradOffset(usampler2DArray sampler,      vec3 P, vec2 dPdx,   vec2 dPdy,  ivec2 offset) noexcept;
[[spirv::builtin]] float textureGradOffset(sampler1DArrayShadow sampler, vec3 P, float dPdx,  float dPdy, int offset) noexcept;
[[spirv::builtin]] float textureGradOffset(sampler2DArrayShadow sampler, vec4 P, vec2 dPdx,   vec2 dPdy,  ivec2 offset) noexcept;

// textureProjGrad
[[spirv::builtin]] vec4  textureProjGrad(sampler1D sampler,           vec2 P, float dPdx, float dPdy) noexcept;
[[spirv::builtin]] ivec4 textureProjGrad(isampler1D sampler,          vec2 P, float dPdx, float dPdy) noexcept;
[[spirv::builtin]] uvec4 textureProjGrad(usampler1D sampler,          vec2 P, float dPdx, float dPdy) noexcept;
[[spirv::builtin]] vec4  textureProjGrad(sampler1D sampler,           vec4 P, float dPdx, float dPdy) noexcept;
[[spirv::builtin]] ivec4 textureProjGrad(isampler1D sampler,          vec4 P, float dPdx, float dPdy) noexcept;
[[spirv::builtin]] uvec4 textureProjGrad(usampler1D sampler,          vec4 P, float dPdx, float dPdy) noexcept;
[[spirv::builtin]] vec4  textureProjGrad(sampler2D sampler,           vec3 P,  vec2 dPdx,  vec2 dPdy) noexcept;
[[spirv::builtin]] ivec4 textureProjGrad(isampler2D sampler,          vec3 P,  vec2 dPdx,  vec2 dPdy) noexcept;
[[spirv::builtin]] uvec4 textureProjGrad(usampler2D sampler,          vec3 P,  vec2 dPdx,  vec2 dPdy) noexcept;
[[spirv::builtin]] vec4  textureProjGrad(sampler2D sampler,           vec4 P,  vec2 dPdx,  vec2 dPdy) noexcept;
[[spirv::builtin]] ivec4 textureProjGrad(isampler2D sampler,          vec4 P,  vec2 dPdx,  vec2 dPdy) noexcept;
[[spirv::builtin]] uvec4 textureProjGrad(usampler2D sampler,          vec4 P,  vec2 dPdx,  vec2 dPdy) noexcept;
[[spirv::builtin]] vec4  textureProjGrad(sampler3D sampler,           vec4 P,  vec3 dPdx,  vec3 dPdy) noexcept;
[[spirv::builtin]] ivec4 textureProjGrad(isampler3D sampler,          vec4 P,  vec3 dPdx,  vec3 dPdy) noexcept;
[[spirv::builtin]] uvec4 textureProjGrad(usampler3D sampler,          vec4 P,  vec3 dPdx,  vec3 dPdy) noexcept;
[[spirv::builtin]] vec4  textureProjGrad(sampler2DRect sampler,       vec3 P,  vec2 dPdx,  vec2 dPdy) noexcept;
[[spirv::builtin]] ivec4 textureProjGrad(isampler2DRect sampler,      vec3 P,  vec2 dPdx,  vec2 dPdy) noexcept;
[[spirv::builtin]] uvec4 textureProjGrad(usampler2DRect sampler,      vec3 P,  vec2 dPdx,  vec2 dPdy) noexcept;
[[spirv::builtin]] vec4  textureProjGrad(sampler2DRect sampler,       vec4 P,  vec2 dPdx,  vec2 dPdy) noexcept;
[[spirv::builtin]] ivec4 textureProjGrad(isampler2DRect sampler,      vec4 P,  vec2 dPdx,  vec2 dPdy) noexcept;
[[spirv::builtin]] uvec4 textureProjGrad(usampler2DRect sampler,      vec4 P,  vec2 dPdx,  vec2 dPdy) noexcept;
[[spirv::builtin]] float textureProjGrad(sampler2DRectShadow sampler, vec4 P, vec2 dPdx,   vec2 dPdy) noexcept;
[[spirv::builtin]] float textureProjGrad(sampler1DShadow sampler,     vec4 P, float dPdx,  float dPdy) noexcept;
[[spirv::builtin]] float textureProjGrad(sampler2DShadow sampler,     vec4 P, vec2 dPdx,   vec2 dPdy) noexcept;

// textureProjGradOffset
[[spirv::builtin]] vec4  textureProjGradOffset(sampler1D sampler,           vec2 P, float dPdx, float dPdy,  int offset) noexcept;
[[spirv::builtin]] ivec4 textureProjGradOffset(isampler1D sampler,          vec2 P, float dPdx, float dPdy,  int offset) noexcept;
[[spirv::builtin]] uvec4 textureProjGradOffset(usampler1D sampler,          vec2 P, float dPdx, float dPdy,  int offset) noexcept;
[[spirv::builtin]] vec4  textureProjGradOffset(sampler1D sampler,           vec4 P, float dPdx, float dPdy,  int offset) noexcept;
[[spirv::builtin]] ivec4 textureProjGradOffset(isampler1D sampler,          vec4 P, float dPdx, float dPdy,  int offset) noexcept;
[[spirv::builtin]] uvec4 textureProjGradOffset(usampler1D sampler,          vec4 P, float dPdx, float dPdy,  int offset) noexcept;
[[spirv::builtin]] vec4  textureProjGradOffset(sampler2D sampler,           vec3 P,  vec2 dPdx,  vec2 dPdy,  ivec2 offset) noexcept;
[[spirv::builtin]] ivec4 textureProjGradOffset(isampler2D sampler,          vec3 P,  vec2 dPdx,  vec2 dPdy,  ivec2 offset) noexcept;
[[spirv::builtin]] uvec4 textureProjGradOffset(usampler2D sampler,          vec3 P,  vec2 dPdx,  vec2 dPdy,  ivec2 offset) noexcept;
[[spirv::builtin]] vec4  textureProjGradOffset(sampler2D sampler,           vec4 P,  vec2 dPdx,  vec2 dPdy,  ivec2 offset) noexcept;
[[spirv::builtin]] ivec4 textureProjGradOffset(isampler2D sampler,          vec4 P,  vec2 dPdx,  vec2 dPdy,  ivec2 offset) noexcept;
[[spirv::builtin]] uvec4 textureProjGradOffset(usampler2D sampler,          vec4 P,  vec2 dPdx,  vec2 dPdy,  ivec2 offset) noexcept;
[[spirv::builtin]] vec4  textureProjGradOffset(sampler3D sampler,           vec4 P,  vec3 dPdx,  vec3 dPdy,  ivec3 offset) noexcept;
[[spirv::builtin]] ivec4 textureProjGradOffset(isampler3D sampler,          vec4 P,  vec3 dPdx,  vec3 dPdy,  ivec3 offset) noexcept;
[[spirv::builtin]] uvec4 textureProjGradOffset(usampler3D sampler,          vec4 P,  vec3 dPdx,  vec3 dPdy,  ivec3 offset) noexcept;
[[spirv::builtin]] vec4  textureProjGradOffset(sampler2DRect sampler,       vec3 P,  vec2 dPdx,  vec2 dPdy,  ivec2 offset) noexcept;
[[spirv::builtin]] ivec4 textureProjGradOffset(isampler2DRect sampler,      vec3 P,  vec2 dPdx,  vec2 dPdy,  ivec2 offset) noexcept;
[[spirv::builtin]] uvec4 textureProjGradOffset(usampler2DRect sampler,      vec3 P,  vec2 dPdx,  vec2 dPdy,  ivec2 offset) noexcept;
[[spirv::builtin]] vec4  textureProjGradOffset(sampler2DRect sampler,       vec4 P,  vec2 dPdx,  vec2 dPdy,  ivec2 offset) noexcept;
[[spirv::builtin]] ivec4 textureProjGradOffset(isampler2DRect sampler,      vec4 P,  vec2 dPdx,  vec2 dPdy,  ivec2 offset) noexcept;
[[spirv::builtin]] uvec4 textureProjGradOffset(usampler2DRect sampler,      vec4 P,  vec2 dPdx,  vec2 dPdy,  ivec2 offset) noexcept;
[[spirv::builtin]] float textureProjGradOffset(sampler2DRectShadow sampler, vec4 P, vec2 dPdx,   vec2 dPdy,  ivec2 offset) noexcept;
[[spirv::builtin]] float textureProjGradOffset(sampler1DShadow sampler,     vec4 P, float dPdx,  float dPdy, int offset)   noexcept;
[[spirv::builtin]] float textureProjGradOffset(sampler2DShadow sampler,     vec4 P, vec2 dPdx,   vec2 dPdy,  ivec2 offset) noexcept;

// 8.9.4. Texture Gather Function noexcept;
// textureGather
[[spirv::builtin]] vec4  textureGather(sampler2D sampler,             vec2 P) noexcept;
[[spirv::builtin]] ivec4 textureGather(isampler2D sampler,            vec2 P) noexcept;
[[spirv::builtin]] uvec4 textureGather(usampler2D sampler,            vec2 P) noexcept;
[[spirv::builtin]] vec4  textureGather(sampler2D sampler,             vec2 P, int comp) noexcept;
[[spirv::builtin]] ivec4 textureGather(isampler2D sampler,            vec2 P, int comp) noexcept;
[[spirv::builtin]] uvec4 textureGather(usampler2D sampler,            vec2 P, int comp) noexcept;

[[spirv::builtin]] vec4  textureGather(sampler2DArray sampler,        vec3 P) noexcept;
[[spirv::builtin]] ivec4 textureGather(isampler2DArray sampler,       vec3 P) noexcept;
[[spirv::builtin]] uvec4 textureGather(usampler2DArray sampler,       vec3 P) noexcept;
[[spirv::builtin]] vec4  textureGather(sampler2DArray sampler,        vec3 P, int comp) noexcept;
[[spirv::builtin]] ivec4 textureGather(isampler2DArray sampler,       vec3 P, int comp) noexcept;
[[spirv::builtin]] uvec4 textureGather(usampler2DArray sampler,       vec3 P, int comp) noexcept;

[[spirv::builtin]] vec4  textureGather(samplerCube sampler,           vec3 P) noexcept;
[[spirv::builtin]] ivec4 textureGather(isamplerCube sampler,          vec3 P) noexcept;
[[spirv::builtin]] uvec4 textureGather(usamplerCube sampler,          vec3 P) noexcept;
[[spirv::builtin]] vec4  textureGather(samplerCube sampler,           vec3 P, int comp) noexcept;
[[spirv::builtin]] ivec4 textureGather(isamplerCube sampler,          vec3 P, int comp) noexcept;
[[spirv::builtin]] uvec4 textureGather(usamplerCube sampler,          vec3 P, int comp) noexcept;

[[spirv::builtin]] vec4  textureGather(samplerCubeArray sampler,      vec4 P) noexcept;
[[spirv::builtin]] ivec4 textureGather(isamplerCubeArray sampler,     vec4 P) noexcept;
[[spirv::builtin]] uvec4 textureGather(usamplerCubeArray sampler,     vec4 P) noexcept;
[[spirv::builtin]] vec4  textureGather(samplerCubeArray sampler,      vec4 P, int cmp) noexcept;
[[spirv::builtin]] ivec4 textureGather(isamplerCubeArray sampler,     vec4 P, int cmp) noexcept;
[[spirv::builtin]] uvec4 textureGather(usamplerCubeArray sampler,     vec4 P, int cmp) noexcept;

[[spirv::builtin]] vec4  textureGather(sampler2DRect sampler,         vec2 P) noexcept;
[[spirv::builtin]] ivec4 textureGather(isampler2DRect sampler,        vec2 P) noexcept;
[[spirv::builtin]] uvec4 textureGather(usampler2DRect sampler,        vec2 P) noexcept;
[[spirv::builtin]] vec4  textureGather(sampler2DRect sampler,         vec2 P, int comp) noexcept;
[[spirv::builtin]] ivec4 textureGather(isampler2DRect sampler,        vec2 P, int comp) noexcept;
[[spirv::builtin]] uvec4 textureGather(usampler2DRect sampler,        vec2 P, int comp) noexcept;

[[spirv::builtin]] vec4 textureGather(sampler2DShadow sampler,        vec2 P, float refZ) noexcept;
[[spirv::builtin]] vec4 textureGather(sampler2DArrayShadow sampler,   vec2 P, float refZ) noexcept;
[[spirv::builtin]] vec4 textureGather(samplerCubeShadow sampler,      vec3 P, float refZ) noexcept;
[[spirv::builtin]] vec4 textureGather(samplerCubeArrayShadow sampler, vec4 P, float refZ) noexcept;
[[spirv::builtin]] vec4 textureGather(sampler2DRectShadow sampler,    vec2 P, float refZ) noexcept;

// textureGatherOffset
[[spirv::builtin]] vec4  textureGatherOffset(sampler2D sampler,            vec2 P, ivec2 offset) noexcept;
[[spirv::builtin]] ivec4 textureGatherOffset(isampler2D sampler,           vec2 P, ivec2 offset) noexcept;
[[spirv::builtin]] uvec4 textureGatherOffset(usampler2D sampler,           vec2 P, ivec2 offset) noexcept;
[[spirv::builtin]] vec4  textureGatherOffset(sampler2D sampler,            vec2 P, ivec2 offset, int comp) noexcept;
[[spirv::builtin]] ivec4 textureGatherOffset(isampler2D sampler,           vec2 P, ivec2 offset, int comp) noexcept;
[[spirv::builtin]] uvec4 textureGatherOffset(usampler2D sampler,           vec2 P, ivec2 offset, int comp) noexcept;

[[spirv::builtin]] vec4  textureGatherOffset(sampler2DArray sampler,       vec3 P, ivec2 offset) noexcept;
[[spirv::builtin]] ivec4 textureGatherOffset(isampler2DArray sampler,      vec3 P, ivec2 offset) noexcept;
[[spirv::builtin]] uvec4 textureGatherOffset(usampler2DArray sampler,      vec3 P, ivec2 offset) noexcept;
[[spirv::builtin]] vec4  textureGatherOffset(sampler2DArray sampler,       vec3 P, ivec2 offset, int comp) noexcept;
[[spirv::builtin]] ivec4 textureGatherOffset(isampler2DArray sampler,      vec3 P, ivec2 offset, int comp) noexcept;
[[spirv::builtin]] uvec4 textureGatherOffset(usampler2DArray sampler,      vec3 P, ivec2 offset, int comp) noexcept;

[[spirv::builtin]] vec4  textureGatherOffset(sampler2DRect sampler,        vec2 P, ivec2 offset) noexcept;
[[spirv::builtin]] ivec4 textureGatherOffset(isampler2DRect sampler,       vec2 P, ivec2 offset) noexcept;
[[spirv::builtin]] uvec4 textureGatherOffset(usampler2DRect sampler,       vec2 P, ivec2 offset) noexcept;
[[spirv::builtin]] vec4  textureGatherOffset(sampler2DRect sampler,        vec2 P, ivec2 offset, int comp) noexcept;
[[spirv::builtin]] ivec4 textureGatherOffset(isampler2DRect sampler,       vec2 P, ivec2 offset, int comp) noexcept;
[[spirv::builtin]] uvec4 textureGatherOffset(usampler2DRect sampler,       vec2 P, ivec2 offset, int comp) noexcept;

[[spirv::builtin]] vec4  textureGatherOffset(sampler2DShadow sampler,      vec2 P, float refZ, ivec2 offset) noexcept;
[[spirv::builtin]] vec4  textureGatherOffset(sampler2DArrayShadow sampler, vec2 P, float refZ, ivec2 offset) noexcept;
[[spirv::builtin]] vec4  textureGatherOffset(sampler2DRectShadow sampler,  vec2 P, float refZ, ivec2 offset) noexcept;

// textureGatherOffset
[[spirv::builtin]] vec4  textureGatherOffsets(sampler2D sampler,            vec2 P, const ivec2 (&offset)[4]) noexcept;
[[spirv::builtin]] ivec4 textureGatherOffsets(isampler2D sampler,           vec2 P, const ivec2 (&offset)[4]) noexcept;
[[spirv::builtin]] uvec4 textureGatherOffsets(usampler2D sampler,           vec2 P, const ivec2 (&offset)[4]) noexcept;
[[spirv::builtin]] vec4  textureGatherOffsets(sampler2D sampler,            vec2 P, const ivec2 (&offset)[4], int comp) noexcept;
[[spirv::builtin]] ivec4 textureGatherOffsets(isampler2D sampler,           vec2 P, const ivec2 (&offset)[4], int comp) noexcept;
[[spirv::builtin]] uvec4 textureGatherOffsets(usampler2D sampler,           vec2 P, const ivec2 (&offset)[4], int comp) noexcept;

[[spirv::builtin]] vec4  textureGatherOffsets(sampler2DArray sampler,       vec3 P, const ivec2 (&offset)[4]) noexcept;
[[spirv::builtin]] ivec4 textureGatherOffsets(isampler2DArray sampler,      vec3 P, const ivec2 (&offset)[4]) noexcept;
[[spirv::builtin]] uvec4 textureGatherOffsets(usampler2DArray sampler,      vec3 P, const ivec2 (&offset)[4]) noexcept;
[[spirv::builtin]] vec4  textureGatherOffsets(sampler2DArray sampler,       vec3 P, const ivec2 (&offset)[4], int comp) noexcept;
[[spirv::builtin]] ivec4 textureGatherOffsets(isampler2DArray sampler,      vec3 P, const ivec2 (&offset)[4], int comp) noexcept;
[[spirv::builtin]] uvec4 textureGatherOffsets(usampler2DArray sampler,      vec3 P, const ivec2 (&offset)[4], int comp) noexcept;

[[spirv::builtin]] vec4  textureGatherOffsets(sampler2DRect sampler,        vec2 P, const ivec2 (&offset)[4]) noexcept;
[[spirv::builtin]] ivec4 textureGatherOffsets(isampler2DRect sampler,       vec2 P, const ivec2 (&offset)[4]) noexcept;
[[spirv::builtin]] uvec4 textureGatherOffsets(usampler2DRect sampler,       vec2 P, const ivec2 (&offset)[4]) noexcept;
[[spirv::builtin]] vec4  textureGatherOffsets(sampler2DRect sampler,        vec2 P, const ivec2 (&offset)[4], int comp) noexcept;
[[spirv::builtin]] ivec4 textureGatherOffsets(isampler2DRect sampler,       vec2 P, const ivec2 (&offset)[4], int comp) noexcept;
[[spirv::builtin]] uvec4 textureGatherOffsets(usampler2DRect sampler,       vec2 P, const ivec2 (&offset)[4], int comp) noexcept;
[[spirv::builtin]] vec4  textureGatherOffsets(sampler2DShadow sampler,      vec2 P, float refZ, const ivec2 (&offset)[4]) noexcept;
[[spirv::builtin]] vec4  textureGatherOffsets(sampler2DArrayShadow sampler, vec2 P, float refZ, const ivec2 (&offset)[4]) noexcept;
[[spirv::builtin]] vec4  textureGatherOffsets(sampler2DRectShadow sampler,  vec2 P, float refZ, const ivec2 (&offset)[4]) noexcept;

////////////////////////////////////////////////////////////////////////////////
// Image types and image functions

// GLSL 4.60 specification
// 4.4.8. Format Layout Qualifiers

// These enums match the values of spv::ImageFormat.
enum class gl_format_t : unsigned {
  unknown,
  
  // float-image-format-qualifier:
  rgba32f,
  rgba16f,
  r32f,
  rgba8,
  rgba8_snorm,
  rg32f,
  rg16f,
  r11f_g11f_b10f,
  r16f,
  rgba16,
  rgb10_a2,
  rg16,
  rg8,
  r16,
  r8,
  rgba16_snorm,
  rg16_snorm,
  rg8_snorm,
  r16_snorm,
  r8_snorm,

  // int-image-format-qualifier:
  rgba32i,
  rgba16i,
  rgba8i,
  r32i,
  rg32i,
  rg16i,
  rg8i,
  r16i,
  r8i,

  // uint-image-format-qualifier:
  rgba32ui,
  rgba16ui,
  rgba8ui,
  r32ui,
  rgb10_a2ui,
  rg32ui,
  rg16ui,
  rg8ui,
  r16ui,
  r8ui,
};

enum class [[spirv::builtin]] image1D                : long long;
enum class [[spirv::builtin]] image1DArray           : long long;
enum class [[spirv::builtin]] image2D                : long long;
enum class [[spirv::builtin]] image2DArray           : long long;
enum class [[spirv::builtin]] image2DMS              : long long;
enum class [[spirv::builtin]] image2DMSArray         : long long;
enum class [[spirv::builtin]] image2DRect            : long long;
enum class [[spirv::builtin]] image3D                : long long;
enum class [[spirv::builtin]] imageCube              : long long;
enum class [[spirv::builtin]] imageCubeArray         : long long;
enum class [[spirv::builtin]] imageBuffer            : long long;

enum class [[spirv::builtin]] iimage1D               : long long;
enum class [[spirv::builtin]] iimage1DArray          : long long;
enum class [[spirv::builtin]] iimage2D               : long long;
enum class [[spirv::builtin]] iimage2DArray          : long long;
enum class [[spirv::builtin]] iimage2DMS             : long long;
enum class [[spirv::builtin]] iimage2DMSArray        : long long;
enum class [[spirv::builtin]] iimage2DRect           : long long;
enum class [[spirv::builtin]] iimage3D               : long long;
enum class [[spirv::builtin]] iimageCube             : long long;
enum class [[spirv::builtin]] iimageCubeArray        : long long;
enum class [[spirv::builtin]] iimageBuffer           : long long;

enum class [[spirv::builtin]] uimage1D               : long long;
enum class [[spirv::builtin]] uimage1DArray          : long long;
enum class [[spirv::builtin]] uimage2D               : long long;
enum class [[spirv::builtin]] uimage2DArray          : long long;
enum class [[spirv::builtin]] uimage2DMS             : long long;
enum class [[spirv::builtin]] uimage2DMSArray        : long long;
enum class [[spirv::builtin]] uimage2DRect           : long long;
enum class [[spirv::builtin]] uimage3D               : long long;
enum class [[spirv::builtin]] uimageCube             : long long;
enum class [[spirv::builtin]] uimageCubeArray        : long long;
enum class [[spirv::builtin]] uimageBuffer           : long long;

enum class [[spirv::builtin]] subpassInput           : long long;
enum class [[spirv::builtin]] subpassInputMS         : long long;
enum class [[spirv::builtin]] isubpassInput          : long long;
enum class [[spirv::builtin]] isubpassInputMS        : long long;
enum class [[spirv::builtin]] usubpassInput          : long long;
enum class [[spirv::builtin]] usubpassInputMS        : long long;


// imageSize
[[spirv::builtin]] int   imageSize(image1D         image) noexcept;
[[spirv::builtin]] ivec2 imageSize(image2D         image) noexcept;
[[spirv::builtin]] ivec3 imageSize(image3D         image) noexcept;
[[spirv::builtin]] ivec2 imageSize(imageCube       image) noexcept;
[[spirv::builtin]] ivec3 imageSize(imageCubeArray  image) noexcept;
[[spirv::builtin]] ivec3 imageSize(image2DArray    image) noexcept;
[[spirv::builtin]] ivec2 imageSize(image2DRect     image) noexcept;
[[spirv::builtin]] ivec2 imageSize(image1DArray    image) noexcept;
[[spirv::builtin]] ivec2 imageSize(image2DMS       image) noexcept;
[[spirv::builtin]] ivec3 imageSize(image2DMSArray  image) noexcept;
[[spirv::builtin]] int   imageSize(imageBuffer     image) noexcept;

[[spirv::builtin]] int   imageSize(iimage1D        image) noexcept;
[[spirv::builtin]] ivec2 imageSize(iimage2D        image) noexcept;
[[spirv::builtin]] ivec3 imageSize(iimage3D        image) noexcept;
[[spirv::builtin]] ivec2 imageSize(iimageCube      image) noexcept;
[[spirv::builtin]] ivec3 imageSize(iimageCubeArray image) noexcept;
[[spirv::builtin]] ivec3 imageSize(iimage2DArray   image) noexcept;
[[spirv::builtin]] ivec2 imageSize(iimage2DRect    image) noexcept;
[[spirv::builtin]] ivec2 imageSize(iimage1DArray   image) noexcept;
[[spirv::builtin]] ivec2 imageSize(iimage2DMS      image) noexcept;
[[spirv::builtin]] ivec3 imageSize(iimage2DMSArray image) noexcept;
[[spirv::builtin]] int   imageSize(iimageBuffer    image) noexcept;

[[spirv::builtin]] int   imageSize(uimage1D        image) noexcept;
[[spirv::builtin]] ivec2 imageSize(uimage2D        image) noexcept;
[[spirv::builtin]] ivec3 imageSize(uimage3D        image) noexcept;
[[spirv::builtin]] ivec2 imageSize(uimageCube      image) noexcept;
[[spirv::builtin]] ivec3 imageSize(uimageCubeArray image) noexcept;
[[spirv::builtin]] ivec3 imageSize(uimage2DArray   image) noexcept;
[[spirv::builtin]] ivec2 imageSize(uimage2DRect    image) noexcept;
[[spirv::builtin]] ivec2 imageSize(uimage1DArray   image) noexcept;
[[spirv::builtin]] ivec2 imageSize(uimage2DMS      image) noexcept;
[[spirv::builtin]] ivec3 imageSize(uimage2DMSArray image) noexcept;
[[spirv::builtin]] int   imageSize(uimageBuffer    image) noexcept;

// imageSamples
[[spirv::builtin]] int imageSamples(image2DMS       image) noexcept;
[[spirv::builtin]] int imageSamples(image2DMSArray  image) noexcept;

[[spirv::builtin]] int imageSamples(iimage2DMS      image) noexcept;
[[spirv::builtin]] int imageSamples(iimage2DMSArray image) noexcept;

[[spirv::builtin]] int imageSamples(uimage2DMS      image) noexcept;
[[spirv::builtin]] int imageSamples(uimage2DMSArray image) noexcept;

// imageLoad
[[spirv::builtin]] vec4  imageLoad(image1D,         int   P            ) noexcept;
[[spirv::builtin]] vec4  imageLoad(image1DArray,    ivec2 P            ) noexcept;
[[spirv::builtin]] vec4  imageLoad(image2D,         ivec2 P            ) noexcept;
[[spirv::builtin]] vec4  imageLoad(image2DArray,    ivec3 P            ) noexcept;
[[spirv::builtin]] vec4  imageLoad(image2DMS,       ivec2 P, int sample) noexcept;
[[spirv::builtin]] vec4  imageLoad(image2DMSArray,  ivec3 P, int sample) noexcept;
[[spirv::builtin]] vec4  imageLoad(image2DRect,     ivec2 P            ) noexcept;
[[spirv::builtin]] vec4  imageLoad(image3D,         ivec3 P            ) noexcept;
[[spirv::builtin]] vec4  imageLoad(imageCube,       ivec3 P            ) noexcept;
[[spirv::builtin]] vec4  imageLoad(imageCubeArray,  ivec3 P            ) noexcept;
[[spirv::builtin]] vec4  imageLoad(imageBuffer,     int   P            ) noexcept;

[[spirv::builtin]] vec4  imageLoad(iimage1D,        int   P            ) noexcept;
[[spirv::builtin]] vec4  imageLoad(iimage1DArray,   ivec2 P            ) noexcept;
[[spirv::builtin]] vec4  imageLoad(iimage2D,        ivec2 P            ) noexcept;
[[spirv::builtin]] vec4  imageLoad(iimage2DArray,   ivec3 P            ) noexcept;
[[spirv::builtin]] vec4  imageLoad(iimage2DMS,      ivec2 P, int sample) noexcept;
[[spirv::builtin]] vec4  imageLoad(iimage2DMSArray, ivec3 P, int sample) noexcept;
[[spirv::builtin]] vec4  imageLoad(iimage2DRect,    ivec2 P            ) noexcept;
[[spirv::builtin]] vec4  imageLoad(iimage3D,        ivec3 P            ) noexcept;
[[spirv::builtin]] vec4  imageLoad(iimageCube,      ivec3 P            ) noexcept;
[[spirv::builtin]] vec4  imageLoad(iimageCubeArray, ivec3 P            ) noexcept;
[[spirv::builtin]] vec4  imageLoad(iimageBuffer,    int   P            ) noexcept;

[[spirv::builtin]] vec4  imageLoad(uimage1D,        int   P            ) noexcept;
[[spirv::builtin]] vec4  imageLoad(uimage1DArray,   ivec2 P            ) noexcept;
[[spirv::builtin]] vec4  imageLoad(uimage2D,        ivec2 P            ) noexcept;
[[spirv::builtin]] vec4  imageLoad(uimage2DArray,   ivec3 P            ) noexcept;
[[spirv::builtin]] vec4  imageLoad(uimage2DMS,      ivec2 P, int sample) noexcept;
[[spirv::builtin]] vec4  imageLoad(uimage2DMSArray, ivec3 P, int sample) noexcept;
[[spirv::builtin]] vec4  imageLoad(uimage2DRect,    ivec2 P            ) noexcept;
[[spirv::builtin]] vec4  imageLoad(uimage3D,        ivec3 P            ) noexcept;
[[spirv::builtin]] vec4  imageLoad(uimageCube,      ivec3 P            ) noexcept;
[[spirv::builtin]] vec4  imageLoad(uimageCubeArray, ivec3 P            ) noexcept;
[[spirv::builtin]] vec4  imageLoad(uimageBuffer,    int   P            ) noexcept;

// imageStore
[[spirv::builtin]] vec4  imageStore(image1D,         int   P            , vec4  data) noexcept;
[[spirv::builtin]] vec4  imageStore(image1DArray,    ivec2 P            , vec4  data) noexcept;
[[spirv::builtin]] vec4  imageStore(image2D,         ivec2 P            , vec4  data) noexcept;
[[spirv::builtin]] vec4  imageStore(image2DArray,    ivec3 P            , vec4  data) noexcept;
[[spirv::builtin]] vec4  imageStore(image2DMS,       ivec2 P, int sample, vec4  data) noexcept;
[[spirv::builtin]] vec4  imageStore(image2DMSArray,  ivec3 P, int sample, vec4  data) noexcept;
[[spirv::builtin]] vec4  imageStore(image2DRect,     ivec2 P            , vec4  data) noexcept;
[[spirv::builtin]] vec4  imageStore(image3D,         ivec3 P            , vec4  data) noexcept;
[[spirv::builtin]] vec4  imageStore(imageCube,       ivec3 P            , vec4  data) noexcept;
[[spirv::builtin]] vec4  imageStore(imageCubeArray,  ivec3 P            , vec4  data) noexcept;
[[spirv::builtin]] vec4  imageStore(imageBuffer,     int   P            , vec4  data) noexcept;

[[spirv::builtin]] vec4  imageStore(iimage1D,        int   P            , ivec4 data) noexcept;
[[spirv::builtin]] vec4  imageStore(iimage1DArray,   ivec2 P            , ivec4 data) noexcept;
[[spirv::builtin]] vec4  imageStore(iimage2D,        ivec2 P            , ivec4 data) noexcept;
[[spirv::builtin]] vec4  imageStore(iimage2DArray,   ivec3 P            , ivec4 data) noexcept;
[[spirv::builtin]] vec4  imageStore(iimage2DMS,      ivec2 P, int sample, ivec4 data) noexcept;
[[spirv::builtin]] vec4  imageStore(iimage2DMSArray, ivec3 P, int sample, ivec4 data) noexcept;
[[spirv::builtin]] vec4  imageStore(iimage2DRect,    ivec2 P            , ivec4 data) noexcept;
[[spirv::builtin]] vec4  imageStore(iimage3D,        ivec3 P            , ivec4 data) noexcept;
[[spirv::builtin]] vec4  imageStore(iimageCube,      ivec3 P            , ivec4 data) noexcept;
[[spirv::builtin]] vec4  imageStore(iimageCubeArray, ivec3 P            , ivec4 data) noexcept;
[[spirv::builtin]] vec4  imageStore(iimageBuffer,    int   P            , ivec4 data) noexcept;

[[spirv::builtin]] vec4  imageStore(uimage1D         int   P            , uvec4 data) noexcept;
[[spirv::builtin]] vec4  imageStore(uimage1DArray,   ivec2 P            , uvec4 data) noexcept;
[[spirv::builtin]] vec4  imageStore(uimage2D,        ivec2 P            , uvec4 data) noexcept;
[[spirv::builtin]] vec4  imageStore(uimage2DArray,   ivec3 P            , uvec4 data) noexcept;
[[spirv::builtin]] vec4  imageStore(uimage2DMS,      ivec2 P, int sample, uvec4 data) noexcept;
[[spirv::builtin]] vec4  imageStore(uimage2DMSArray, ivec3 P, int sample, uvec4 data) noexcept;
[[spirv::builtin]] vec4  imageStore(uimage2DRect,    ivec2 P            , uvec4 data) noexcept;
[[spirv::builtin]] vec4  imageStore(uimage3D,        ivec3 P            , uvec4 data) noexcept;
[[spirv::builtin]] vec4  imageStore(uimageCube,      ivec3 P            , uvec4 data) noexcept;
[[spirv::builtin]] vec4  imageStore(uimageCubeArray, ivec3 P            , uvec4 data) noexcept;
[[spirv::builtin]] vec4  imageStore(uimageBuffer,    int   P            , uvec4 data) noexcept;

// imageAtomicAdd
[[spirv::builtin]] int  imageAtomicAdd(iimage1D,        int   P            , int  data) noexcept;
[[spirv::builtin]] int  imageAtomicAdd(iimage1DArray,   ivec2 P            , int  data) noexcept;
[[spirv::builtin]] int  imageAtomicAdd(iimage2D,        ivec2 P            , int  data) noexcept;
[[spirv::builtin]] int  imageAtomicAdd(iimage2DArray,   ivec3 P            , int  data) noexcept;
[[spirv::builtin]] int  imageAtomicAdd(iimage2DMS,      ivec2 P, int sample, int  data) noexcept;
[[spirv::builtin]] int  imageAtomicAdd(iimage2DMSArray, ivec3 P, int sample, int  data) noexcept;
[[spirv::builtin]] int  imageAtomicAdd(iimage2DRect,    ivec2 P            , int  data) noexcept;
[[spirv::builtin]] int  imageAtomicAdd(iimage3D,        ivec3 P            , int  data) noexcept;
[[spirv::builtin]] int  imageAtomicAdd(iimageCube,      ivec3 P            , int  data) noexcept;
[[spirv::builtin]] int  imageAtomicAdd(iimageCubeArray, ivec3 P            , int  data) noexcept;
[[spirv::builtin]] int  imageAtomicAdd(iimageBuffer,    int   P            , int  data) noexcept;

[[spirv::builtin]] uint imageAtomicAdd(uimage1D,        int   P            , uint data) noexcept;
[[spirv::builtin]] uint imageAtomicAdd(uimage1DArray,   ivec2 P            , uint data) noexcept;
[[spirv::builtin]] uint imageAtomicAdd(uimage2D,        ivec2 P            , uint data) noexcept;
[[spirv::builtin]] uint imageAtomicAdd(uimage2DArray,   ivec3 P            , uint data) noexcept;
[[spirv::builtin]] uint imageAtomicAdd(uimage2DMS,      ivec2 P, int sample, uint data) noexcept;
[[spirv::builtin]] uint imageAtomicAdd(uimage2DMSArray, ivec3 P, int sample, uint data) noexcept;
[[spirv::builtin]] uint imageAtomicAdd(uimage2DRect,    ivec2 P            , uint data) noexcept;
[[spirv::builtin]] uint imageAtomicAdd(uimage3D,        ivec3 P            , uint data) noexcept;
[[spirv::builtin]] uint imageAtomicAdd(uimageCube,      ivec3 P            , uint data) noexcept;
[[spirv::builtin]] uint imageAtomicAdd(uimageCubeArray, ivec3 P            , uint data) noexcept;
[[spirv::builtin]] uint imageAtomicAdd(uimageBuffer,    int   P            , uint data) noexcept;

// imageAtomicMin
[[spirv::builtin]] int  imageAtomicMin(iimage1D,        int   P            , int  data) noexcept;
[[spirv::builtin]] int  imageAtomicMin(iimage1DArray,   ivec2 P            , int  data) noexcept;
[[spirv::builtin]] int  imageAtomicMin(iimage2D,        ivec2 P            , int  data) noexcept;
[[spirv::builtin]] int  imageAtomicMin(iimage2DArray,   ivec3 P            , int  data) noexcept;
[[spirv::builtin]] int  imageAtomicMin(iimage2DMS,      ivec2 P, int sample, int  data) noexcept;
[[spirv::builtin]] int  imageAtomicMin(iimage2DMSArray, ivec3 P, int sample, int  data) noexcept;
[[spirv::builtin]] int  imageAtomicMin(iimage2DRect,    ivec2 P            , int  data) noexcept;
[[spirv::builtin]] int  imageAtomicMin(iimage3D,        ivec3 P            , int  data) noexcept;
[[spirv::builtin]] int  imageAtomicMin(iimageCube,      ivec3 P            , int  data) noexcept;
[[spirv::builtin]] int  imageAtomicMin(iimageCubeArray, ivec3 P            , int  data) noexcept;
[[spirv::builtin]] int  imageAtomicMin(iimageBuffer,    int   P            , int  data) noexcept;

[[spirv::builtin]] uint imageAtomicMin(uimage1D,        int   P            , uint data) noexcept;
[[spirv::builtin]] uint imageAtomicMin(uimage1DArray,   ivec2 P            , uint data) noexcept;
[[spirv::builtin]] uint imageAtomicMin(uimage2D,        ivec2 P            , uint data) noexcept;
[[spirv::builtin]] uint imageAtomicMin(uimage2DArray,   ivec3 P            , uint data) noexcept;
[[spirv::builtin]] uint imageAtomicMin(uimage2DMS,      ivec2 P, int sample, uint data) noexcept;
[[spirv::builtin]] uint imageAtomicMin(uimage2DMSArray, ivec3 P, int sample, uint data) noexcept;
[[spirv::builtin]] uint imageAtomicMin(uimage2DRect,    ivec2 P            , uint data) noexcept;
[[spirv::builtin]] uint imageAtomicMin(uimage3D,        ivec3 P            , uint data) noexcept;
[[spirv::builtin]] uint imageAtomicMin(uimageCube,      ivec3 P            , uint data) noexcept;
[[spirv::builtin]] uint imageAtomicMin(uimageCubeArray, ivec3 P            , uint data) noexcept;
[[spirv::builtin]] uint imageAtomicMin(uimageBuffer,    int   P            , uint data) noexcept;

// imageAtomicMax
[[spirv::builtin]] int  imageAtomicMax(iimage1D,        int   P            , int  data) noexcept;
[[spirv::builtin]] int  imageAtomicMax(iimage1DArray,   ivec2 P            , int  data) noexcept;
[[spirv::builtin]] int  imageAtomicMax(iimage2D,        ivec2 P            , int  data) noexcept;
[[spirv::builtin]] int  imageAtomicMax(iimage2DArray,   ivec3 P            , int  data) noexcept;
[[spirv::builtin]] int  imageAtomicMax(iimage2DMS,      ivec2 P, int sample, int  data) noexcept;
[[spirv::builtin]] int  imageAtomicMax(iimage2DMSArray, ivec3 P, int sample, int  data) noexcept;
[[spirv::builtin]] int  imageAtomicMax(iimage2DRect,    ivec2 P            , int  data) noexcept;
[[spirv::builtin]] int  imageAtomicMax(iimage3D,        ivec3 P            , int  data) noexcept;
[[spirv::builtin]] int  imageAtomicMax(iimageCube,      ivec3 P            , int  data) noexcept;
[[spirv::builtin]] int  imageAtomicMax(iimageCubeArray, ivec3 P            , int  data) noexcept;
[[spirv::builtin]] int  imageAtomicMax(iimageBuffer,    int   P            , int  data) noexcept;

[[spirv::builtin]] uint  imageAtomicMax(uimage1D,        int   P            , uint data) noexcept;
[[spirv::builtin]] uint  imageAtomicMax(uimage1DArray,   ivec2 P            , uint data) noexcept;
[[spirv::builtin]] uint  imageAtomicMax(uimage2D,        ivec2 P            , uint data) noexcept;
[[spirv::builtin]] uint  imageAtomicMax(uimage2DArray,   ivec3 P            , uint data) noexcept;
[[spirv::builtin]] uint  imageAtomicMax(uimage2DMS,      ivec2 P, int sample, uint data) noexcept;
[[spirv::builtin]] uint  imageAtomicMax(uimage2DMSArray, ivec3 P, int sample, uint data) noexcept;
[[spirv::builtin]] uint  imageAtomicMax(uimage2DRect,    ivec2 P            , uint data) noexcept;
[[spirv::builtin]] uint  imageAtomicMax(uimage3D,        ivec3 P            , uint data) noexcept;
[[spirv::builtin]] uint  imageAtomicMax(uimageCube,      ivec3 P            , uint data) noexcept;
[[spirv::builtin]] uint  imageAtomicMax(uimageCubeArray, ivec3 P            , uint data) noexcept;
[[spirv::builtin]] uint  imageAtomicMax(uimageBuffer,    int   P            , uint data) noexcept;

// imageAtomicAnd
[[spirv::builtin]] int  imageAtomicAnd(iimage1D,        int   P            , int  data) noexcept;
[[spirv::builtin]] int  imageAtomicAnd(iimage1DArray,   ivec2 P            , int  data) noexcept;
[[spirv::builtin]] int  imageAtomicAnd(iimage2D,        ivec2 P            , int  data) noexcept;
[[spirv::builtin]] int  imageAtomicAnd(iimage2DArray,   ivec3 P            , int  data) noexcept;
[[spirv::builtin]] int  imageAtomicAnd(iimage2DMS,      ivec2 P, int sample, int  data) noexcept;
[[spirv::builtin]] int  imageAtomicAnd(iimage2DMSArray, ivec3 P, int sample, int  data) noexcept;
[[spirv::builtin]] int  imageAtomicAnd(iimage2DRect,    ivec2 P            , int  data) noexcept;
[[spirv::builtin]] int  imageAtomicAnd(iimage3D,        ivec3 P            , int  data) noexcept;
[[spirv::builtin]] int  imageAtomicAnd(iimageCube,      ivec3 P            , int  data) noexcept;
[[spirv::builtin]] int  imageAtomicAnd(iimageCubeArray, ivec3 P            , int  data) noexcept;
[[spirv::builtin]] int  imageAtomicAnd(iimageBuffer,    int   P            , int  data) noexcept;

[[spirv::builtin]] uint imageAtomicAnd(uimage1D,        int   P            , uint data) noexcept;
[[spirv::builtin]] uint imageAtomicAnd(uimage1DArray,   ivec2 P            , uint data) noexcept;
[[spirv::builtin]] uint imageAtomicAnd(uimage2D,        ivec2 P            , uint data) noexcept;
[[spirv::builtin]] uint imageAtomicAnd(uimage2DArray,   ivec3 P            , uint data) noexcept;
[[spirv::builtin]] uint imageAtomicAnd(uimage2DMS,      ivec2 P, int sample, uint data) noexcept;
[[spirv::builtin]] uint imageAtomicAnd(uimage2DMSArray, ivec3 P, int sample, uint data) noexcept;
[[spirv::builtin]] uint imageAtomicAnd(uimage2DRect,    ivec2 P            , uint data) noexcept;
[[spirv::builtin]] uint imageAtomicAnd(uimage3D,        ivec3 P            , uint data) noexcept;
[[spirv::builtin]] uint imageAtomicAnd(uimageCube,      ivec3 P            , uint data) noexcept;
[[spirv::builtin]] uint imageAtomicAnd(uimageCubeArray, ivec3 P            , uint data) noexcept;
[[spirv::builtin]] uint imageAtomicAnd(uimageBuffer,    int   P            , uint data) noexcept;

// imageAtomicOr
[[spirv::builtin]] int  imageAtomicOr(iimage1D,        int   P            , int  data) noexcept;
[[spirv::builtin]] int  imageAtomicOr(iimage1DArray,   ivec2 P            , int  data) noexcept;
[[spirv::builtin]] int  imageAtomicOr(iimage2D,        ivec2 P            , int  data) noexcept;
[[spirv::builtin]] int  imageAtomicOr(iimage2DArray,   ivec3 P            , int  data) noexcept;
[[spirv::builtin]] int  imageAtomicOr(iimage2DMS,      ivec2 P, int sample, int  data) noexcept;
[[spirv::builtin]] int  imageAtomicOr(iimage2DMSArray, ivec3 P, int sample, int  data) noexcept;
[[spirv::builtin]] int  imageAtomicOr(iimage2DRect,    ivec2 P            , int  data) noexcept;
[[spirv::builtin]] int  imageAtomicOr(iimage3D,        ivec3 P            , int  data) noexcept;
[[spirv::builtin]] int  imageAtomicOr(iimageCube,      ivec3 P            , int  data) noexcept;
[[spirv::builtin]] int  imageAtomicOr(iimageCubeArray, ivec3 P            , int  data) noexcept;
[[spirv::builtin]] int  imageAtomicOr(iimageBuffer,    int   P            , int  data) noexcept;

[[spirv::builtin]] uint imageAtomicOr(uimage1D,        int   P            , uint data) noexcept;
[[spirv::builtin]] uint imageAtomicOr(uimage1DArray,   ivec2 P            , uint data) noexcept;
[[spirv::builtin]] uint imageAtomicOr(uimage2D,        ivec2 P            , uint data) noexcept;
[[spirv::builtin]] uint imageAtomicOr(uimage2DArray,   ivec3 P            , uint data) noexcept;
[[spirv::builtin]] uint imageAtomicOr(uimage2DMS,      ivec2 P, int sample, uint data) noexcept;
[[spirv::builtin]] uint imageAtomicOr(uimage2DMSArray, ivec3 P, int sample, uint data) noexcept;
[[spirv::builtin]] uint imageAtomicOr(uimage2DRect,    ivec2 P            , uint data) noexcept;
[[spirv::builtin]] uint imageAtomicOr(uimage3D,        ivec3 P            , uint data) noexcept;
[[spirv::builtin]] uint imageAtomicOr(uimageCube,      ivec3 P            , uint data) noexcept;
[[spirv::builtin]] uint imageAtomicOr(uimageCubeArray, ivec3 P            , uint data) noexcept;
[[spirv::builtin]] uint imageAtomicOr(uimageBuffer,    int   P            , uint data) noexcept;

// imageAtomicXor
[[spirv::builtin]] int  imageAtomicXor(iimage1D,        int   P            , int  data) noexcept;
[[spirv::builtin]] int  imageAtomicXor(iimage1DArray,   ivec2 P            , int  data) noexcept;
[[spirv::builtin]] int  imageAtomicXor(iimage2D,        ivec2 P            , int  data) noexcept;
[[spirv::builtin]] int  imageAtomicXor(iimage2DArray,   ivec3 P            , int  data) noexcept;
[[spirv::builtin]] int  imageAtomicXor(iimage2DMS,      ivec2 P, int sample, int  data) noexcept;
[[spirv::builtin]] int  imageAtomicXor(iimage2DMSArray, ivec3 P, int sample, int  data) noexcept;
[[spirv::builtin]] int  imageAtomicXor(iimage2DRect,    ivec2 P            , int  data) noexcept;
[[spirv::builtin]] int  imageAtomicXor(iimage3D,        ivec3 P            , int  data) noexcept;
[[spirv::builtin]] int  imageAtomicXor(iimageCube,      ivec3 P            , int  data) noexcept;
[[spirv::builtin]] int  imageAtomicXor(iimageCubeArray, ivec3 P            , int  data) noexcept;
[[spirv::builtin]] int  imageAtomicXor(iimageBuffer,    int   P            , int  data) noexcept;

[[spirv::builtin]] uint imageAtomicXor(uimage1D,        int   P            , uint data) noexcept;
[[spirv::builtin]] uint imageAtomicXor(uimage1DArray,   ivec2 P            , uint data) noexcept;
[[spirv::builtin]] uint imageAtomicXor(uimage2D,        ivec2 P            , uint data) noexcept;
[[spirv::builtin]] uint imageAtomicXor(uimage2DArray,   ivec3 P            , uint data) noexcept;
[[spirv::builtin]] uint imageAtomicXor(uimage2DMS,      ivec2 P, int sample, uint data) noexcept;
[[spirv::builtin]] uint imageAtomicXor(uimage2DMSArray, ivec3 P, int sample, uint data) noexcept;
[[spirv::builtin]] uint imageAtomicXor(uimage2DRect,    ivec2 P            , uint data) noexcept;
[[spirv::builtin]] uint imageAtomicXor(uimage3D,        ivec3 P            , uint data) noexcept;
[[spirv::builtin]] uint imageAtomicXor(uimageCube,      ivec3 P            , uint data) noexcept;
[[spirv::builtin]] uint imageAtomicXor(uimageCubeArray, ivec3 P            , uint data) noexcept;
[[spirv::builtin]] uint imageAtomicXor(uimageBuffer,    int   P            , uint data) noexcept;

// imageAtomicExchange
[[spirv::builtin]] float imageAtomicExchange(image1D,         int   P            , int  data) noexcept;
[[spirv::builtin]] float imageAtomicExchange(image1DArray,    ivec2 P            , int  data) noexcept;
[[spirv::builtin]] float imageAtomicExchange(image2D,         ivec2 P            , int  data) noexcept;
[[spirv::builtin]] float imageAtomicExchange(image2DArray,    ivec3 P            , int  data) noexcept;
[[spirv::builtin]] float imageAtomicExchange(image2DMS,       ivec2 P, int sample, int  data) noexcept;
[[spirv::builtin]] float imageAtomicExchange(image2DMSArray,  ivec3 P, int sample, int  data) noexcept;
[[spirv::builtin]] float imageAtomicExchange(image2DRect,     ivec2 P            , int  data) noexcept;
[[spirv::builtin]] float imageAtomicExchange(image3D,         ivec3 P            , int  data) noexcept;
[[spirv::builtin]] float imageAtomicExchange(imageCube,       ivec3 P            , int  data) noexcept;
[[spirv::builtin]] float imageAtomicExchange(imageCubeArray,  ivec3 P            , int  data) noexcept;
[[spirv::builtin]] float imageAtomicExchange(imageBuffer,     int   P            , int  data) noexcept;

[[spirv::builtin]] int   imageAtomicExchange(iimage1D,        int   P            , int  data) noexcept;
[[spirv::builtin]] int   imageAtomicExchange(iimage1DArray,   ivec2 P            , int  data) noexcept;
[[spirv::builtin]] int   imageAtomicExchange(iimage2D,        ivec2 P            , int  data) noexcept;
[[spirv::builtin]] int   imageAtomicExchange(iimage2DArray,   ivec3 P            , int  data) noexcept;
[[spirv::builtin]] int   imageAtomicExchange(iimage2DMS,      ivec2 P, int sample, int  data) noexcept;
[[spirv::builtin]] int   imageAtomicExchange(iimage2DMSArray, ivec3 P, int sample, int  data) noexcept;
[[spirv::builtin]] int   imageAtomicExchange(iimage2DRect,    ivec2 P            , int  data) noexcept;
[[spirv::builtin]] int   imageAtomicExchange(iimage3D,        ivec3 P            , int  data) noexcept;
[[spirv::builtin]] int   imageAtomicExchange(iimageCube,      ivec3 P            , int  data) noexcept;
[[spirv::builtin]] int   imageAtomicExchange(iimageCubeArray, ivec3 P            , int  data) noexcept;
[[spirv::builtin]] int   imageAtomicExchange(iimageBuffer,    int   P            , int  data) noexcept;
 
[[spirv::builtin]] uint  imageAtomicExchange(uimage1D,        int   P            , uint data) noexcept;
[[spirv::builtin]] uint  imageAtomicExchange(uimage1DArray,   ivec2 P            , uint data) noexcept;
[[spirv::builtin]] uint  imageAtomicExchange(uimage2D,        ivec2 P            , uint data) noexcept;
[[spirv::builtin]] uint  imageAtomicExchange(uimage2DArray,   ivec3 P            , uint data) noexcept;
[[spirv::builtin]] uint  imageAtomicExchange(uimage2DMS,      ivec2 P, int sample, uint data) noexcept;
[[spirv::builtin]] uint  imageAtomicExchange(uimage2DMSArray, ivec3 P, int sample, uint data) noexcept;
[[spirv::builtin]] uint  imageAtomicExchange(uimage2DRect,    ivec2 P            , uint data) noexcept;
[[spirv::builtin]] uint  imageAtomicExchange(uimage3D,        ivec3 P            , uint data) noexcept;
[[spirv::builtin]] uint  imageAtomicExchange(uimageCube,      ivec3 P            , uint data) noexcept;
[[spirv::builtin]] uint  imageAtomicExchange(uimageCubeArray, ivec3 P            , uint data) noexcept;
[[spirv::builtin]] uint  imageAtomicExchange(uimageBuffer,    int   P            , uint data) noexcept;

// imageAtomicCompSwap
[[spirv::builtin]] int   imageAtomicExchange(iimage1D,        int   P            , int  compare, int  data) noexcept;
[[spirv::builtin]] int   imageAtomicExchange(iimage1DArray,   ivec2 P            , int  compare, int  data) noexcept;
[[spirv::builtin]] int   imageAtomicExchange(iimage2D,        ivec2 P            , int  compare, int  data) noexcept;
[[spirv::builtin]] int   imageAtomicExchange(iimage2DArray,   ivec3 P            , int  compare, int  data) noexcept;
[[spirv::builtin]] int   imageAtomicExchange(iimage2DMS,      ivec2 P, int sample, int  compare, int  data) noexcept;
[[spirv::builtin]] int   imageAtomicExchange(iimage2DMSArray, ivec3 P, int sample, int  compare, int  data) noexcept;
[[spirv::builtin]] int   imageAtomicExchange(iimage2DRect,    ivec2 P            , int  compare, int  data) noexcept;
[[spirv::builtin]] int   imageAtomicExchange(iimage3D,        ivec3 P            , int  compare, int  data) noexcept;
[[spirv::builtin]] int   imageAtomicExchange(iimageCube,      ivec3 P            , int  compare, int  data) noexcept;
[[spirv::builtin]] int   imageAtomicExchange(iimageCubeArray, ivec3 P            , int  compare, int  data) noexcept;
[[spirv::builtin]] int   imageAtomicExchange(iimageBuffer,    int   P            , int  compare, int  data) noexcept;
 
[[spirv::builtin]] uint  imageAtomicExchange(uimage1D,        int   P            , uint compare, uint data) noexcept;
[[spirv::builtin]] uint  imageAtomicExchange(uimage1DArray,   ivec2 P            , uint compare, uint data) noexcept;
[[spirv::builtin]] uint  imageAtomicExchange(uimage2D,        ivec2 P            , uint compare, uint data) noexcept;
[[spirv::builtin]] uint  imageAtomicExchange(uimage2DArray,   ivec3 P            , uint compare, uint data) noexcept;
[[spirv::builtin]] uint  imageAtomicExchange(uimage2DMS,      ivec2 P, int sample, uint compare, uint data) noexcept;
[[spirv::builtin]] uint  imageAtomicExchange(uimage2DMSArray, ivec3 P, int sample, uint compare, uint data) noexcept;
[[spirv::builtin]] uint  imageAtomicExchange(uimage2DRect,    ivec2 P            , uint compare, uint data) noexcept;
[[spirv::builtin]] uint  imageAtomicExchange(uimage3D,        ivec3 P            , uint compare, uint data) noexcept;
[[spirv::builtin]] uint  imageAtomicExchange(uimageCube,      ivec3 P            , uint compare, uint data) noexcept;
[[spirv::builtin]] uint  imageAtomicExchange(uimageCubeArray, ivec3 P            , uint compare, uint data) noexcept;
[[spirv::builtin]] uint  imageAtomicExchange(uimageBuffer,    int   P            , uint compare, uint data) noexcept;

////////////////////////////////////////////////////////////////////////////////
// https://github.com/KhronosGroup/GLSL/blob/master/extensions/nv/GLSL_NV_ray_tracing.txt

// rgen  - ray generation
// rint  - ray intersection
// rahit - ray any hit
// rchit - ray closest hit
// rmiss - ray miss
// rcall - ray call 

// In the ray generation shading language, built-in variables are 
// declared as follows:

// Work dimensions
[[spirv::builtin(5319)]] uvec3  glray_LaunchID;
[[spirv::builtin(5320)]] uvec3  glray_LaunchSize;

// Geometry instance ids
[[spirv::builtin(7)]]    int    glray_PrimitiveID;
[[spirv::builtin(6)]]    int    glray_InstanceID;
[[spirv::builtin(5327)]] int    glray_InstanceCustomIndex;

// World space parameters
[[spirv::builtin(5321)]] vec3   glray_WorldRayOrigin;
[[spirv::builtin(5322)]] vec3   glray_WorldRayDirection;
[[spirv::builtin(5323)]] vec3   glray_ObjectRayOrigin;
[[spirv::builtin(5324)]] vec3   glray_ObjectRayDirection;

// Ray parameters
[[spirv::builtin(5325)]] float  glray_Tmin;
[[spirv::builtin(5326)]] float  glray_Tmax;
[[spirv::builtin(5351)]] uint   glray_IncomingRayFlags;

// Ray hit info
[[spirv::builtin(5332)]] float  glray_HitT;
[[spirv::builtin(5333)]] uint   glray_HitKind;

// Transform matrices
[[spirv::builtin(5330)]] mat4x3 glray_ObjectToWorld;
[[spirv::builtin(5331)]] mat4x3 glray_WorldToObject;

// todo: rename as glray_accelerationStructure?
enum class [[spirv::opcode(5341)]] accelerationStructure : long long;

[[spirv::opcode(5337)]] void glray_Trace(
  accelerationStructure topLevel,
  uint rayFlags,
  uint cullMask,
  uint sbtRecordOffset,
  uint sbtRecordStride,
  uint missIndex,
  vec3 origin,
  float Tmin,
  vec3 direction,
  float Tmax,
  int payload);

// Payload is a pointer to the ray payload structure to use for this trace.
// Payload must be the result of an OpVariable with a storage class of 
// RayPayloadKHR or IncomingRayPayloadKHR.
// [[spirv::opcode(4445)]] void glray_TraceKHR(
//   accelerationStructure topLevel,
//   uint rayFlags,
//   uint cullMask,
//   uint sbtRecordOffset,
//   uint sbtRecordStride,
//   uint missIndex,
//   vec3 origin,
//   float Tmin,
//   vec3 direction,
//   float Tmax,
//   void* payload); 

const uint gl_RayFlagsNone = 0;
const uint gl_RayFlagsOpaque = 1;
const uint gl_RayFlagsNoOpaque = 2;
const uint gl_RayFlagsTerminateOnFirstHit = 4;
const uint gl_RayFlagsSkipClosestHitShader = 8;
const uint gl_RayFlagsCullBackFacingTriangles = 16;
const uint gl_RayFlagsCullFrontFacingTriangles = 32;
const uint gl_RayFlagsCullOpaque = 64;
const uint gl_RayFlagsCullNoOpaque = 128;
const uint gl_RayFlagsSkipTriangles = 256;
const uint gl_RayFlagsSkipAABBs = 512;

[[spirv::opcode(5334)]] bool gl_reportIntersection(float hitT, uint hitKind);
[[spirv::opcode(5335)]] void gl_ignoreIntersection();
[[spirv::opcode(5336)]] void gl_terminateRay();
[[spirv::opcode(5344)]] void gl_executeCallable(uint sbtRecordIndex, int callable);

////////////////////////////////////////////////////////////////////////////////
// https://github.com/KhronosGroup/GLSL/blob/master/extensions/ext/GLSL_EXT_ray_query.txt

enum class [[spirv::opcode(4472)]] gl_rayQuery : long long;

const uint gl_RayQueryCommittedIntersectionNone = 0;
const uint gl_RayQueryCommittedIntersectionTriangle = 1;
const uint gl_RayQueryCommittedIntersectionGenerated = 2;

const uint gl_RayQueryCandidateIntersectionTriangle = 0;
const uint gl_RayQueryCandidateIntersectionAABB = 1;

[[spirv::opcode(4473)]] void gl_rayQueryInitialize(
  gl_rayQuery& q,
  accelerationStructure topLevel,
  uint rayFlags,
  uint cullMask,
  vec3 origin,
  float tMin,
  vec3 direction,
  float tMax);

[[spirv::opcode(4477)]] bool   gl_rayQueryProceed(gl_rayQuery& q);
[[spirv::opcode(4474)]] void   gl_rayQueryTerminate(gl_rayQuery& q);
[[spirv::opcode(4475)]] void   gl_rayQueryGenerateIntersection(gl_rayQuery& q, float tHit);
[[spirv::opcode(4476)]] void   gl_rayQueryConfirmIntersection(gl_rayQuery& q);
[[spirv::opcode(4479)]] uint   gl_rayQueryGetIntersectionType(gl_rayQuery& q, int committed);
[[spirv::opcode(6016)]] float  gl_rayQueryGetRayTMin(gl_rayQuery& q);
[[spirv::opcode(6017)]] uint   gl_rayQueryGetRayFlags(gl_rayQuery& q);
[[spirv::opcode(6030)]] vec3   gl_rayQueryGetWorldRayOrigin(gl_rayQuery& q);
[[spirv::opcode(6029)]] vec3   gl_rayQueryGetWorldRayDirection(gl_rayQuery& q);
[[spirv::opcode(6018)]] float  gl_rayQueryGetIntersectionT(gl_rayQuery& q, int committed);
[[spirv::opcode(6019)]] int    gl_rayQueryGetIntersectionInstanceCustomIndex(gl_rayQuery& q, int committed);
[[spirv::opcode(6020)]] int    gl_rayQueryGetIntersectionInstanceId(gl_rayQuery& q, int committed);
[[spirv::opcode(6021)]] uint   gl_rayQueryGetIntersectionInstanceShaderBindingTableRecordOffset(gl_rayQuery&q, bool committed);
[[spirv::opcode(6022)]] int    gl_rayQueryGetIntersectionGeometryIndex(gl_rayQuery& q, int committed);
[[spirv::opcode(6023)]] int    gl_rayQueryGetIntersectionPrimitiveIndex(gl_rayQuery& q, int committed);
[[spirv::opcode(6024)]] vec2   gl_rayQueryGetIntersectionBarycentrics(gl_rayQuery& q, int committed);
[[spirv::opcode(6025)]] bool   gl_rayQueryGetIntersectionFrontFace(gl_rayQuery& q, int committed);
[[spirv::opcode(6026)]] bool   gl_rayQueryGetIntersectionCandidateAABBOpaque(gl_rayQuery& q);
[[spirv::opcode(6027)]] vec3   gl_rayQueryGetIntersectionObjectRayDirection(gl_rayQuery& q, int committed);
[[spirv::opcode(6028)]] vec3   gl_rayQueryGetIntersectionObjectRayOrigin(gl_rayQuery& q, int committed);
[[spirv::opcode(6031)]] mat4x3 gl_rayQueryGetIntersectionObjectToWorld(gl_rayQuery& q, int committed);
[[spirv::opcode(6032)]] mat4x3 gl_rayQueryGetIntersectionWorldToObject(gl_rayQuery& q, int committed);

////////////////////////////////////////////////////////////////////////////////
// https://github.com/KhronosGroup/GLSL/blob/master/extensions/khr/GL_KHR_shader_subgroup.txt
// GL_KHR_shader_subgroup_ballot

[[spirv::builtin(38)]] uint gl_NumSubgroups;
[[spirv::builtin(40)]] uint gl_SubgroupID;

[[spirv::builtin(36)  ]] uint  gl_SubgroupSize;
[[spirv::builtin(41)  ]] uint  gl_SubgroupInvocationID;
[[spirv::builtin(4416)]] uvec4 gl_SubgroupEqMask;
[[spirv::builtin(4417)]] uvec4 gl_SubgroupGeMask;
[[spirv::builtin(4418)]] uvec4 gl_SubgroupGtMask;
[[spirv::builtin(4419)]] uvec4 gl_SubgroupLeMask;
[[spirv::builtin(4420)]] uvec4 gl_SubgroupLtMask;

// GL_KHR_shader_subgroup_basic
[[spirv::builtin]] void gl_subgroupBarrier() noexcept;
[[spirv::builtin]] void gl_subgroupMemoryBarrier() noexcept;
[[spirv::builtin]] void gl_subgroupMemoryBarrierBuffer() noexcept;
[[spirv::builtin]] void gl_subgroupMemoryBarrierShared() noexcept;
[[spirv::builtin]] void gl_subgroupMemoryBarrierImage() noexcept;
[[spirv::builtin]] bool gl_subgroupElect() noexcept;

// GL_KHR_shader_subgroup_vote
[[spirv::builtin]] bool gl_subgroupAll(bool value) noexcept;
[[spirv::builtin]] bool gl_subgroupAny(bool value) noexcept;
[[spirv::builtin]] bool gl_subgroupAllEqual(float  value) noexcept;
[[spirv::builtin]] bool gl_subgroupAllEqual(vec2   value) noexcept;
[[spirv::builtin]] bool gl_subgroupAllEqual(vec3   value) noexcept;
[[spirv::builtin]] bool gl_subgroupAllEqual(vec4   value) noexcept;
[[spirv::builtin]] bool gl_subgroupAllEqual(int    value) noexcept;
[[spirv::builtin]] bool gl_subgroupAllEqual(ivec2  value) noexcept;
[[spirv::builtin]] bool gl_subgroupAllEqual(ivec3  value) noexcept;
[[spirv::builtin]] bool gl_subgroupAllEqual(ivec4  value) noexcept;
[[spirv::builtin]] bool gl_subgroupAllEqual(uint   value) noexcept;
[[spirv::builtin]] bool gl_subgroupAllEqual(uvec2  value) noexcept;
[[spirv::builtin]] bool gl_subgroupAllEqual(uvec3  value) noexcept;
[[spirv::builtin]] bool gl_subgroupAllEqual(uvec4  value) noexcept;
[[spirv::builtin]] bool gl_subgroupAllEqual(bool   value) noexcept;
[[spirv::builtin]] bool gl_subgroupAllEqual(bvec2  value) noexcept;
[[spirv::builtin]] bool gl_subgroupAllEqual(bvec3  value) noexcept;
[[spirv::builtin]] bool gl_subgroupAllEqual(bvec4  value) noexcept;
[[spirv::builtin]] bool gl_subgroupAllEqual(double value) noexcept;
[[spirv::builtin]] bool gl_subgroupAllEqual(dvec2  value) noexcept;
[[spirv::builtin]] bool gl_subgroupAllEqual(dvec3  value) noexcept;
[[spirv::builtin]] bool gl_subgroupAllEqual(dvec4  value) noexcept;

// GL_KHR_shader_subgroup_ballot
[[spirv::builtin]] float  gl_subgroupBroadcast(float  value, uint id) noexcept;
[[spirv::builtin]] vec2   gl_subgroupBroadcast(vec2   value, uint id) noexcept;
[[spirv::builtin]] vec3   gl_subgroupBroadcast(vec3   value, uint id) noexcept;
[[spirv::builtin]] vec4   gl_subgroupBroadcast(vec4   value, uint id) noexcept;
[[spirv::builtin]] int    gl_subgroupBroadcast(int    value, uint id) noexcept;
[[spirv::builtin]] ivec2  gl_subgroupBroadcast(ivec2  value, uint id) noexcept;
[[spirv::builtin]] ivec3  gl_subgroupBroadcast(ivec3  value, uint id) noexcept;
[[spirv::builtin]] ivec4  gl_subgroupBroadcast(ivec4  value, uint id) noexcept;
[[spirv::builtin]] uint   gl_subgroupBroadcast(uint   value, uint id) noexcept;
[[spirv::builtin]] uvec2  gl_subgroupBroadcast(uvec2  value, uint id) noexcept;
[[spirv::builtin]] uvec3  gl_subgroupBroadcast(uvec3  value, uint id) noexcept;
[[spirv::builtin]] uvec4  gl_subgroupBroadcast(uvec4  value, uint id) noexcept;
[[spirv::builtin]] bool   gl_subgroupBroadcast(bool   value, uint id) noexcept;
[[spirv::builtin]] bvec2  gl_subgroupBroadcast(bvec2  value, uint id) noexcept;
[[spirv::builtin]] bvec3  gl_subgroupBroadcast(bvec3  value, uint id) noexcept;
[[spirv::builtin]] bvec4  gl_subgroupBroadcast(bvec4  value, uint id) noexcept;
[[spirv::builtin]] double gl_subgroupBroadcast(double value, uint id) noexcept;
[[spirv::builtin]] dvec2  gl_subgroupBroadcast(dvec2  value, uint id) noexcept;
[[spirv::builtin]] dvec3  gl_subgroupBroadcast(dvec3  value, uint id) noexcept;
[[spirv::builtin]] dvec4  gl_subgroupBroadcast(dvec4  value, uint id) noexcept;

[[spirv::builtin]] float  gl_subgroupBroadcastFirst(float  value) noexcept;
[[spirv::builtin]] vec2   gl_subgroupBroadcastFirst(vec2   value) noexcept;
[[spirv::builtin]] vec3   gl_subgroupBroadcastFirst(vec3   value) noexcept;
[[spirv::builtin]] vec4   gl_subgroupBroadcastFirst(vec4   value) noexcept;
[[spirv::builtin]] int    gl_subgroupBroadcastFirst(int    value) noexcept;
[[spirv::builtin]] ivec2  gl_subgroupBroadcastFirst(ivec2  value) noexcept;
[[spirv::builtin]] ivec3  gl_subgroupBroadcastFirst(ivec3  value) noexcept;
[[spirv::builtin]] ivec4  gl_subgroupBroadcastFirst(ivec4  value) noexcept;
[[spirv::builtin]] uint   gl_subgroupBroadcastFirst(uint   value) noexcept;
[[spirv::builtin]] uvec2  gl_subgroupBroadcastFirst(uvec2  value) noexcept;
[[spirv::builtin]] uvec3  gl_subgroupBroadcastFirst(uvec3  value) noexcept;
[[spirv::builtin]] uvec4  gl_subgroupBroadcastFirst(uvec4  value) noexcept;
[[spirv::builtin]] bool   gl_subgroupBroadcastFirst(bool   value) noexcept;
[[spirv::builtin]] bvec2  gl_subgroupBroadcastFirst(bvec2  value) noexcept;
[[spirv::builtin]] bvec3  gl_subgroupBroadcastFirst(bvec3  value) noexcept;
[[spirv::builtin]] bvec4  gl_subgroupBroadcastFirst(bvec4  value) noexcept;
[[spirv::builtin]] double gl_subgroupBroadcastFirst(double value) noexcept;
[[spirv::builtin]] dvec2  gl_subgroupBroadcastFirst(dvec2  value) noexcept;
[[spirv::builtin]] dvec3  gl_subgroupBroadcastFirst(dvec3  value) noexcept;
[[spirv::builtin]] dvec4  gl_subgroupBroadcastFirst(dvec4  value) noexcept;

[[spirv::builtin]] uvec4 gl_subgroupBallot(bool value) noexcept;
[[spirv::builtin]] bool  gl_subgroupInverseBallot(uvec4 value) noexcept;
[[spirv::builtin]] bool  gl_subgroupBallotBitExtract(uvec4 value, uint index) noexcept;
[[spirv::builtin]] uint  gl_subgroupBallotBitCount(uvec4 value) noexcept;
[[spirv::builtin]] uint  gl_subgroupBallotInclusiveBitCount(uvec4 value) noexcept;
[[spirv::builtin]] uint  gl_subgroupBallotExclusiveBitCount(uvec4 value) noexcept;
[[spirv::builtin]] uint  gl_subgroupBallotFindLSB(uvec4 value) noexcept;
[[spirv::builtin]] uint  gl_subgroupBallotFindMSB(uvec4 value) noexcept;

// GL_KHR_shader_subgroup_shuffle

[[spirv::builtin]] float  gl_subgroupShuffle(float  value, uint id) noexcept;
[[spirv::builtin]] vec2   gl_subgroupShuffle(vec2   value, uint id) noexcept;
[[spirv::builtin]] vec3   gl_subgroupShuffle(vec3   value, uint id) noexcept;
[[spirv::builtin]] vec4   gl_subgroupShuffle(vec4   value, uint id) noexcept;
[[spirv::builtin]] int    gl_subgroupShuffle(int    value, uint id) noexcept;
[[spirv::builtin]] ivec2  gl_subgroupShuffle(ivec2  value, uint id) noexcept;
[[spirv::builtin]] ivec3  gl_subgroupShuffle(ivec3  value, uint id) noexcept;
[[spirv::builtin]] ivec4  gl_subgroupShuffle(ivec4  value, uint id) noexcept;
[[spirv::builtin]] uint   gl_subgroupShuffle(uint   value, uint id) noexcept;
[[spirv::builtin]] uvec2  gl_subgroupShuffle(uvec2  value, uint id) noexcept;
[[spirv::builtin]] uvec3  gl_subgroupShuffle(uvec3  value, uint id) noexcept;
[[spirv::builtin]] uvec4  gl_subgroupShuffle(uvec4  value, uint id) noexcept;
[[spirv::builtin]] bool   gl_subgroupShuffle(bool   value, uint id) noexcept;
[[spirv::builtin]] bvec2  gl_subgroupShuffle(bvec2  value, uint id) noexcept;
[[spirv::builtin]] bvec3  gl_subgroupShuffle(bvec3  value, uint id) noexcept;
[[spirv::builtin]] bvec4  gl_subgroupShuffle(bvec4  value, uint id) noexcept;
[[spirv::builtin]] double gl_subgroupShuffle(double value, uint id) noexcept;
[[spirv::builtin]] dvec2  gl_subgroupShuffle(dvec2  value, uint id) noexcept;
[[spirv::builtin]] dvec3  gl_subgroupShuffle(dvec3  value, uint id) noexcept;
[[spirv::builtin]] dvec4  gl_subgroupShuffle(dvec4  value, uint id) noexcept;

[[spirv::builtin]] float  gl_subgroupShuffleXor(float  value, uint mask) noexcept;
[[spirv::builtin]] vec2   gl_subgroupShuffleXor(vec2   value, uint mask) noexcept;
[[spirv::builtin]] vec3   gl_subgroupShuffleXor(vec3   value, uint mask) noexcept;
[[spirv::builtin]] vec4   gl_subgroupShuffleXor(vec4   value, uint mask) noexcept;
[[spirv::builtin]] int    gl_subgroupShuffleXor(int    value, uint mask) noexcept;
[[spirv::builtin]] ivec2  gl_subgroupShuffleXor(ivec2  value, uint mask) noexcept;
[[spirv::builtin]] ivec3  gl_subgroupShuffleXor(ivec3  value, uint mask) noexcept;
[[spirv::builtin]] ivec4  gl_subgroupShuffleXor(ivec4  value, uint mask) noexcept;
[[spirv::builtin]] uint   gl_subgroupShuffleXor(uint   value, uint mask) noexcept;
[[spirv::builtin]] uvec2  gl_subgroupShuffleXor(uvec2  value, uint mask) noexcept;
[[spirv::builtin]] uvec3  gl_subgroupShuffleXor(uvec3  value, uint mask) noexcept;
[[spirv::builtin]] uvec4  gl_subgroupShuffleXor(uvec4  value, uint mask) noexcept;
[[spirv::builtin]] bool   gl_subgroupShuffleXor(bool   value, uint mask) noexcept;
[[spirv::builtin]] bvec2  gl_subgroupShuffleXor(bvec2  value, uint mask) noexcept;
[[spirv::builtin]] bvec3  gl_subgroupShuffleXor(bvec3  value, uint mask) noexcept;
[[spirv::builtin]] bvec4  gl_subgroupShuffleXor(bvec4  value, uint mask) noexcept;
[[spirv::builtin]] double gl_subgroupShuffleXor(double value, uint mask) noexcept;
[[spirv::builtin]] dvec2  gl_subgroupShuffleXor(dvec2  value, uint mask) noexcept;
[[spirv::builtin]] dvec3  gl_subgroupShuffleXor(dvec3  value, uint mask) noexcept;
[[spirv::builtin]] dvec4  gl_subgroupShuffleXor(dvec4  value, uint mask) noexcept;

[[spirv::builtin]] float  gl_subgroupShuffleUp(float  value, uint delta) noexcept;
[[spirv::builtin]] vec2   gl_subgroupShuffleUp(vec2   value, uint delta) noexcept;
[[spirv::builtin]] vec3   gl_subgroupShuffleUp(vec3   value, uint delta) noexcept;
[[spirv::builtin]] vec4   gl_subgroupShuffleUp(vec4   value, uint delta) noexcept;
[[spirv::builtin]] int    gl_subgroupShuffleUp(int    value, uint delta) noexcept;
[[spirv::builtin]] ivec2  gl_subgroupShuffleUp(ivec2  value, uint delta) noexcept;
[[spirv::builtin]] ivec3  gl_subgroupShuffleUp(ivec3  value, uint delta) noexcept;
[[spirv::builtin]] ivec4  gl_subgroupShuffleUp(ivec4  value, uint delta) noexcept;
[[spirv::builtin]] uint   gl_subgroupShuffleUp(uint   value, uint delta) noexcept;
[[spirv::builtin]] uvec2  gl_subgroupShuffleUp(uvec2  value, uint delta) noexcept;
[[spirv::builtin]] uvec3  gl_subgroupShuffleUp(uvec3  value, uint delta) noexcept;
[[spirv::builtin]] uvec4  gl_subgroupShuffleUp(uvec4  value, uint delta) noexcept;
[[spirv::builtin]] bool   gl_subgroupShuffleUp(bool   value, uint delta) noexcept;
[[spirv::builtin]] bvec2  gl_subgroupShuffleUp(bvec2  value, uint delta) noexcept;
[[spirv::builtin]] bvec3  gl_subgroupShuffleUp(bvec3  value, uint delta) noexcept;
[[spirv::builtin]] bvec4  gl_subgroupShuffleUp(bvec4  value, uint delta) noexcept;
[[spirv::builtin]] double gl_subgroupShuffleUp(double value, uint delta) noexcept;
[[spirv::builtin]] dvec2  gl_subgroupShuffleUp(dvec2  value, uint delta) noexcept;
[[spirv::builtin]] dvec3  gl_subgroupShuffleUp(dvec3  value, uint delta) noexcept;
[[spirv::builtin]] dvec4  gl_subgroupShuffleUp(dvec4  value, uint delta) noexcept;

[[spirv::builtin]] float  gl_subgroupShuffleDown(float  value, uint delta) noexcept;
[[spirv::builtin]] vec2   gl_subgroupShuffleDown(vec2   value, uint delta) noexcept;
[[spirv::builtin]] vec3   gl_subgroupShuffleDown(vec3   value, uint delta) noexcept;
[[spirv::builtin]] vec4   gl_subgroupShuffleDown(vec4   value, uint delta) noexcept;
[[spirv::builtin]] int    gl_subgroupShuffleDown(int    value, uint delta) noexcept;
[[spirv::builtin]] ivec2  gl_subgroupShuffleDown(ivec2  value, uint delta) noexcept;
[[spirv::builtin]] ivec3  gl_subgroupShuffleDown(ivec3  value, uint delta) noexcept;
[[spirv::builtin]] ivec4  gl_subgroupShuffleDown(ivec4  value, uint delta) noexcept;
[[spirv::builtin]] uint   gl_subgroupShuffleDown(uint   value, uint delta) noexcept;
[[spirv::builtin]] uvec2  gl_subgroupShuffleDown(uvec2  value, uint delta) noexcept;
[[spirv::builtin]] uvec3  gl_subgroupShuffleDown(uvec3  value, uint delta) noexcept;
[[spirv::builtin]] uvec4  gl_subgroupShuffleDown(uvec4  value, uint delta) noexcept;
[[spirv::builtin]] bool   gl_subgroupShuffleDown(bool   value, uint delta) noexcept;
[[spirv::builtin]] bvec2  gl_subgroupShuffleDown(bvec2  value, uint delta) noexcept;
[[spirv::builtin]] bvec3  gl_subgroupShuffleDown(bvec3  value, uint delta) noexcept;
[[spirv::builtin]] bvec4  gl_subgroupShuffleDown(bvec4  value, uint delta) noexcept;
[[spirv::builtin]] double gl_subgroupShuffleDown(double value, uint delta) noexcept;
[[spirv::builtin]] dvec2  gl_subgroupShuffleDown(dvec2  value, uint delta) noexcept;
[[spirv::builtin]] dvec3  gl_subgroupShuffleDown(dvec3  value, uint delta) noexcept;
[[spirv::builtin]] dvec4  gl_subgroupShuffleDown(dvec4  value, uint delta) noexcept;

// GL_KHR_shader_subgroup_arithmetic

[[spirv::builtin]] float  gl_subgroupAdd(float  value) noexcept;
[[spirv::builtin]] vec2   gl_subgroupAdd(vec2   value) noexcept;
[[spirv::builtin]] vec3   gl_subgroupAdd(vec3   value) noexcept;
[[spirv::builtin]] vec4   gl_subgroupAdd(vec4   value) noexcept;
[[spirv::builtin]] int    gl_subgroupAdd(int    value) noexcept;
[[spirv::builtin]] ivec2  gl_subgroupAdd(ivec2  value) noexcept;
[[spirv::builtin]] ivec3  gl_subgroupAdd(ivec3  value) noexcept;
[[spirv::builtin]] ivec4  gl_subgroupAdd(ivec4  value) noexcept;
[[spirv::builtin]] uint   gl_subgroupAdd(uint   value) noexcept;
[[spirv::builtin]] uvec2  gl_subgroupAdd(uvec2  value) noexcept;
[[spirv::builtin]] uvec3  gl_subgroupAdd(uvec3  value) noexcept;
[[spirv::builtin]] uvec4  gl_subgroupAdd(uvec4  value) noexcept;
[[spirv::builtin]] double gl_subgroupAdd(double value) noexcept;
[[spirv::builtin]] dvec2  gl_subgroupAdd(dvec2  value) noexcept;
[[spirv::builtin]] dvec3  gl_subgroupAdd(dvec3  value) noexcept;
[[spirv::builtin]] dvec4  gl_subgroupAdd(dvec4  value) noexcept;

[[spirv::builtin]] float  gl_subgroupMul(float  value) noexcept;
[[spirv::builtin]] vec2   gl_subgroupMul(vec2   value) noexcept;
[[spirv::builtin]] vec3   gl_subgroupMul(vec3   value) noexcept;
[[spirv::builtin]] vec4   gl_subgroupMul(vec4   value) noexcept;
[[spirv::builtin]] int    gl_subgroupMul(int    value) noexcept;
[[spirv::builtin]] ivec2  gl_subgroupMul(ivec2  value) noexcept;
[[spirv::builtin]] ivec3  gl_subgroupMul(ivec3  value) noexcept;
[[spirv::builtin]] ivec4  gl_subgroupMul(ivec4  value) noexcept;
[[spirv::builtin]] uint   gl_subgroupMul(uint   value) noexcept;
[[spirv::builtin]] uvec2  gl_subgroupMul(uvec2  value) noexcept;
[[spirv::builtin]] uvec3  gl_subgroupMul(uvec3  value) noexcept;
[[spirv::builtin]] uvec4  gl_subgroupMul(uvec4  value) noexcept;
[[spirv::builtin]] double gl_subgroupMul(double value) noexcept;
[[spirv::builtin]] dvec2  gl_subgroupMul(dvec2  value) noexcept;
[[spirv::builtin]] dvec3  gl_subgroupMul(dvec3  value) noexcept;
[[spirv::builtin]] dvec4  gl_subgroupMul(dvec4  value) noexcept;

[[spirv::builtin]] float  gl_subgroupMin(float  value) noexcept;
[[spirv::builtin]] vec2   gl_subgroupMin(vec2   value) noexcept;
[[spirv::builtin]] vec3   gl_subgroupMin(vec3   value) noexcept;
[[spirv::builtin]] vec4   gl_subgroupMin(vec4   value) noexcept;
[[spirv::builtin]] int    gl_subgroupMin(int    value) noexcept;
[[spirv::builtin]] ivec2  gl_subgroupMin(ivec2  value) noexcept;
[[spirv::builtin]] ivec3  gl_subgroupMin(ivec3  value) noexcept;
[[spirv::builtin]] ivec4  gl_subgroupMin(ivec4  value) noexcept;
[[spirv::builtin]] uint   gl_subgroupMin(uint   value) noexcept;
[[spirv::builtin]] uvec2  gl_subgroupMin(uvec2  value) noexcept;
[[spirv::builtin]] uvec3  gl_subgroupMin(uvec3  value) noexcept;
[[spirv::builtin]] uvec4  gl_subgroupMin(uvec4  value) noexcept;
[[spirv::builtin]] double gl_subgroupMin(double value) noexcept;
[[spirv::builtin]] dvec2  gl_subgroupMin(dvec2  value) noexcept;
[[spirv::builtin]] dvec3  gl_subgroupMin(dvec3  value) noexcept;
[[spirv::builtin]] dvec4  gl_subgroupMin(dvec4  value) noexcept;

[[spirv::builtin]] float  gl_subgroupMax(float  value) noexcept;
[[spirv::builtin]] vec2   gl_subgroupMax(vec2   value) noexcept;
[[spirv::builtin]] vec3   gl_subgroupMax(vec3   value) noexcept;
[[spirv::builtin]] vec4   gl_subgroupMax(vec4   value) noexcept;
[[spirv::builtin]] int    gl_subgroupMax(int    value) noexcept;
[[spirv::builtin]] ivec2  gl_subgroupMax(ivec2  value) noexcept;
[[spirv::builtin]] ivec3  gl_subgroupMax(ivec3  value) noexcept;
[[spirv::builtin]] ivec4  gl_subgroupMax(ivec4  value) noexcept;
[[spirv::builtin]] uint   gl_subgroupMax(uint   value) noexcept;
[[spirv::builtin]] uvec2  gl_subgroupMax(uvec2  value) noexcept;
[[spirv::builtin]] uvec3  gl_subgroupMax(uvec3  value) noexcept;
[[spirv::builtin]] uvec4  gl_subgroupMax(uvec4  value) noexcept;
[[spirv::builtin]] double gl_subgroupMax(double value) noexcept;
[[spirv::builtin]] dvec2  gl_subgroupMax(dvec2  value) noexcept;
[[spirv::builtin]] dvec3  gl_subgroupMax(dvec3  value) noexcept;
[[spirv::builtin]] dvec4  gl_subgroupMax(dvec4  value) noexcept;

[[spirv::builtin]] int    gl_subgroupAnd(int    value) noexcept;
[[spirv::builtin]] ivec2  gl_subgroupAnd(ivec2  value) noexcept;
[[spirv::builtin]] ivec3  gl_subgroupAnd(ivec3  value) noexcept;
[[spirv::builtin]] ivec4  gl_subgroupAnd(ivec4  value) noexcept;
[[spirv::builtin]] uint   gl_subgroupAnd(uint   value) noexcept;
[[spirv::builtin]] uvec2  gl_subgroupAnd(uvec2  value) noexcept;
[[spirv::builtin]] uvec3  gl_subgroupAnd(uvec3  value) noexcept;
[[spirv::builtin]] uvec4  gl_subgroupAnd(uvec4  value) noexcept;
[[spirv::builtin]] bool   gl_subgroupAnd(bool   value) noexcept;
[[spirv::builtin]] bvec2  gl_subgroupAnd(bvec2  value) noexcept;
[[spirv::builtin]] bvec3  gl_subgroupAnd(bvec3  value) noexcept;
[[spirv::builtin]] bvec4  gl_subgroupAnd(bvec4  value) noexcept;

[[spirv::builtin]] int    gl_subgroupOr(int    value) noexcept;
[[spirv::builtin]] ivec2  gl_subgroupOr(ivec2  value) noexcept;
[[spirv::builtin]] ivec3  gl_subgroupOr(ivec3  value) noexcept;
[[spirv::builtin]] ivec4  gl_subgroupOr(ivec4  value) noexcept;
[[spirv::builtin]] uint   gl_subgroupOr(uint   value) noexcept;
[[spirv::builtin]] uvec2  gl_subgroupOr(uvec2  value) noexcept;
[[spirv::builtin]] uvec3  gl_subgroupOr(uvec3  value) noexcept;
[[spirv::builtin]] uvec4  gl_subgroupOr(uvec4  value) noexcept;
[[spirv::builtin]] bool   gl_subgroupOr(bool   value) noexcept;
[[spirv::builtin]] bvec2  gl_subgroupOr(bvec2  value) noexcept;
[[spirv::builtin]] bvec3  gl_subgroupOr(bvec3  value) noexcept;
[[spirv::builtin]] bvec4  gl_subgroupOr(bvec4  value) noexcept;

[[spirv::builtin]] int    gl_subgroupXor(int    value) noexcept;
[[spirv::builtin]] ivec2  gl_subgroupXor(ivec2  value) noexcept;
[[spirv::builtin]] ivec3  gl_subgroupXor(ivec3  value) noexcept;
[[spirv::builtin]] ivec4  gl_subgroupXor(ivec4  value) noexcept;
[[spirv::builtin]] uint   gl_subgroupXor(uint   value) noexcept;
[[spirv::builtin]] uvec2  gl_subgroupXor(uvec2  value) noexcept;
[[spirv::builtin]] uvec3  gl_subgroupXor(uvec3  value) noexcept;
[[spirv::builtin]] uvec4  gl_subgroupXor(uvec4  value) noexcept;
[[spirv::builtin]] bool   gl_subgroupXor(bool   value) noexcept;
[[spirv::builtin]] bvec2  gl_subgroupXor(bvec2  value) noexcept;
[[spirv::builtin]] bvec3  gl_subgroupXor(bvec3  value) noexcept;
[[spirv::builtin]] bvec4  gl_subgroupXor(bvec4  value) noexcept;

[[spirv::builtin]] float  gl_subgroupInclusiveAdd(float  value) noexcept;
[[spirv::builtin]] vec2   gl_subgroupInclusiveAdd(vec2   value) noexcept;
[[spirv::builtin]] vec3   gl_subgroupInclusiveAdd(vec3   value) noexcept;
[[spirv::builtin]] vec4   gl_subgroupInclusiveAdd(vec4   value) noexcept;
[[spirv::builtin]] int    gl_subgroupInclusiveAdd(int    value) noexcept;
[[spirv::builtin]] ivec2  gl_subgroupInclusiveAdd(ivec2  value) noexcept;
[[spirv::builtin]] ivec3  gl_subgroupInclusiveAdd(ivec3  value) noexcept;
[[spirv::builtin]] ivec4  gl_subgroupInclusiveAdd(ivec4  value) noexcept;
[[spirv::builtin]] uint   gl_subgroupInclusiveAdd(uint   value) noexcept;
[[spirv::builtin]] uvec2  gl_subgroupInclusiveAdd(uvec2  value) noexcept;
[[spirv::builtin]] uvec3  gl_subgroupInclusiveAdd(uvec3  value) noexcept;
[[spirv::builtin]] uvec4  gl_subgroupInclusiveAdd(uvec4  value) noexcept;
[[spirv::builtin]] double gl_subgroupInclusiveAdd(double value) noexcept;
[[spirv::builtin]] dvec2  gl_subgroupInclusiveAdd(dvec2  value) noexcept;
[[spirv::builtin]] dvec3  gl_subgroupInclusiveAdd(dvec3  value) noexcept;
[[spirv::builtin]] dvec4  gl_subgroupInclusiveAdd(dvec4  value) noexcept;

[[spirv::builtin]] float  gl_subgroupInclusiveMul(float  value) noexcept;
[[spirv::builtin]] vec2   gl_subgroupInclusiveMul(vec2   value) noexcept;
[[spirv::builtin]] vec3   gl_subgroupInclusiveMul(vec3   value) noexcept;
[[spirv::builtin]] vec4   gl_subgroupInclusiveMul(vec4   value) noexcept;
[[spirv::builtin]] int    gl_subgroupInclusiveMul(int    value) noexcept;
[[spirv::builtin]] ivec2  gl_subgroupInclusiveMul(ivec2  value) noexcept;
[[spirv::builtin]] ivec3  gl_subgroupInclusiveMul(ivec3  value) noexcept;
[[spirv::builtin]] ivec4  gl_subgroupInclusiveMul(ivec4  value) noexcept;
[[spirv::builtin]] uint   gl_subgroupInclusiveMul(uint   value) noexcept;
[[spirv::builtin]] uvec2  gl_subgroupInclusiveMul(uvec2  value) noexcept;
[[spirv::builtin]] uvec3  gl_subgroupInclusiveMul(uvec3  value) noexcept;
[[spirv::builtin]] uvec4  gl_subgroupInclusiveMul(uvec4  value) noexcept;
[[spirv::builtin]] double gl_subgroupInclusiveMul(double value) noexcept;
[[spirv::builtin]] dvec2  gl_subgroupInclusiveMul(dvec2  value) noexcept;
[[spirv::builtin]] dvec3  gl_subgroupInclusiveMul(dvec3  value) noexcept;
[[spirv::builtin]] dvec4  gl_subgroupInclusiveMul(dvec4  value) noexcept;

[[spirv::builtin]] float  gl_subgroupInclusiveMin(float  value) noexcept;
[[spirv::builtin]] vec2   gl_subgroupInclusiveMin(vec2   value) noexcept;
[[spirv::builtin]] vec3   gl_subgroupInclusiveMin(vec3   value) noexcept;
[[spirv::builtin]] vec4   gl_subgroupInclusiveMin(vec4   value) noexcept;
[[spirv::builtin]] int    gl_subgroupInclusiveMin(int    value) noexcept;
[[spirv::builtin]] ivec2  gl_subgroupInclusiveMin(ivec2  value) noexcept;
[[spirv::builtin]] ivec3  gl_subgroupInclusiveMin(ivec3  value) noexcept;
[[spirv::builtin]] ivec4  gl_subgroupInclusiveMin(ivec4  value) noexcept;
[[spirv::builtin]] uint   gl_subgroupInclusiveMin(uint   value) noexcept;
[[spirv::builtin]] uvec2  gl_subgroupInclusiveMin(uvec2  value) noexcept;
[[spirv::builtin]] uvec3  gl_subgroupInclusiveMin(uvec3  value) noexcept;
[[spirv::builtin]] uvec4  gl_subgroupInclusiveMin(uvec4  value) noexcept;
[[spirv::builtin]] double gl_subgroupInclusiveMin(double value) noexcept;
[[spirv::builtin]] dvec2  gl_subgroupInclusiveMin(dvec2  value) noexcept;
[[spirv::builtin]] dvec3  gl_subgroupInclusiveMin(dvec3  value) noexcept;
[[spirv::builtin]] dvec4  gl_subgroupInclusiveMin(dvec4  value) noexcept;

[[spirv::builtin]] float  gl_subgroupInclusiveMax(float  value) noexcept;
[[spirv::builtin]] vec2   gl_subgroupInclusiveMax(vec2   value) noexcept;
[[spirv::builtin]] vec3   gl_subgroupInclusiveMax(vec3   value) noexcept;
[[spirv::builtin]] vec4   gl_subgroupInclusiveMax(vec4   value) noexcept;
[[spirv::builtin]] int    gl_subgroupInclusiveMax(int    value) noexcept;
[[spirv::builtin]] ivec2  gl_subgroupInclusiveMax(ivec2  value) noexcept;
[[spirv::builtin]] ivec3  gl_subgroupInclusiveMax(ivec3  value) noexcept;
[[spirv::builtin]] ivec4  gl_subgroupInclusiveMax(ivec4  value) noexcept;
[[spirv::builtin]] uint   gl_subgroupInclusiveMax(uint   value) noexcept;
[[spirv::builtin]] uvec2  gl_subgroupInclusiveMax(uvec2  value) noexcept;
[[spirv::builtin]] uvec3  gl_subgroupInclusiveMax(uvec3  value) noexcept;
[[spirv::builtin]] uvec4  gl_subgroupInclusiveMax(uvec4  value) noexcept;
[[spirv::builtin]] double gl_subgroupInclusiveMax(double value) noexcept;
[[spirv::builtin]] dvec2  gl_subgroupInclusiveMax(dvec2  value) noexcept;
[[spirv::builtin]] dvec3  gl_subgroupInclusiveMax(dvec3  value) noexcept;
[[spirv::builtin]] dvec4  gl_subgroupInclusiveMax(dvec4  value) noexcept;

[[spirv::builtin]] int    gl_subgroupInclusiveAnd(int    value) noexcept;
[[spirv::builtin]] ivec2  gl_subgroupInclusiveAnd(ivec2  value) noexcept;
[[spirv::builtin]] ivec3  gl_subgroupInclusiveAnd(ivec3  value) noexcept;
[[spirv::builtin]] ivec4  gl_subgroupInclusiveAnd(ivec4  value) noexcept;
[[spirv::builtin]] uint   gl_subgroupInclusiveAnd(uint   value) noexcept;
[[spirv::builtin]] uvec2  gl_subgroupInclusiveAnd(uvec2  value) noexcept;
[[spirv::builtin]] uvec3  gl_subgroupInclusiveAnd(uvec3  value) noexcept;
[[spirv::builtin]] uvec4  gl_subgroupInclusiveAnd(uvec4  value) noexcept;
[[spirv::builtin]] bool   gl_subgroupInclusiveAnd(bool   value) noexcept;
[[spirv::builtin]] bvec2  gl_subgroupInclusiveAnd(bvec2  value) noexcept;
[[spirv::builtin]] bvec3  gl_subgroupInclusiveAnd(bvec3  value) noexcept;
[[spirv::builtin]] bvec4  gl_subgroupInclusiveAnd(bvec4  value) noexcept;

[[spirv::builtin]] int    gl_subgroupInclusiveOr(int    value) noexcept;
[[spirv::builtin]] ivec2  gl_subgroupInclusiveOr(ivec2  value) noexcept;
[[spirv::builtin]] ivec3  gl_subgroupInclusiveOr(ivec3  value) noexcept;
[[spirv::builtin]] ivec4  gl_subgroupInclusiveOr(ivec4  value) noexcept;
[[spirv::builtin]] uint   gl_subgroupInclusiveOr(uint   value) noexcept;
[[spirv::builtin]] uvec2  gl_subgroupInclusiveOr(uvec2  value) noexcept;
[[spirv::builtin]] uvec3  gl_subgroupInclusiveOr(uvec3  value) noexcept;
[[spirv::builtin]] uvec4  gl_subgroupInclusiveOr(uvec4  value) noexcept;
[[spirv::builtin]] bool   gl_subgroupInclusiveOr(bool   value) noexcept;
[[spirv::builtin]] bvec2  gl_subgroupInclusiveOr(bvec2  value) noexcept;
[[spirv::builtin]] bvec3  gl_subgroupInclusiveOr(bvec3  value) noexcept;
[[spirv::builtin]] bvec4  gl_subgroupInclusiveOr(bvec4  value) noexcept;

[[spirv::builtin]] int    gl_subgroupInclusiveXor(int    value) noexcept;
[[spirv::builtin]] ivec2  gl_subgroupInclusiveXor(ivec2  value) noexcept;
[[spirv::builtin]] ivec3  gl_subgroupInclusiveXor(ivec3  value) noexcept;
[[spirv::builtin]] ivec4  gl_subgroupInclusiveXor(ivec4  value) noexcept;
[[spirv::builtin]] uint   gl_subgroupInclusiveXor(uint   value) noexcept;
[[spirv::builtin]] uvec2  gl_subgroupInclusiveXor(uvec2  value) noexcept;
[[spirv::builtin]] uvec3  gl_subgroupInclusiveXor(uvec3  value) noexcept;
[[spirv::builtin]] uvec4  gl_subgroupInclusiveXor(uvec4  value) noexcept;
[[spirv::builtin]] bool   gl_subgroupInclusiveXor(bool   value) noexcept;
[[spirv::builtin]] bvec2  gl_subgroupInclusiveXor(bvec2  value) noexcept;
[[spirv::builtin]] bvec3  gl_subgroupInclusiveXor(bvec3  value) noexcept;
[[spirv::builtin]] bvec4  gl_subgroupInclusiveXor(bvec4  value) noexcept;

[[spirv::builtin]] float  gl_subgroupExclusiveAdd(float  value) noexcept;
[[spirv::builtin]] vec2   gl_subgroupExclusiveAdd(vec2   value) noexcept;
[[spirv::builtin]] vec3   gl_subgroupExclusiveAdd(vec3   value) noexcept;
[[spirv::builtin]] vec4   gl_subgroupExclusiveAdd(vec4   value) noexcept;
[[spirv::builtin]] int    gl_subgroupExclusiveAdd(int    value) noexcept;
[[spirv::builtin]] ivec2  gl_subgroupExclusiveAdd(ivec2  value) noexcept;
[[spirv::builtin]] ivec3  gl_subgroupExclusiveAdd(ivec3  value) noexcept;
[[spirv::builtin]] ivec4  gl_subgroupExclusiveAdd(ivec4  value) noexcept;
[[spirv::builtin]] uint   gl_subgroupExclusiveAdd(uint   value) noexcept;
[[spirv::builtin]] uvec2  gl_subgroupExclusiveAdd(uvec2  value) noexcept;
[[spirv::builtin]] uvec3  gl_subgroupExclusiveAdd(uvec3  value) noexcept;
[[spirv::builtin]] uvec4  gl_subgroupExclusiveAdd(uvec4  value) noexcept;
[[spirv::builtin]] double gl_subgroupExclusiveAdd(double value) noexcept;
[[spirv::builtin]] dvec2  gl_subgroupExclusiveAdd(dvec2  value) noexcept;
[[spirv::builtin]] dvec3  gl_subgroupExclusiveAdd(dvec3  value) noexcept;
[[spirv::builtin]] dvec4  gl_subgroupExclusiveAdd(dvec4  value) noexcept;

[[spirv::builtin]] float  gl_subgroupExclusiveMul(float  value) noexcept;
[[spirv::builtin]] vec2   gl_subgroupExclusiveMul(vec2   value) noexcept;
[[spirv::builtin]] vec3   gl_subgroupExclusiveMul(vec3   value) noexcept;
[[spirv::builtin]] vec4   gl_subgroupExclusiveMul(vec4   value) noexcept;
[[spirv::builtin]] int    gl_subgroupExclusiveMul(int    value) noexcept;
[[spirv::builtin]] ivec2  gl_subgroupExclusiveMul(ivec2  value) noexcept;
[[spirv::builtin]] ivec3  gl_subgroupExclusiveMul(ivec3  value) noexcept;
[[spirv::builtin]] ivec4  gl_subgroupExclusiveMul(ivec4  value) noexcept;
[[spirv::builtin]] uint   gl_subgroupExclusiveMul(uint   value) noexcept;
[[spirv::builtin]] uvec2  gl_subgroupExclusiveMul(uvec2  value) noexcept;
[[spirv::builtin]] uvec3  gl_subgroupExclusiveMul(uvec3  value) noexcept;
[[spirv::builtin]] uvec4  gl_subgroupExclusiveMul(uvec4  value) noexcept;
[[spirv::builtin]] double gl_subgroupExclusiveMul(double value) noexcept;
[[spirv::builtin]] dvec2  gl_subgroupExclusiveMul(dvec2  value) noexcept;
[[spirv::builtin]] dvec3  gl_subgroupExclusiveMul(dvec3  value) noexcept;
[[spirv::builtin]] dvec4  gl_subgroupExclusiveMul(dvec4  value) noexcept;

[[spirv::builtin]] float  gl_subgroupExclusiveMin(float  value) noexcept;
[[spirv::builtin]] vec2   gl_subgroupExclusiveMin(vec2   value) noexcept;
[[spirv::builtin]] vec3   gl_subgroupExclusiveMin(vec3   value) noexcept;
[[spirv::builtin]] vec4   gl_subgroupExclusiveMin(vec4   value) noexcept;
[[spirv::builtin]] int    gl_subgroupExclusiveMin(int    value) noexcept;
[[spirv::builtin]] ivec2  gl_subgroupExclusiveMin(ivec2  value) noexcept;
[[spirv::builtin]] ivec3  gl_subgroupExclusiveMin(ivec3  value) noexcept;
[[spirv::builtin]] ivec4  gl_subgroupExclusiveMin(ivec4  value) noexcept;
[[spirv::builtin]] uint   gl_subgroupExclusiveMin(uint   value) noexcept;
[[spirv::builtin]] uvec2  gl_subgroupExclusiveMin(uvec2  value) noexcept;
[[spirv::builtin]] uvec3  gl_subgroupExclusiveMin(uvec3  value) noexcept;
[[spirv::builtin]] uvec4  gl_subgroupExclusiveMin(uvec4  value) noexcept;
[[spirv::builtin]] double gl_subgroupExclusiveMin(double value) noexcept;
[[spirv::builtin]] dvec2  gl_subgroupExclusiveMin(dvec2  value) noexcept;
[[spirv::builtin]] dvec3  gl_subgroupExclusiveMin(dvec3  value) noexcept;
[[spirv::builtin]] dvec4  gl_subgroupExclusiveMin(dvec4  value) noexcept;

[[spirv::builtin]] float  gl_subgroupExclusiveMax(float  value) noexcept;
[[spirv::builtin]] vec2   gl_subgroupExclusiveMax(vec2   value) noexcept;
[[spirv::builtin]] vec3   gl_subgroupExclusiveMax(vec3   value) noexcept;
[[spirv::builtin]] vec4   gl_subgroupExclusiveMax(vec4   value) noexcept;
[[spirv::builtin]] int    gl_subgroupExclusiveMax(int    value) noexcept;
[[spirv::builtin]] ivec2  gl_subgroupExclusiveMax(ivec2  value) noexcept;
[[spirv::builtin]] ivec3  gl_subgroupExclusiveMax(ivec3  value) noexcept;
[[spirv::builtin]] ivec4  gl_subgroupExclusiveMax(ivec4  value) noexcept;
[[spirv::builtin]] uint   gl_subgroupExclusiveMax(uint   value) noexcept;
[[spirv::builtin]] uvec2  gl_subgroupExclusiveMax(uvec2  value) noexcept;
[[spirv::builtin]] uvec3  gl_subgroupExclusiveMax(uvec3  value) noexcept;
[[spirv::builtin]] uvec4  gl_subgroupExclusiveMax(uvec4  value) noexcept;
[[spirv::builtin]] double gl_subgroupExclusiveMax(double value) noexcept;
[[spirv::builtin]] dvec2  gl_subgroupExclusiveMax(dvec2  value) noexcept;
[[spirv::builtin]] dvec3  gl_subgroupExclusiveMax(dvec3  value) noexcept;
[[spirv::builtin]] dvec4  gl_subgroupExclusiveMax(dvec4  value) noexcept;

[[spirv::builtin]] int    gl_subgroupExclusiveAnd(int    value) noexcept;
[[spirv::builtin]] ivec2  gl_subgroupExclusiveAnd(ivec2  value) noexcept;
[[spirv::builtin]] ivec3  gl_subgroupExclusiveAnd(ivec3  value) noexcept;
[[spirv::builtin]] ivec4  gl_subgroupExclusiveAnd(ivec4  value) noexcept;
[[spirv::builtin]] uint   gl_subgroupExclusiveAnd(uint   value) noexcept;
[[spirv::builtin]] uvec2  gl_subgroupExclusiveAnd(uvec2  value) noexcept;
[[spirv::builtin]] uvec3  gl_subgroupExclusiveAnd(uvec3  value) noexcept;
[[spirv::builtin]] uvec4  gl_subgroupExclusiveAnd(uvec4  value) noexcept;
[[spirv::builtin]] bool   gl_subgroupExclusiveAnd(bool   value) noexcept;
[[spirv::builtin]] bvec2  gl_subgroupExclusiveAnd(bvec2  value) noexcept;
[[spirv::builtin]] bvec3  gl_subgroupExclusiveAnd(bvec3  value) noexcept;
[[spirv::builtin]] bvec4  gl_subgroupExclusiveAnd(bvec4  value) noexcept;

[[spirv::builtin]] int    gl_subgroupExclusiveOr(int    value) noexcept;
[[spirv::builtin]] ivec2  gl_subgroupExclusiveOr(ivec2  value) noexcept;
[[spirv::builtin]] ivec3  gl_subgroupExclusiveOr(ivec3  value) noexcept;
[[spirv::builtin]] ivec4  gl_subgroupExclusiveOr(ivec4  value) noexcept;
[[spirv::builtin]] uint   gl_subgroupExclusiveOr(uint   value) noexcept;
[[spirv::builtin]] uvec2  gl_subgroupExclusiveOr(uvec2  value) noexcept;
[[spirv::builtin]] uvec3  gl_subgroupExclusiveOr(uvec3  value) noexcept;
[[spirv::builtin]] uvec4  gl_subgroupExclusiveOr(uvec4  value) noexcept;
[[spirv::builtin]] bool   gl_subgroupExclusiveOr(bool   value) noexcept;
[[spirv::builtin]] bvec2  gl_subgroupExclusiveOr(bvec2  value) noexcept;
[[spirv::builtin]] bvec3  gl_subgroupExclusiveOr(bvec3  value) noexcept;
[[spirv::builtin]] bvec4  gl_subgroupExclusiveOr(bvec4  value) noexcept;

[[spirv::builtin]] int    gl_subgroupExclusiveXor(int    value) noexcept;
[[spirv::builtin]] ivec2  gl_subgroupExclusiveXor(ivec2  value) noexcept;
[[spirv::builtin]] ivec3  gl_subgroupExclusiveXor(ivec3  value) noexcept;
[[spirv::builtin]] ivec4  gl_subgroupExclusiveXor(ivec4  value) noexcept;
[[spirv::builtin]] uint   gl_subgroupExclusiveXor(uint   value) noexcept;
[[spirv::builtin]] uvec2  gl_subgroupExclusiveXor(uvec2  value) noexcept;
[[spirv::builtin]] uvec3  gl_subgroupExclusiveXor(uvec3  value) noexcept;
[[spirv::builtin]] uvec4  gl_subgroupExclusiveXor(uvec4  value) noexcept;
[[spirv::builtin]] bool   gl_subgroupExclusiveXor(bool   value) noexcept;
[[spirv::builtin]] bvec2  gl_subgroupExclusiveXor(bvec2  value) noexcept;
[[spirv::builtin]] bvec3  gl_subgroupExclusiveXor(bvec3  value) noexcept;
[[spirv::builtin]] bvec4  gl_subgroupExclusiveXor(bvec4  value) noexcept;

// GL_KHR_shader_subgroup_clustered

[[spirv::builtin]] float  gl_subgroupClusteredAdd(float  value, uint clusterSize) noexcept;
[[spirv::builtin]] vec2   gl_subgroupClusteredAdd(vec2   value, uint clusterSize) noexcept;
[[spirv::builtin]] vec3   gl_subgroupClusteredAdd(vec3   value, uint clusterSize) noexcept;
[[spirv::builtin]] vec4   gl_subgroupClusteredAdd(vec4   value, uint clusterSize) noexcept;
[[spirv::builtin]] int    gl_subgroupClusteredAdd(int    value, uint clusterSize) noexcept;
[[spirv::builtin]] ivec2  gl_subgroupClusteredAdd(ivec2  value, uint clusterSize) noexcept;
[[spirv::builtin]] ivec3  gl_subgroupClusteredAdd(ivec3  value, uint clusterSize) noexcept;
[[spirv::builtin]] ivec4  gl_subgroupClusteredAdd(ivec4  value, uint clusterSize) noexcept;
[[spirv::builtin]] uint   gl_subgroupClusteredAdd(uint   value, uint clusterSize) noexcept;
[[spirv::builtin]] uvec2  gl_subgroupClusteredAdd(uvec2  value, uint clusterSize) noexcept;
[[spirv::builtin]] uvec3  gl_subgroupClusteredAdd(uvec3  value, uint clusterSize) noexcept;
[[spirv::builtin]] uvec4  gl_subgroupClusteredAdd(uvec4  value, uint clusterSize) noexcept;
[[spirv::builtin]] double gl_subgroupClusteredAdd(double value, uint clusterSize) noexcept;
[[spirv::builtin]] dvec2  gl_subgroupClusteredAdd(dvec2  value, uint clusterSize) noexcept;
[[spirv::builtin]] dvec3  gl_subgroupClusteredAdd(dvec3  value, uint clusterSize) noexcept;
[[spirv::builtin]] dvec4  gl_subgroupClusteredAdd(dvec4  value, uint clusterSize) noexcept;

[[spirv::builtin]] float  gl_subgroupClusteredMul(float  value, uint clusterSize) noexcept;
[[spirv::builtin]] vec2   gl_subgroupClusteredMul(vec2   value, uint clusterSize) noexcept;
[[spirv::builtin]] vec3   gl_subgroupClusteredMul(vec3   value, uint clusterSize) noexcept;
[[spirv::builtin]] vec4   gl_subgroupClusteredMul(vec4   value, uint clusterSize) noexcept;
[[spirv::builtin]] int    gl_subgroupClusteredMul(int    value, uint clusterSize) noexcept;
[[spirv::builtin]] ivec2  gl_subgroupClusteredMul(ivec2  value, uint clusterSize) noexcept;
[[spirv::builtin]] ivec3  gl_subgroupClusteredMul(ivec3  value, uint clusterSize) noexcept;
[[spirv::builtin]] ivec4  gl_subgroupClusteredMul(ivec4  value, uint clusterSize) noexcept;
[[spirv::builtin]] uint   gl_subgroupClusteredMul(uint   value, uint clusterSize) noexcept;
[[spirv::builtin]] uvec2  gl_subgroupClusteredMul(uvec2  value, uint clusterSize) noexcept;
[[spirv::builtin]] uvec3  gl_subgroupClusteredMul(uvec3  value, uint clusterSize) noexcept;
[[spirv::builtin]] uvec4  gl_subgroupClusteredMul(uvec4  value, uint clusterSize) noexcept;
[[spirv::builtin]] double gl_subgroupClusteredMul(double value, uint clusterSize) noexcept;
[[spirv::builtin]] dvec2  gl_subgroupClusteredMul(dvec2  value, uint clusterSize) noexcept;
[[spirv::builtin]] dvec3  gl_subgroupClusteredMul(dvec3  value, uint clusterSize) noexcept;
[[spirv::builtin]] dvec4  gl_subgroupClusteredMul(dvec4  value, uint clusterSize) noexcept;

[[spirv::builtin]] float  gl_subgroupClusteredMin(float  value, uint clusterSize) noexcept;
[[spirv::builtin]] vec2   gl_subgroupClusteredMin(vec2   value, uint clusterSize) noexcept;
[[spirv::builtin]] vec3   gl_subgroupClusteredMin(vec3   value, uint clusterSize) noexcept;
[[spirv::builtin]] vec4   gl_subgroupClusteredMin(vec4   value, uint clusterSize) noexcept;
[[spirv::builtin]] int    gl_subgroupClusteredMin(int    value, uint clusterSize) noexcept;
[[spirv::builtin]] ivec2  gl_subgroupClusteredMin(ivec2  value, uint clusterSize) noexcept;
[[spirv::builtin]] ivec3  gl_subgroupClusteredMin(ivec3  value, uint clusterSize) noexcept;
[[spirv::builtin]] ivec4  gl_subgroupClusteredMin(ivec4  value, uint clusterSize) noexcept;
[[spirv::builtin]] uint   gl_subgroupClusteredMin(uint   value, uint clusterSize) noexcept;
[[spirv::builtin]] uvec2  gl_subgroupClusteredMin(uvec2  value, uint clusterSize) noexcept;
[[spirv::builtin]] uvec3  gl_subgroupClusteredMin(uvec3  value, uint clusterSize) noexcept;
[[spirv::builtin]] uvec4  gl_subgroupClusteredMin(uvec4  value, uint clusterSize) noexcept;
[[spirv::builtin]] double gl_subgroupClusteredMin(double value, uint clusterSize) noexcept;
[[spirv::builtin]] dvec2  gl_subgroupClusteredMin(dvec2  value, uint clusterSize) noexcept;
[[spirv::builtin]] dvec3  gl_subgroupClusteredMin(dvec3  value, uint clusterSize) noexcept;
[[spirv::builtin]] dvec4  gl_subgroupClusteredMin(dvec4  value, uint clusterSize) noexcept;

[[spirv::builtin]] float  gl_subgroupClusteredMax(float  value, uint clusterSize) noexcept;
[[spirv::builtin]] vec2   gl_subgroupClusteredMax(vec2   value, uint clusterSize) noexcept;
[[spirv::builtin]] vec3   gl_subgroupClusteredMax(vec3   value, uint clusterSize) noexcept;
[[spirv::builtin]] vec4   gl_subgroupClusteredMax(vec4   value, uint clusterSize) noexcept;
[[spirv::builtin]] int    gl_subgroupClusteredMax(int    value, uint clusterSize) noexcept;
[[spirv::builtin]] ivec2  gl_subgroupClusteredMax(ivec2  value, uint clusterSize) noexcept;
[[spirv::builtin]] ivec3  gl_subgroupClusteredMax(ivec3  value, uint clusterSize) noexcept;
[[spirv::builtin]] ivec4  gl_subgroupClusteredMax(ivec4  value, uint clusterSize) noexcept;
[[spirv::builtin]] uint   gl_subgroupClusteredMax(uint   value, uint clusterSize) noexcept;
[[spirv::builtin]] uvec2  gl_subgroupClusteredMax(uvec2  value, uint clusterSize) noexcept;
[[spirv::builtin]] uvec3  gl_subgroupClusteredMax(uvec3  value, uint clusterSize) noexcept;
[[spirv::builtin]] uvec4  gl_subgroupClusteredMax(uvec4  value, uint clusterSize) noexcept;
[[spirv::builtin]] double gl_subgroupClusteredMax(double value, uint clusterSize) noexcept;
[[spirv::builtin]] dvec2  gl_subgroupClusteredMax(dvec2  value, uint clusterSize) noexcept;
[[spirv::builtin]] dvec3  gl_subgroupClusteredMax(dvec3  value, uint clusterSize) noexcept;
[[spirv::builtin]] dvec4  gl_subgroupClusteredMax(dvec4  value, uint clusterSize) noexcept;

[[spirv::builtin]] int    gl_subgroupClusteredAnd(int    value, uint clusterSize) noexcept;
[[spirv::builtin]] ivec2  gl_subgroupClusteredAnd(ivec2  value, uint clusterSize) noexcept;
[[spirv::builtin]] ivec3  gl_subgroupClusteredAnd(ivec3  value, uint clusterSize) noexcept;
[[spirv::builtin]] ivec4  gl_subgroupClusteredAnd(ivec4  value, uint clusterSize) noexcept;
[[spirv::builtin]] uint   gl_subgroupClusteredAnd(uint   value, uint clusterSize) noexcept;
[[spirv::builtin]] uvec2  gl_subgroupClusteredAnd(uvec2  value, uint clusterSize) noexcept;
[[spirv::builtin]] uvec3  gl_subgroupClusteredAnd(uvec3  value, uint clusterSize) noexcept;
[[spirv::builtin]] uvec4  gl_subgroupClusteredAnd(uvec4  value, uint clusterSize) noexcept;
[[spirv::builtin]] bool   gl_subgroupClusteredAnd(bool   value, uint clusterSize) noexcept;
[[spirv::builtin]] bvec2  gl_subgroupClusteredAnd(bvec2  value, uint clusterSize) noexcept;
[[spirv::builtin]] bvec3  gl_subgroupClusteredAnd(bvec3  value, uint clusterSize) noexcept;
[[spirv::builtin]] bvec4  gl_subgroupClusteredAnd(bvec4  value, uint clusterSize) noexcept;

[[spirv::builtin]] int    gl_subgroupClusteredOr(int    value, uint clusterSize) noexcept;
[[spirv::builtin]] ivec2  gl_subgroupClusteredOr(ivec2  value, uint clusterSize) noexcept;
[[spirv::builtin]] ivec3  gl_subgroupClusteredOr(ivec3  value, uint clusterSize) noexcept;
[[spirv::builtin]] ivec4  gl_subgroupClusteredOr(ivec4  value, uint clusterSize) noexcept;
[[spirv::builtin]] uint   gl_subgroupClusteredOr(uint   value, uint clusterSize) noexcept;
[[spirv::builtin]] uvec2  gl_subgroupClusteredOr(uvec2  value, uint clusterSize) noexcept;
[[spirv::builtin]] uvec3  gl_subgroupClusteredOr(uvec3  value, uint clusterSize) noexcept;
[[spirv::builtin]] uvec4  gl_subgroupClusteredOr(uvec4  value, uint clusterSize) noexcept;
[[spirv::builtin]] bool   gl_subgroupClusteredOr(bool   value, uint clusterSize) noexcept;
[[spirv::builtin]] bvec2  gl_subgroupClusteredOr(bvec2  value, uint clusterSize) noexcept;
[[spirv::builtin]] bvec3  gl_subgroupClusteredOr(bvec3  value, uint clusterSize) noexcept;
[[spirv::builtin]] bvec4  gl_subgroupClusteredOr(bvec4  value, uint clusterSize) noexcept;

[[spirv::builtin]] int    gl_subgroupClusteredXor(int    value, uint clusterSize) noexcept;
[[spirv::builtin]] ivec2  gl_subgroupClusteredXor(ivec2  value, uint clusterSize) noexcept;
[[spirv::builtin]] ivec3  gl_subgroupClusteredXor(ivec3  value, uint clusterSize) noexcept;
[[spirv::builtin]] ivec4  gl_subgroupClusteredXor(ivec4  value, uint clusterSize) noexcept;
[[spirv::builtin]] uint   gl_subgroupClusteredXor(uint   value, uint clusterSize) noexcept;
[[spirv::builtin]] uvec2  gl_subgroupClusteredXor(uvec2  value, uint clusterSize) noexcept;
[[spirv::builtin]] uvec3  gl_subgroupClusteredXor(uvec3  value, uint clusterSize) noexcept;
[[spirv::builtin]] uvec4  gl_subgroupClusteredXor(uvec4  value, uint clusterSize) noexcept;
[[spirv::builtin]] bool   gl_subgroupClusteredXor(bool   value, uint clusterSize) noexcept;
[[spirv::builtin]] bvec2  gl_subgroupClusteredXor(bvec2  value, uint clusterSize) noexcept;
[[spirv::builtin]] bvec3  gl_subgroupClusteredXor(bvec3  value, uint clusterSize) noexcept;
[[spirv::builtin]] bvec4  gl_subgroupClusteredXor(bvec4  value, uint clusterSize) noexcept;

// GL_KHR_shader_subgroup_quad

[[spirv::builtin]] float  gl_subgroupQuadBroadcast(float  value, uint id) noexcept;
[[spirv::builtin]] vec2   gl_subgroupQuadBroadcast(vec2   value, uint id) noexcept;
[[spirv::builtin]] vec3   gl_subgroupQuadBroadcast(vec3   value, uint id) noexcept;
[[spirv::builtin]] vec4   gl_subgroupQuadBroadcast(vec4   value, uint id) noexcept;
[[spirv::builtin]] int    gl_subgroupQuadBroadcast(int    value, uint id) noexcept;
[[spirv::builtin]] ivec2  gl_subgroupQuadBroadcast(ivec2  value, uint id) noexcept;
[[spirv::builtin]] ivec3  gl_subgroupQuadBroadcast(ivec3  value, uint id) noexcept;
[[spirv::builtin]] ivec4  gl_subgroupQuadBroadcast(ivec4  value, uint id) noexcept;
[[spirv::builtin]] uint   gl_subgroupQuadBroadcast(uint   value, uint id) noexcept;
[[spirv::builtin]] uvec2  gl_subgroupQuadBroadcast(uvec2  value, uint id) noexcept;
[[spirv::builtin]] uvec3  gl_subgroupQuadBroadcast(uvec3  value, uint id) noexcept;
[[spirv::builtin]] uvec4  gl_subgroupQuadBroadcast(uvec4  value, uint id) noexcept;
[[spirv::builtin]] bool   gl_subgroupQuadBroadcast(bool   value, uint id) noexcept;
[[spirv::builtin]] bvec2  gl_subgroupQuadBroadcast(bvec2  value, uint id) noexcept;
[[spirv::builtin]] bvec3  gl_subgroupQuadBroadcast(bvec3  value, uint id) noexcept;
[[spirv::builtin]] bvec4  gl_subgroupQuadBroadcast(bvec4  value, uint id) noexcept;
[[spirv::builtin]] double gl_subgroupQuadBroadcast(double value, uint id) noexcept;
[[spirv::builtin]] dvec2  gl_subgroupQuadBroadcast(dvec2  value, uint id) noexcept;
[[spirv::builtin]] dvec3  gl_subgroupQuadBroadcast(dvec3  value, uint id) noexcept;
[[spirv::builtin]] dvec4  gl_subgroupQuadBroadcast(dvec4  value, uint id) noexcept;

[[spirv::builtin]] float  gl_subgroupQuadSwapHorizontal(float  value) noexcept;
[[spirv::builtin]] vec2   gl_subgroupQuadSwapHorizontal(vec2   value) noexcept;
[[spirv::builtin]] vec3   gl_subgroupQuadSwapHorizontal(vec3   value) noexcept;
[[spirv::builtin]] vec4   gl_subgroupQuadSwapHorizontal(vec4   value) noexcept;
[[spirv::builtin]] int    gl_subgroupQuadSwapHorizontal(int    value) noexcept;
[[spirv::builtin]] ivec2  gl_subgroupQuadSwapHorizontal(ivec2  value) noexcept;
[[spirv::builtin]] ivec3  gl_subgroupQuadSwapHorizontal(ivec3  value) noexcept;
[[spirv::builtin]] ivec4  gl_subgroupQuadSwapHorizontal(ivec4  value) noexcept;
[[spirv::builtin]] uint   gl_subgroupQuadSwapHorizontal(uint   value) noexcept;
[[spirv::builtin]] uvec2  gl_subgroupQuadSwapHorizontal(uvec2  value) noexcept;
[[spirv::builtin]] uvec3  gl_subgroupQuadSwapHorizontal(uvec3  value) noexcept;
[[spirv::builtin]] uvec4  gl_subgroupQuadSwapHorizontal(uvec4  value) noexcept;
[[spirv::builtin]] bool   gl_subgroupQuadSwapHorizontal(bool   value) noexcept;
[[spirv::builtin]] bvec2  gl_subgroupQuadSwapHorizontal(bvec2  value) noexcept;
[[spirv::builtin]] bvec3  gl_subgroupQuadSwapHorizontal(bvec3  value) noexcept;
[[spirv::builtin]] bvec4  gl_subgroupQuadSwapHorizontal(bvec4  value) noexcept;
[[spirv::builtin]] double gl_subgroupQuadSwapHorizontal(double value) noexcept;
[[spirv::builtin]] dvec2  gl_subgroupQuadSwapHorizontal(dvec2  value) noexcept;
[[spirv::builtin]] dvec3  gl_subgroupQuadSwapHorizontal(dvec3  value) noexcept;
[[spirv::builtin]] dvec4  gl_subgroupQuadSwapHorizontal(dvec4  value) noexcept;

[[spirv::builtin]] float  gl_subgroupQuadSwapVertical(float  value) noexcept;
[[spirv::builtin]] vec2   gl_subgroupQuadSwapVertical(vec2   value) noexcept;
[[spirv::builtin]] vec3   gl_subgroupQuadSwapVertical(vec3   value) noexcept;
[[spirv::builtin]] vec4   gl_subgroupQuadSwapVertical(vec4   value) noexcept;
[[spirv::builtin]] int    gl_subgroupQuadSwapVertical(int    value) noexcept;
[[spirv::builtin]] ivec2  gl_subgroupQuadSwapVertical(ivec2  value) noexcept;
[[spirv::builtin]] ivec3  gl_subgroupQuadSwapVertical(ivec3  value) noexcept;
[[spirv::builtin]] ivec4  gl_subgroupQuadSwapVertical(ivec4  value) noexcept;
[[spirv::builtin]] uint   gl_subgroupQuadSwapVertical(uint   value) noexcept;
[[spirv::builtin]] uvec2  gl_subgroupQuadSwapVertical(uvec2  value) noexcept;
[[spirv::builtin]] uvec3  gl_subgroupQuadSwapVertical(uvec3  value) noexcept;
[[spirv::builtin]] uvec4  gl_subgroupQuadSwapVertical(uvec4  value) noexcept;
[[spirv::builtin]] bool   gl_subgroupQuadSwapVertical(bool   value) noexcept;
[[spirv::builtin]] bvec2  gl_subgroupQuadSwapVertical(bvec2  value) noexcept;
[[spirv::builtin]] bvec3  gl_subgroupQuadSwapVertical(bvec3  value) noexcept;
[[spirv::builtin]] bvec4  gl_subgroupQuadSwapVertical(bvec4  value) noexcept;
[[spirv::builtin]] double gl_subgroupQuadSwapVertical(double value) noexcept;
[[spirv::builtin]] dvec2  gl_subgroupQuadSwapVertical(dvec2  value) noexcept;
[[spirv::builtin]] dvec3  gl_subgroupQuadSwapVertical(dvec3  value) noexcept;
[[spirv::builtin]] dvec4  gl_subgroupQuadSwapVertical(dvec4  value) noexcept;

[[spirv::builtin]] float  gl_subgroupQuadSwapDiagonal(float  value) noexcept;
[[spirv::builtin]] vec2   gl_subgroupQuadSwapDiagonal(vec2   value) noexcept;
[[spirv::builtin]] vec3   gl_subgroupQuadSwapDiagonal(vec3   value) noexcept;
[[spirv::builtin]] vec4   gl_subgroupQuadSwapDiagonal(vec4   value) noexcept;
[[spirv::builtin]] int    gl_subgroupQuadSwapDiagonal(int    value) noexcept;
[[spirv::builtin]] ivec2  gl_subgroupQuadSwapDiagonal(ivec2  value) noexcept;
[[spirv::builtin]] ivec3  gl_subgroupQuadSwapDiagonal(ivec3  value) noexcept;
[[spirv::builtin]] ivec4  gl_subgroupQuadSwapDiagonal(ivec4  value) noexcept;
[[spirv::builtin]] uint   gl_subgroupQuadSwapDiagonal(uint   value) noexcept;
[[spirv::builtin]] uvec2  gl_subgroupQuadSwapDiagonal(uvec2  value) noexcept;
[[spirv::builtin]] uvec3  gl_subgroupQuadSwapDiagonal(uvec3  value) noexcept;
[[spirv::builtin]] uvec4  gl_subgroupQuadSwapDiagonal(uvec4  value) noexcept;
[[spirv::builtin]] bool   gl_subgroupQuadSwapDiagonal(bool   value) noexcept;
[[spirv::builtin]] bvec2  gl_subgroupQuadSwapDiagonal(bvec2  value) noexcept;
[[spirv::builtin]] bvec3  gl_subgroupQuadSwapDiagonal(bvec3  value) noexcept;
[[spirv::builtin]] bvec4  gl_subgroupQuadSwapDiagonal(bvec4  value) noexcept;
[[spirv::builtin]] double gl_subgroupQuadSwapDiagonal(double value) noexcept;
[[spirv::builtin]] dvec2  gl_subgroupQuadSwapDiagonal(dvec2  value) noexcept;
[[spirv::builtin]] dvec3  gl_subgroupQuadSwapDiagonal(dvec3  value) noexcept;
[[spirv::builtin]] dvec4  gl_subgroupQuadSwapDiagonal(dvec4  value) noexcept;

////////////////////////////////////////////////////////////////////////////////
// GL_ARB_shader_clock

// OpReadClockKHR
[[spirv::builtin]] uint64_t gl_clock() noexcept;

////////////////////////////////////////////////////////////////////////////////
// SPIRV data storage. These has internal linkage but are marked extern
// to allow const qualifiers.
extern const char* const __spirv_data;
extern const size_t __spirv_size;

#ifdef SPIRV_IMPLICIT_NAMESPACE
} // namespace

#endif

)text";

template<typename type_t>
static type_t* get_decl(ident_t ident, scope_ns_t* ns) {
  decl_t* decl = ns->find_name(ident);
  return cir_cast<type_t>(decl);
}

spirv_decls_t make_spirv_decls(context_t& ctx) {
  spirv_decls_t decls { };
  fe::grammar_t g { ctx };

  // Parse the pre-declarations.  
  range_t range = frontend.preprocessor->inject_text(spirv_pre, 
    "SPIR-V implicit declarations");
  auto series = g.syntax_series(range);
  g.statements(series->attr);

  if(ctx.errors->size())
    throw logging::abort_t();

  // Get a pointer to the namespace.
  scope_ns_t* ns = frontend.global_ns.get();
  if(frontend.options.spirv_namespace.size()) {
    ns = cir_cast<scope_ns_t>(frontend.global_ns->find_name(
      decl_name_t(frontend.ident(frontend.options.spirv_namespace))));
  }

  #define GET_CLASS(name) \
    get_decl<class_t>(ident_##name, ns)
  #define GET_ENUM(name) \
    get_decl<type_enum_t>(ident_##name, ns)

  #define SET_MEMBER(name) \
    decls.name = decls.gl_PerVertex->def->find_member( \
      decl_name_t { ident_##name }, 0);
  #define SET_CLASS(name) \
    decls.name = get_decl<class_t>(ident_##name, ns)
  #define SET_OBJECT(name) \
    decls.name = get_decl<decl_object_t>(ident_##name, ns)
  #define SET_FUNCTION(name) \
    decls.name = get_decl<function_t>(ident_##name, ns)
  #define SET_ENUM(name) \
    decls.name = GET_ENUM(name)

  SET_ENUM(gl_layout_t);
  SET_ENUM(gl_format_t);
  SET_ENUM(gltese_primitive_t);
  SET_ENUM(gltese_spacing_t);
  SET_ENUM(gltese_ordering_t);
  SET_ENUM(glgeom_input_t);
  SET_ENUM(glgeom_output_t);
  SET_ENUM(glfrag_origin_t);
  SET_ENUM(glmesh_output_t);


  //////////////////////////////////////////////////////////////////////////////
  // Sampler types

  // Floating-point opaque types.
  decls.samplers[0].sampler1D =              GET_ENUM(sampler1D);
  decls.samplers[0].sampler1DShadow =        GET_ENUM(sampler1DShadow);
  decls.samplers[0].sampler1DArray =         GET_ENUM(sampler1DArray);
  decls.samplers[0].sampler1DArrayShadow =   GET_ENUM(sampler1DArrayShadow);
  decls.samplers[0].sampler2D =              GET_ENUM(sampler2D);
  decls.samplers[0].sampler2DShadow =        GET_ENUM(sampler2DShadow);
  decls.samplers[0].sampler2DArray =         GET_ENUM(sampler2DArray);
  decls.samplers[0].sampler2DArrayShadow =   GET_ENUM(sampler2DArrayShadow);
  decls.samplers[0].sampler2DMS =            GET_ENUM(sampler2DMS);
  decls.samplers[0].sampler2DMSArray =       GET_ENUM(sampler2DMSArray);
  decls.samplers[0].sampler2DRect =          GET_ENUM(sampler2DRect);
  decls.samplers[0].sampler2DRectShadow =    GET_ENUM(sampler2DRectShadow);
  decls.samplers[0].sampler3D =              GET_ENUM(sampler3D);
  decls.samplers[0].samplerCube =            GET_ENUM(samplerCube);
  decls.samplers[0].samplerCubeShadow =      GET_ENUM(samplerCubeShadow);
  decls.samplers[0].samplerCubeArray =       GET_ENUM(samplerCubeArray);
  decls.samplers[0].samplerCubeArrayShadow = GET_ENUM(samplerCubeArrayShadow);
  decls.samplers[0].samplerBuffer =          GET_ENUM(samplerBuffer);

  // Signed integer opaque types.
  decls.samplers[1].sampler1D =              GET_ENUM(isampler1D);
  decls.samplers[1].sampler1DArray =         GET_ENUM(isampler1DArray);
  decls.samplers[1].sampler2D =              GET_ENUM(isampler2D);
  decls.samplers[1].sampler2DArray =         GET_ENUM(isampler2DArray);
  decls.samplers[1].sampler2DMS =            GET_ENUM(isampler2DMS);
  decls.samplers[1].sampler2DMSArray =       GET_ENUM(isampler2DMSArray);
  decls.samplers[1].sampler2DRect =          GET_ENUM(isampler2DRect);
  decls.samplers[1].sampler3D =              GET_ENUM(isampler3D);
  decls.samplers[1].samplerCube =            GET_ENUM(isamplerCube);
  decls.samplers[1].samplerCubeArray =       GET_ENUM(isamplerCubeArray);
  decls.samplers[1].samplerBuffer =          GET_ENUM(isamplerBuffer);

  // Unsigned integer opaque types.
  decls.samplers[2].sampler1D =              GET_ENUM(usampler1D);
  decls.samplers[2].sampler1DArray =         GET_ENUM(usampler1DArray);
  decls.samplers[2].sampler2D =              GET_ENUM(usampler2D);
  decls.samplers[2].sampler2DArray =         GET_ENUM(usampler2DArray);
  decls.samplers[2].sampler2DMS =            GET_ENUM(usampler2DMS);
  decls.samplers[2].sampler2DMSArray =       GET_ENUM(usampler2DMSArray);
  decls.samplers[2].sampler2DRect =          GET_ENUM(usampler2DRect);
  decls.samplers[2].sampler3D =              GET_ENUM(usampler3D);
  decls.samplers[2].samplerCube =            GET_ENUM(usamplerCube);
  decls.samplers[2].samplerCubeArray =       GET_ENUM(usamplerCubeArray);
  decls.samplers[2].samplerBuffer =          GET_ENUM(usamplerBuffer);

  //////////////////////////////////////////////////////////////////////////////
  // Image types

  // Floating-point opaque types.
  decls.images[0].image1D =                  GET_ENUM(image1D);
  decls.images[0].image1DArray =             GET_ENUM(image1DArray);
  decls.images[0].image2D =                  GET_ENUM(image2D);
  decls.images[0].image2DArray =             GET_ENUM(image2DArray);
  decls.images[0].image2DMS =                GET_ENUM(image2DMS);
  decls.images[0].image2DMSArray =           GET_ENUM(image2DMSArray);
  decls.images[0].image2DRect =              GET_ENUM(image2DRect);
  decls.images[0].image3D =                  GET_ENUM(image3D);
  decls.images[0].imageCube =                GET_ENUM(imageCube);
  decls.images[0].imageCubeArray =           GET_ENUM(imageCubeArray);
  decls.images[0].imageBuffer =              GET_ENUM(imageBuffer);
  decls.images[0].subpassInput =             GET_ENUM(subpassInput);
  decls.images[0].subpassInputMS =           GET_ENUM(subpassInputMS);

  // Signed integer opaque types.
  decls.images[1].image1D =                  GET_ENUM(iimage1D);
  decls.images[1].image1DArray =             GET_ENUM(iimage1DArray);
  decls.images[1].image2D =                  GET_ENUM(iimage2D);
  decls.images[1].image2DArray =             GET_ENUM(iimage2DArray);
  decls.images[1].image2DMS =                GET_ENUM(iimage2DMS);
  decls.images[1].image2DMSArray =           GET_ENUM(iimage2DMSArray);
  decls.images[1].image2DRect =              GET_ENUM(iimage2DRect);
  decls.images[1].image3D =                  GET_ENUM(iimage3D);
  decls.images[1].imageCube =                GET_ENUM(iimageCube);
  decls.images[1].imageCubeArray =           GET_ENUM(iimageCubeArray);
  decls.images[1].imageBuffer =              GET_ENUM(iimageBuffer);
  decls.images[1].subpassInput =             GET_ENUM(isubpassInput);
  decls.images[2].subpassInputMS =           GET_ENUM(isubpassInputMS);
  
  // Unsigned integer opaque types.
  decls.images[2].image1D =                  GET_ENUM(uimage1D);
  decls.images[2].image1DArray =             GET_ENUM(uimage1DArray);
  decls.images[2].image2D =                  GET_ENUM(uimage2D);
  decls.images[2].image2DArray =             GET_ENUM(uimage2DArray);
  decls.images[2].image2DMS =                GET_ENUM(uimage2DMS);
  decls.images[2].image2DMSArray =           GET_ENUM(uimage2DMSArray);
  decls.images[2].image2DRect =              GET_ENUM(uimage2DRect);
  decls.images[2].image3D =                  GET_ENUM(uimage3D);
  decls.images[2].imageCube =                GET_ENUM(uimageCube);
  decls.images[2].imageCubeArray =           GET_ENUM(uimageCubeArray);
  decls.images[2].imageBuffer =              GET_ENUM(uimageBuffer);
  decls.images[2].subpassInput =             GET_ENUM(usubpassInput);
  decls.images[2].subpassInputMS =           GET_ENUM(usubpassInputMS);

  SET_OBJECT(__spirv_data);
  SET_OBJECT(__spirv_size);

  #undef GET_CLASS
  #undef GET_ENUM
  #undef SET_FUNCTION
  #undef SET_OBJECT
  #undef SET_MEMBER
  #undef SET_CLASS

  return decls;
}

END_SEMA_NAMESPACE
