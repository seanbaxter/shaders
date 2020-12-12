#if __circle_build__ < 108
#error "Circle build 108 required"
#endif

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
