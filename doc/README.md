# Document
目录结构
~~~
   -|
    |- UserManual.md            面向服务开发者的接口手册
    |- 1-Hello.md               入门 第一个 Service          
    |- 2-Echo.md                实现 Echo Service
    |- 3-Request.md             处理 加群 / 加好友请求
    |- 4-Notice.md              处理 通知事件 
    |- 5-Meta.md                处理 元事件
    |- 6-Mixin.md               同时处理多种事件
    |- 7-Timer.md               定时器接口
    |- 8-State.md               带状态的 Service
    |- 9-Script.md              脚本接口入门
    |- 10-Recommand.md          Service 开发的一些建议    
    |- dev                      面向 Ayan 框架开发者的手册
        |-...
~~~

## 环境准备
1. 要求编译器支持 C++ 20 标准   
下面列出的版本为已经验证过的可用版本， 不保证是对应编译器的最低可用版本  
    + MSVC 17.0 (VS2022) 
    + Clang 10.0.0
    + GCC 11.4 
   

2. 安装 xmake  
可以从 [这里](https://xmake.io/#/guide/installation) 找到安装 xmake 的办法

3. 安装任意的 onebot 协议适配器，并要求其开启正向 WebSocket 服务， 例如：

    + [go-cqhttp ](https://github.com/Mrs4s/go-cqhttp)  
    基于 Go 的跨平台 onebot 实现， 支持 onebot11  
    + [Walle-core](https://github.com/abrahum/Walle-core)   
    基于 Rust 的跨平台（存疑） onebot 实现， 支持 onebot11/12     
     

# Build 
Ayan 使用 [xmake](https://github.com/xmake-io/xmake) 跨平台构建，参见 [BUILD.md](../BUILD.md).  

# Run First Service
Ayan 本身不负责模拟 QQ 客户端对 raw UDP packet 拆包， 而是由协议适配器完成这项工作。 Ayan 只与协议适配器进行通信， 在此基础上实现消息处理逻辑。

1. 开启某个 Onebot 协议适配器， 获取正向 WebSocket 服务的监听地址和端口号。  
建议使用搜索引擎获取 go-cqhttp 的使用方法， 或者见本文末尾的潦草教程。

2. 编辑 `main.cpp`:
``` c++
#include "ayan/ayan.h"
#include "ayan/service/edump.h"

using namespace ayan;

int main(int argc, char **argv) {
  if (argc != 3) {
    fmt::print("usuage: {0} <WebSocket Server IP> <Port>\n", argv[0]);
    ::exit(0);
  }

  util::clear_screen();
  fmt::print("hello ayan! \n");

  // ip, port 字段即为 协议适配器 正向ws 所监听的地址和端口号
  const char *ip   = argv[1];
  int         port = std::atoi(argv[2]);

  auto ayan = //
      BotBuilder::from(Envir::global())
          .connect(ip, port)
          .with_qid(2821006329)
          .with_name("Ayan")
          .build();

  // 为 bot 订阅该服务
  ayan->subscribe<DumpOnebotEvent>();   

  // 启动 bot 
  ayan->run();

  util::block_here();
  return 0;
}
```

3. 启动 go-cqhttp, 并使用以下语句重新编译和启动 Ayan 

~~~
    $ xmake 
    $ xmake -run HelloAyan <ip> <port>
~~~
此时 Ayan 启动完毕且服务加载成功。每当收到消息时， 将会在 Ayan 的控制台中看到关于 event dump 的信息。

其他服务的示例请参见 [example](../example/README.md)   
开发定制化的服务请参见 [UserManual.md](UserManual.md) 以及 [1-Hello.md](1-Hello.md)


## 开启脚本语言接口
### Python 
1. 修改 xmake.lua 中`AYAN_PYTHON_INTERFACE` 为 `true` 并设置 `FOREIGN_INTERPRETER_PATH` 的值为本地的解释器路径
        （通常为 Anaconda 中 python.exe 所在路径， 以方便引入外部库）
2. 在任一 Env 中提供此项服务以完成解释器的加载, bot `无需` 像普通服务一样进行订阅， 即可在任意代码中使用如下接口进行访问
    ~~~ c++
        // 某一环境中加载解释器
        env.supplv<buildin_service::PythonInterface>();
        
        // 多次加载也不会出现异常
        other_env.supplv<buildin_service::PythonInterface>();

        // 即可在任意代码中使用如下语句
        py::exec(R"(
            import sys
            print(sys.platform)

            kwargs = dict(name="World", number=42) 
            hi = "Hello, {name}! The answer is {number}".format(**kwargs)
            print(hi)
        )");

        // 执行文件
        py::eval_file("script.py");
    ~~~

    可以通过查询 pybind11 的[文档](https://pybind11.readthedocs.io/_/downloads/en/latest/pdf/)来获得更多用法.  
    ### 注意： 
    由此种方法开启的解释器仍然受 GIL 的限制， 因此所有代码中使用的解释器均为同一个全局解释器，且版本与外部解释器路径下的版本相同。   
    此外还存在以下限制：  
    1. 在解释器 drop 后（也即任何一个 Env 中都已卸载 Python Interface 服务后）， c++ 中能不存在任何对 python 对象的引用， 否则会 crush , 请保证对象的生命周期短于解释器的生命周期。
    2. 在多线程并发争夺 解释器所有权 时， 需要自行加锁， Ayan 本身不保证解释器的线程安全。       


### Lua
1. 修改 xmake.lua 中 `LUA_INTERFACE` 为 `true`， 即可自动依赖 lua (latest verion)， 若需指定版本（包括 5.4 / 5.3 / luajit等）, 请参照 xmake 文档添加依赖配置.

2. 在任一代码片段中开启解释器并运行
    ~~~ c++
        // 新建解释器
        sol::state lua;

        // 开启所有内置标准库
        lua.open_libraries();
        
        lua.script(R"(
            require("other_lua_file")

            local x = 137
            print(x)
            
            local tab = {}
            table.insert(tab, "wuhu", x + 1)

            local welcome = "hello"
            print(welcome))");

        lua.script_file("script.lua");
    ~~~ 
    由于lua的小巧和灵活， 你可以在任意线程中开启任意多个解释器而不会造成内存占用膨胀问题。   
    更多用法参照 sol3 的[文档](https://sol2.readthedocs.io/en/latest/index.html)， 编写更加灵活的扩展工具。


# 附录： go-cqhttp 的安装和使用  

1. 到 [go-cqhttp release](https://github.com/Mrs4s/go-cqhttp/releases) 中下载所在平台对应的版本。

2. 运行go-cqhttp一次, 生成启动脚本 go-cqhttp.bat， 再通过启动脚本运行一次， 启用功能部分选择 `正向 WebSocket`, 此时将生成配置文件 `config.yaml`

3. 编辑配置文件, 根据注释提示填写中如下字段：
    + account : uin   
     account : password   
    对应你想要挂载的机器人的账号和密码（可为空）   
    
    + message : post-format  
    修改此项的值为 array， 表明使用 onebot 11 标准中 消息段 格式传递消息  

    + servers : ws : port   
    此项为你需要使用的本地端口号    

4. 使用 bat 重启 go-cqhttp, 扫码登陆，出现登陆成功日志即可    