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