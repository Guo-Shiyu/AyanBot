# Mixin 服务  
Mixin 服务是为了编写一个服务类同时处理多种事件， 例如同时处理加群请求与消息事件。

与之前其他服务的区别仅仅在于， 继承时需要满足特定的格式要求。  
设 新服务名称为 O, 欲处理事件类型为 T1, T2, T3...  Tn = { M, N, R, A }, 分别对应(message， notice， request， meta)， 那么应当写作：

~~~ C++
class O : public MixinService<T1<O>, T2<O>... Tn<O>> 
{...};

// 例如 MixinService.hpp 中同时处理 Message Event 和 Request Event 的服务：

struct MsgAndReqService 
: public MixinService<MessageService<MsgAndReqService>, RequestService<MsgAndReqService>>
{...};

~~~
此外， 对于 Mixin Service 有一个特殊要求：
+ 若重写了某一接口函数， 那么要么同时实现 T1, T2... Tn 中所有的该接口函数， 要么均同时不实现该接口函数。    
    即， 要么像 MixinService.hpp 中 MsgAndReqService 一样， 为所有父类都实现 trig/act 接口， 要么像 MsgAndMetaService 类一样均使用默认接口。  
    这是因为 C++ 的方法重写导致的对同名基类方法的遮盖。  