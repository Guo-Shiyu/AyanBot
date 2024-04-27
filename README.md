# Notice
由于 QQ 官方对协议库的封锁， 所有基于通讯协议的 onebot 协议适配器已经停止维护或不可用，Ayanbot 已事实上不再可用。

-> [QQBot 的未来及迁移建议](https://github.com/Mrs4s/go-cqhttp/issues/2471)

感谢上游每一位开发者为开源 QQ Bot 生态做出的贡献。 

# AyanBot

Ayan 是基于 Modern C++ 编写的轻量, 异步, 易于扩展的跨平台 Bot SDK。其底层与 onebot 实现进行交互,  为上层实现需求的开发者提供易于使用的基础接口。   

在使用此 SDK 进行开发之前, 建议阅读:  
+ [onebot 是什么](https://onebot.dev/)       

+ [onebot 实现与 SDK 的区别](https://onebot.dev/ecosystem.html)     

+ [Ayan 中与 onebot 保持一致的术语](https://12.onebot.dev/glossary/)   

目前 Ayan 对接的 onebot 标准是 [onebot11](https://github.com/botuniverse/onebot-11), 面向的平台是 Tencent QQ.   

建议对接的 onebot 实现是:   
+ [go-cqhttp](https://github.com/Mrs4s/go-cqhttp)  
+ [walle-q (rs-cqhttp)](https://github.com/onebot-walle/walle-q)

# 设计目标  
+ 易于开发和部署    
    `Service` 是 Ayan 对外提供服务的核心概念, 一切自定义的处理逻辑都是服务, 服务之间可以彼此依赖, 按需运行.    
    你可以查看 [服务样例](example/README.md)  来感受 Ayan 在处理聊天逻辑时的方便高效.    

    Ayan 的核心实现是 `libayan`, 作为一个库对外提供, 你可以将其内嵌在任何一个 C++ 程序中. 而作为服务的编写者, 你也可以将聊天逻辑所需的任何内容内嵌在一个 `Service` 中. Ayan 也随库提供了命令行版本和数个内置服务. 前者仅仅几十行代码, 实现 `main` 函数并调用了 `libayan` 提供的接口, 后者大都是 example 中的内容.

    Ayan 对外提供的各种接口是精心润色的, 尽量保持强大和简洁, 符合作者认为的人体工学,  同时预留了脚本语言接口, 也可以使用 `python` 和 `lua` 来让机器人更加丰富和灵活.  

    目前唯一与该设计目标相违背的点在于, Ayan 依赖 `cpp20` 语言标准进行编译, 这让其无法在某些不支持该标准的平台上运行, 例如一些嵌入式设备.   

+ 轻量高效     
    Ayan 的实现尽量追求最高性能. ~~但由于缺少 benchmark, 将高效列为 feature 有些为时过早~~,  目前已知数据为：  
    + 加载全部 example service 无脚本语言接口的情况下,  release 版内存占用低于 1mb    
    Ubuntu 22.0 LTS,   gcc 11.2 / clang 15, O3, strip all, no-LTO

    + 开启 python interface 将升高 7-10 mb  
    调用外部裸解释器, python 3.7 

    + 每开启一个 lua interface 将升高 < 1 mb        
    内嵌裸解释器,  lua 5.4 
    
    如果你发现了违背这一目标的地方, 欢迎 PR 和 ISSUE.      

你可以查看 [Ayan Design & Overview](todo) 来了解 Ayan 是如何实现以上目标的。  

# 文档  
## 构建   
参见 [BUILD.md](./BUILD.md)   

## 基于 Ayan 开发
参见 [Ayan Programmer's Manual]()  

+ 快速部署自己的 Bot    
参见 [快速开始](doc/README.md)    

+ 开发个性化的的聊天服务  
参见 [服务开发教程](doc/1-Hello.md), [服务样例](example/README.md)  

+ 了解 Ayan 对 onbot 标准的适配情况   
参见 [接口兼容性](doc/UserManual.md)  

# 交流   
QQ群：933327998      

欢迎 Issue / PR 

# Road Map
+ Lua Extension API
+ 更加完善的文档及测试用例
+ 更多内建 Service

# 其他

+ Ayan 的名字   
“阿言” 是一位我很喜欢的动漫角色的昵称.    

