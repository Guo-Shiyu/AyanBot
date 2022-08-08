#pragma once


#include <memory>

namespace ayan
{
    using Qid = uint64_t;
    using MsgId = int32_t;

    template <typename T>
    using Shared = std::shared_ptr<T>;

    // template <typename T>
    // using Unique = std::unique_ptr<T>;

}