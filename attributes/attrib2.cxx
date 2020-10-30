#include <iostream>
#include <type_traits>

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

using title [[attribute]] = const char*;
using url   [[attribute]] = const char*;
using magic [[attribute]] = int;

enum class edge {
  left, top, right, bottom,
};
enum class corner {
  nw, ne, se, sw,
};

template<typename type_t>
std::string to_string(type_t x) {
  if constexpr(std::is_enum_v<type_t>) {
    return enum_to_name(x);

  } else if constexpr(std::is_same_v<bool, type_t>) {
    return x ? "true" : "false";

  } else if constexpr(std::is_arithmetic_v<type_t>) {
    return std::to_string(x);

  } else {
    static_assert(std::is_same_v<const char*, type_t>);
    return x;
  }
}

struct foo_t {
  [[.edge=right, .url="https://www.fake.url/"]] int x;
  [[.title="Sometimes a vowel", .corner=ne]]    int y;
  [[.magic=10101, .title="The magic number"]]   int z;
};

template<typename type_t>
void print_member_attributes() {
  std::cout<< "Member attributes for "<< @type_string(type_t)<< "\n";

  @meta for(int i = 0; i < @member_count(type_t); ++i) {{
    // Loop over each member.
    std::cout<< "  "<< @member_name(type_t, i)<< ":\n";

    // @member_attribute_list is a type parameter pack of all attribute names.
    // Loop over them and print them out.
    @meta for typename(t : { @member_attribute_list(type_t, i)... }) {
      std::cout<< "    "<< @type_string(t)<< " = "<< 
        to_string(@member_attribute(type_t, i, t))<< "\n";
    }
  }}
}

int main() {
  print_member_attributes<foo_t>();
}