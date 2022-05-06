# AyanBot

Ayan 是基于 Modern C++ 编写的轻量，异步，易于扩展的跨平台 QQ机器人 框架。其底层与实现了 [OneBot-v11](https://github.com/botuniverse/onebot-11) 标准的协议适配器进行交互， 为上层实现需求的开发者提供易于使用的接口。  


# 设计目标  
+ 易于开发和部署  
    使开发者能专注于功能的实现，使用者能方便地进行功能管理和权限控制， 最终达到任何用户都能灵活便捷地满足个人需求的效果。  

    如果你想快速部署自己的 Bot， 参见 [快速开始](doc/README.md)  
    如果你想开发个性化的的功能， 参见 [服务样例](example/README.md)， [接口手册](doc/UserManual.md), [服务开发教程](doc/1-Hello.md)

+ 轻量高效    
    无论是开发者还是使用者均不必为他们不需要的部分付出任何额外的心智成本， 或是时间空间上的代价， 一切都是“零开销抽象”的。   

    你可以查看 [开发者文档](doc/dev/README.md) 来了解 Ayan 是如何实现这一目标的。  

# 文档  
文档参见 [Doc](doc/README.md)  

交流QQ群：933327998   

# 特性
你可以浏览 [Features](doc/Features.md) 来获取详细的特性支持.

## 主要特性
+ `易于使用`  
    借助于 Modern C++ 的特性与设计方法， 那些可能被频繁使用的接口都尽量地润色到符合（个人认为）的人体工学。

+ `异步`   
    Ayan 同时提供了基于 future 和 callback 的异步解决方案。

    
+ `高效`   
    由于缺少 benchmark，~~便将高效列为 feature 有些为时过早~~， 但目前已知数据为：  
    + 加载全部 example service 无脚本语言接口的情况下， release 版内存占用低于 1mb 
    + 开启 python interface (裸解释器) 将升高 7-10 mb  
    + 每开启一个 lua interface (裸解释器) 将升高 约 1 mb      
    
    Ayan 会始终将高效作为目标， 以适应在简陋硬件条件下的开发。

## 次要特性
+ `丰富的基础组件`   
这主要归功于开源网络库 [libhv](https://github.com/ithewei/libhv) 提供的各种基础组件，其次 Ayan 也封装了一些更为常用的工具， 位于 `src/core/utility` 中， 关于如何使用工具， 参见 [test/utility]().  
    详见 [libhv](https://github.com/ithewei/libhv) 的 [接口手册](https://blog.csdn.net/GG_SiMiDa/article/details/103976875), [使用教程](https://hewei.blog.csdn.net/article/details/113733758?spm=1001.2014.3001.5502)

+ `脚本语言接口`    
    Ayan 提供了两种脚本语言的接口: python & lua, 均默认不开启。   
    参见 [开启脚本语言接口](doc/README.md/#开启脚本语言接口)

    ## python   
    通过 pybind11 绑定到外部 python 解释器，支持引入 python 外部库 
    
    ## Lua
    通过 sol3 嵌入 lua 解释器, 支持所有的 lua 标准库.
    
# 核心概念  
Ayan 中的核心概念包括以下几点：
+ `Env`    
    Env 中集成了不同机器人中公用部分，包括用于完成异步、定时任务的线程池，管理员权限的控制以及对各种`Service`和日志流的管理。 

+ `Bot`   
    每个 Bot 持有一条 WebSocket 连接并与对应的与 onebot 协议适配器进行交互， 向上提供 onebot API. 通过订阅 `Env` 中的 `Service` 进行个性化的服务配置。

+ `Service`   
    Service 负责完成各种具体功能， 不同种类的 Service 只会响应自己负责的 `Event`, 彼此不会干扰。 Service 通过实现不同的 [Service Trait](doc/dev/service_system.md) 来控制自身加载时、响应时、甚至空闲时的行为。 Service 只会加载于 Env 中， 被 Bot 订阅后才会在 Bot 上生效。    

除此之外， 还有一些概念基于已有的 [onebot](https://github.com/botuniverse/onebot-11) 标准， 例如 Event, Message，在 Ayan 中也有同样的封装。

# Road Map
+ Lua Extension API
+ 更加完善的文档及测试用例
+ Service Market

# Q&A 

+ `Q: 为什么这个框架叫 Ayan？`   
A:  QQ机器人实现任何功能都需要对包括消息在内的各种事件无一遗漏地进行检查并作出反应，与“察言观色”这一成语语义相合。 同时，“阿言” 是一位我很喜欢的动漫角色的昵称， 其绝技名也恰好为 “察言观色”. 因此借人物昵称命名机器人框架为阿言。 

+ `Q: 为什么不向脚本语言中注册 ayan 的接口来使用脚本语言开发服务？`  
A: 设计上， Python 接口用于引入 python 社区中丰富的第三方库， 以完成各种 “未曾设想的功能”而非处理消息回复等逻辑， 因此并未向其中注册 Ayan 的 api 来进行扩展。 如果你想全部使用 python 来完成服务开发， [nonebot2](https://github.com/nonebot/nonebot2) 是更好的选择.   
而 Lua 接口则是专门为了解决编译型语言的不够灵活， 服务开发不够快速敏捷等问题而引入的， 但现阶段 lua extension api 的形式尚未确定, 后续将会完善并开放。

