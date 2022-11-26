#pragma once

#include "ayan/onebot/api.h"

namespace ayan {

/// TODO:
/// 如果下面这种方式不能编译通过， 那么就用这种
// struct ApiCaller : public onebot::ApiMixin<
//                        onebot::api::SendPrivateMessage, onebot::api::SendGroupMessage>
//                        {};

using ApiCaller =
    onebot::ApiMixin<onebot::api::SendPrivateMessage, onebot::api::SendGroupMessage>;

/// NOTICE:
/// ensure that we can reinterpret a pointer to any struct in namespace `onebot::api`
/// as a pointer to `ApiCaller`, that structs are required to be empty
static_assert(sizeof(ApiCaller) == sizeof(onebot::ApiCall));

template <typename T>
onebot::HookFn Deprecate = [](T _) {};

} // namespace ayan
