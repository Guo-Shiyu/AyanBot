#include "ayan/core/service.h"

#include <cuchar>

namespace ayan {
namespace literals {
std::u32string operator""_utf8(const char *str, size_t len) {
  static std::vector<char32_t> dyn_buf{};
  std::mbstate_t               state{};

  if (len > dyn_buf.size())
    dyn_buf.reserve(len * 2);
  dyn_buf.clear();

  size_t nchar = std::mbrtoc32(dyn_buf.data(), str, len, &state);
  return std::u32string(dyn_buf.data(), nchar);
}
} // namespace literals


} // namespace ayan