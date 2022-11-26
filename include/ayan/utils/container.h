#pragma once

#include <algorithm>
#include <array>
#include <stdexcept>

namespace ayan {
template <typename Key, typename Value, size_t Size>
struct KDenseMap {
  using Elem = std::pair<Key, Value>;
  std::array<Elem, Size> series_;

  constexpr KDenseMap(std::initializer_list<Elem> &&i) {
    auto src  = i.begin();
    auto dest = series_.begin();

    while (dest != series_.end()) {
      *dest++ = *src++;
    }
  }

  constexpr Value at(const Key &k) const {
    auto it = std::find_if(series_.begin(), series_.end(), [&](const Elem &elem) {
      return elem.first == k;
    });
    if (it != series_.end())
      return it->second;
    else
      throw std::out_of_range("out of range in KDenseMap");
  }
};
} // namespace ayan