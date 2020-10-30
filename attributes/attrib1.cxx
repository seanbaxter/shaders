#include <cstdio>

template<typename type_t>
const char* enum_to_name(type_t x) {
  switch(x) {
    @meta for enum(type_t x2 : type_t)
      case x2:
        return @enum_name(x2);
    default:
      return "<unknown>";
  }
}

// Three kinds of attributes:
// 1. Class types.
// 2. Enum type.
// 3. Attribute aliases to any other type.

// Define a class type to be used as an attribute.
struct extent_t {
  int width, height;
};

// Name the class type after a dot in the attribute sequence.
// Because this is an aggregate (no user-provided constructor), use the
// braced aggregate initializer.
[[.extent_t { 800, 600 }]] int X;

// Define an enum for an attribute.
enum class shape_t {
  circle,
  square,
  triangle,
};

// Name the enum after the dot. For enum attributes, a single-identifier 
// token initializer will be matched to enumeration names.
[[.shape_t=triangle]] int Y;

// Define an attribute alias to some other type. This lets us reuse types
// without ambiguity.
using title_t [[attribute]] = const char*;

// Name the attribute alias. We can't name the underlying type. 
// The attribute alias is like an opaque typedef.
[[.title_t="A great var"]] int Z;

int main() {
  // The @attribute keyword is provided a declaration and an 
  // attribute type or attribute alias. It gives an lvalue to that
  // compile-time attribute.
  printf("X width = %d\n", @attribute(X, extent_t).width);  

  // If non-const, you can even modify it at compile time. This is helpful
  // for keeping counters.
  @meta ++@attribute(X, extent_t).width;
  printf("X width = %d\n", @attribute(X, extent_t).width);  

  // Use Circle reflection to turn this enum attribute into a string:
  printf("Y shape = %s\n", enum_to_name(@attribute(Y, shape_t)));

  // Print directly from a compile-time const char*.
  printf("Z title = %s\n", @attribute(Z, title_t));
}
