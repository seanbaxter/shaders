
# Shader parameters and push constants

All shader stage entry points in both GLSL are HLSL return void and take no function parameters. To pass data to a shader stage, you have to explicitly collect it into uniform or shader-stage buffor objects and bind those resources to the graphics context. This differs from OpenCL and CUDA, which accept kernel arguments through the [clSetKernelArg](https://www.khronos.org/registry/OpenCL/specs/3.0-unified/html/OpenCL_API.html#_setting_kernel_arguments) and [cudaLaunchKernel](https://docs.nvidia.com/cuda/cuda-runtime-api/group__CUDART__EXECUTION.html#group__CUDART__EXECUTION_1g5064cdf5d8e6741ace56fd8be951783c) APIs. These arguments are sent to the device as part of the kernel dispatch command, so no explicit binding is required. 

GLSL and Vulkan lack a shader parameter support, although Vulkan does support push constants, which are small binary objects sent with the dispatch command and implicitly bound to special uniform buffer objects in the shader program. Devices support push constants of at least 128 bytes. The [vkCmdPushConstants](https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/vkCmdPushConstants.html) selects the binary data on the host which gets bound to this special UBO on the indicated shader stage in a Vulkan pipeline.

To provide a smoother experience that more closely tracks CUDA and OpenCL, will implicitly declare a push constant shader variable when a shader entry point is marked with the `[[spirv::push]]` C++ attribute. Each function parameter is a data member in the push constant structure. When the shader is executed, the push constant is implicitly disaggregated, and each of its members is copied to an automatic storage duration variable in the shader body, to mimic the behavior of real function parameters.

[**push1.cxx**](pushc/push1.cxx)
```cpp
[[using spirv: buffer, binding(0)]]
mat4 x[];

[[using spirv: comp, local_size(128), push]]
void comp_main(mat4 matrix1, int count, int index, float scale) {
  // These parameters are implicitly packed into a push constant block
  // then disaggregated back into Function parameters. They act just like
  // normal parameters.
  x[count] = matrix1 * scale;

  // You can even modify them because they've been copied to Function storage.
  ++index;
}
```
```
               OpEntryPoint GLCompute %_Z9comp_mainMv4_4_fiif "_Z9comp_mainMv4_4_fiif" %8 %x
               OpExecutionMode %_Z9comp_mainMv4_4_fiif LocalSize 128 1 1
               OpName %_Z9comp_mainMv4_4_fiif "_Z9comp_mainMv4_4_fiif"
               OpDecorate %_struct_6 Block
               OpMemberDecorate %_struct_6 0 Offset 0
               OpMemberDecorate %_struct_6 0 ColMajor
               OpMemberDecorate %_struct_6 0 MatrixStride 16
               OpMemberDecorate %_struct_6 1 Offset 64
               OpMemberDecorate %_struct_6 2 Offset 68
               OpMemberDecorate %_struct_6 3 Offset 72

  %_struct_6 = OpTypeStruct %mat4v4float %int %int %float
%_ptr_PushConstant__struct_6 = OpTypePointer PushConstant %_struct_6
          %8 = OpVariable %_ptr_PushConstant__struct_6 PushConstant

%_Z9comp_mainMv4_4_fiif = OpFunction %void None %14
   %bb_entry = OpLabel
         %17 = OpVariable %_ptr_Function_mat4v4float Function
         %19 = OpVariable %_ptr_Function_int Function
         %20 = OpVariable %_ptr_Function_int Function
         %22 = OpVariable %_ptr_Function_float Function
         %25 = OpAccessChain %_ptr_PushConstant_mat4v4float %8 %int_0
         %26 = OpLoad %mat4v4float %25
               OpStore %17 %26
         %29 = OpAccessChain %_ptr_PushConstant_int %8 %int_1
         %30 = OpLoad %int %29
               OpStore %19 %30
         %32 = OpAccessChain %_ptr_PushConstant_int %8 %int_2
         %33 = OpLoad %int %32
               OpStore %20 %33
         %36 = OpAccessChain %_ptr_PushConstant_float %8 %int_3
         %37 = OpLoad %float %36
               OpStore %22 %37
         %38 = OpLoad %mat4v4float %17
         %39 = OpLoad %float %22
         %40 = OpMatrixTimesScalar %mat4v4float %38 %39
         %42 = OpAccessChain %_ptr_StorageBuffer__runtimearr_mat4v4float %x %int_0
         %43 = OpLoad %int %19
         %45 = OpAccessChain %_ptr_StorageBuffer_mat4v4float %42 %43
               OpStore %45 %40
         %46 = OpLoad %int %20
         %47 = OpIAdd %int %46 %int_1
               OpStore %20 %47
               OpReturn
               OpFunctionEnd
```

As with all shaders, the entry point `comp_main` remains a function with no parameters:
```
%_Z9comp_mainMv4_4_fiif = OpFunction %void None %14
```

Function storage class variables are allocated for each parameter, and the implicit push constant is disaggregated into these locals:

```
         %17 = OpVariable %_ptr_Function_mat4v4float Function
         %19 = OpVariable %_ptr_Function_int Function
         %20 = OpVariable %_ptr_Function_int Function
         %22 = OpVariable %_ptr_Function_float Function
         %25 = OpAccessChain %_ptr_PushConstant_mat4v4float %8 %int_0
         %26 = OpLoad %mat4v4float %25
               OpStore %17 %26
         %29 = OpAccessChain %_ptr_PushConstant_int %8 %int_1
         %30 = OpLoad %int %29
               OpStore %19 %30
         %32 = OpAccessChain %_ptr_PushConstant_int %8 %int_2
         %33 = OpLoad %int %32
               OpStore %20 %33
         %36 = OpAccessChain %_ptr_PushConstant_float %8 %int_3
         %37 = OpLoad %float %36
               OpStore %22 %37
```

Disaggregating the push constant causes references to parameters to yield mutable lvalues, consistent with ordinary C++ functions.

### Push constant limits

Unlike UBOs, push constants have very tight size limits. Only 128 bytes are guaranteed on all Vulkan platforms.

[**push2.cxx**](push2.cxx)
```cpp
[[using spirv: buffer, binding(0)]]
mat4 x[];

[[using spirv: comp, local_size(128), push]]
void comp_main(mat4 matrix1, mat4 matrix2, int count, int index, float scale) {
  // These parameters are implicitly packed into a push constant block
  // then disaggregated back into Function parameters. They act just like
  // normal parameters.
  x[count] = matrix1 * scale;

  // You can even modify them because they've been copied to Function storage.
  ++index;
}
```
```
$ circle -shader -o push2.spv push2.cxx
error: push2.cxx:5:6:34267
size of implicit push constant (144-bytes) exceeds 128-byte limit
void comp_main(mat4 matrix1, mat4 matrix2, int count, int index, float scale) { 
     ^
```

To prevent invalid shaders from being generated, push constants, either implicitly or explicitly declared, generate a compiler error if their sizes exceed 128 bytes.

[**push3.cxx**](push3.cxx)
```cpp
[[using spirv: buffer, binding(0)]]
mat4 x[];

[[using spirv: comp, local_size(128), push(256)]]
void comp_main(mat4 matrix1, mat4 matrix2, int count, int index, float scale) {
  // These parameters are implicitly packed into a push constant block
  // then disaggregated back into Function parameters. They act just like
  // normal parameters.
  x[count] = matrix1 * scale;

  // You can even modify them because they've been copied to Function storage.
  ++index;
}
```

Some devices support larger push constants. NVIDIA hardware supports push constants up to 256 bytes. The `[[spirv::push]]` attribute takes an optional max size operand. Setting this to 256 allows the shader to compile, although the resulting shader may produce a runtime error for devices that only support the minimum push constant size.

### Argument deduction of push constants

By defining a push constant through a shader's function parameters, we've gained the ability to use C++ to _deduce_ the structure of the push constant.

```cpp
#include <cstdio>
#include <array>

template<typename... types_t>
[[using spirv: comp, local_size(128), push]]
void my_kernel(types_t... args) {
  // Signature of shader and layout of push constant is inferred from
  // template parameters, which are deduced from the call arguments on the
  // host side.
  @meta printf("%d: %s\n", int..., @type_string(types_t))...;
}

template<typename... types_t>
struct tuple_t {
  types_t @(int...) ...;
};

template<typename... types_t>
void dispatch(/* vulkan pipeline args */ const types_t&... x) {
  // Put the arguments in an aggregate.
  tuple_t<types_t...> storage { x... };

  @meta printf("tuple storage is %zu bytes with %zu elements\n", 
    sizeof(storage), @member_count(decltype(storage)));

  // Can push from this address. It will have the same layout as the implicit
  // PushConstant type created by the shader.
  // vkCmdPushConstants(cmd, layout, stage, 0, sizeof(storage), &storage);
}

int main() {
  // Evaluate the shader call in an unevaluated context to infer template 
  // parameters.
  const char* name = 
    @spirv(my_kernel(std::array { 1, 2, 3 }, 5, 3.14, vec2(), mat3()));
  puts(name);

  // Capture parameters into a struct to pass to Vulkan.
  dispatch(std::array {1, 2, 3 }, 5, 3.14, vec2(), mat3());
}
```
```
$ circle -shader push4.cxx
tuple storage is 72 bytes with 5 elements
0: std::array<int, 3>
1: int
2: double
3: vec2
4: mat3
... wrote push4.spv
$ ./push4
_Z9my_kernelIJSt5arrayIiLm3EEidDv2_fMv3_3_fEEvT_
```

`my_kernel` is a compute shader taking a function parameter pack. The types in the parameter pack are deduced when by calling the function inside the `@spirv` extension. This is performed in an _unevaluated context_. The shader isn't actually called, but providing the arguments there selects which template specialization to use when generating the shader binary. Since the push constant structure is inferred from the shader parameter types, template argument deduction now generates push constants.

Circle automates the unpacking of the push constant into parameters on the shader side, but it doesn't yet automate packing arguments the push constant to send over the wire. Fortunately this can be done manually very easily. The `tuple_t` type is a one-line [Circle tuple](https://github.com/seanbaxter/circle/blob/master/reflection/README.md#dynamic-names). We can pass arguments to a `dispatch` function which takes them as a function parameter pack. We can them specialize `tuple_t` over the deduced types, and aggregate initialize an object from a pack expansion. The members in the tuple are layed out according to natural alignments, which matches the layout in the push constant. We can pass the size and pointer to the tuple object to `vkCmdPushConstants` to emulate a shader function call.

Unfortunately the design of Vulkan doesn't allow a single function call expression to both specialize the shader function template and pass push constant arguments to it. Hopefully a future Vulkan extension will allow Circle to add frontend porcelain like CUDA's chevron launch `<<< >>>` mechanism, which performs argument deduction to specialize a kernel template function, ODR-uses the kernel to generate code, and selects the kernel for execution and pushes its arguments, all in one expression.