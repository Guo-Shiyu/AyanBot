#pragma once


#include <memory>

namespace ayan
{
    template <typename T>
    using Shared = std::shared_ptr<T>;

    // template <typename T>
    // using Unique = std::unique_ptr<T>;

}