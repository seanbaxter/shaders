// Semantic index 0 is implicit.
[[spirv::in, dxil::semantic("POSITION")]]
vec4 pos0;

extern "C" [[spirv::vert]]
void f0() {
  glvert_Output.Position = pos0;
}

// Semantic index 1 is explicit.
[[spirv::in, dxil::semantic("POSITION1")]]
vec4 pos1;

extern "C" [[spirv::vert]]
void f1() {
  glvert_Output.Position = pos1;
}

// Semantic index is a template parameter.
template<typename type_t, int index>
[[spirv::in, dxil::semantic("POSITION", index)]]
type_t input_index;

extern "C" [[spirv::vert]]
void f2() {
  glvert_Output.Position = input_index<vec4, 2>;
}

// Both semantic name and index are template parameters.
template<typename type_t, const char name[], int index>
[[spirv::in, dxil::semantic(name, index)]]
type_t input_name_index;

extern "C" [[spirv::vert]]
void f3() {
  glvert_Output.Position = input_name_index<vec4, "POSITION", 3>;
}