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

# Build 
Ayan 使用 [xmake](https://github.com/xmake-io/xmake) 构建，以达到跨平台快速构建的目的。

任意平台下：  
1. 安装 xmake  
    可以从 [这里](https://xmake.io/#/guide/installation) 找到安装 xmake 的办法

2. clone 本项目后， 进入目录， 执行 xmake 命令， 
~~~
    $ git clone https://github.com/SilverCicada/AyanBot.git 
    $ cd AyanBot 
    $ xmake 
~~~

在这个过程中会xmake自动寻找并安装依赖。

3. 若需要生成特定工程项目, 使用:

~~~
    Visual Studio:
    $ xmake project -k vsxmake
    
    makefile:
    $ xmake project -k makefile
~~~


4. 编译成功后, 参照后续方法开启运行 HelloService.

注意：xmake 默认不使用代理进行下载， 若安装依赖时间过长或出现网络连接失败需要手动下载， 请通过以下命令配置代理 
~~~
    $ xmake g --help
    -x PROXY, --proxy=PROXY  Use proxy on given port. [PROTOCOL://]HOST[:PORT]
                                 e.g.
                                 - xmake g --proxy='http://host:port'
                                 - xmake g --proxy='https://host:port'
                                 - xmake g --proxy='socks5://host:port'
~~~

# Run Hello Service
Ayan 本身不负责模拟 QQ 客户端对 raw UDP packet 拆包， 而是由协议适配器完成这项工作。 Ayan 只与协议适配器进行通信， 在此基础上实现消息处理逻辑。

1. 下载某个 Onebot 协议适配器， 这里以 go-cqhttp 为例：
到 [go-cqhttp release](https://github.com/Mrs4s/go-cqhttp/releases) 中下载所在平台对应的版本。

2. 运行go-cqhttp一次, 生成启动脚本 go-cqhttp.bat， 再通过启动脚本运行一次， 启用功能部分选择 `正向 WebSocket`, 此时将生成配置文件 config.yaml。

3. 编辑配置文件根据注释提示填写中如下字段：
    + account:uin， account:password   
    你想要挂载的机器人的账号和密码   
    + message:post-format  
    修改此项的值为 array   
    + servers: - ws: port   
    此项为你需要使用的端口号    

5. 拷贝 example 目录下 HelloService.hpp 到 src/service 下， 并修改 Ayan.h 文件， 在文件末尾添加：
~~~ c++
    #include "service/HelloService.hpp" 
~~~ 
修改 Ayan.cpp, 其中注释了 diff 字样的语句为新加入/修改的语句：

~~~ c++
#include "Ayan.h"

using namespace ayan;

int main(int argc, char **argv)
{
	std::system("chcp 65001 & cls");     

	auto env = Env::from()
				   .with_name("Global")
				   .with_thread_num(1)
				   .init();

    // diff-1
    env->supply<HelloService, true>();

	auto bot = Bot::from(env)
				   .connect("127.0.0.1", "6700")    // diff-2
				   .with_name("Ayan")
				   .init();

	bot->start();

	block_here();
	return 0;
}
~~~
注意， diff-1 表明注册并为机器人订阅该服务, diff-2 处的 ip, port 字段即为刚刚 协议适配器 正向ws 所监听的地址和端口号。

6. 启动 go-cqhttp, 使用以下语句重新编译和启动 Ayan 

~~~
    $ xmake 
    $ xmake -run
~~~

你将会在 Ayan 的控制台中看到如下绿色字样

~~~
    [20xx-xx-xx hh-mm-ss] [Ayan] [DEBUG]: Welcome!, now is ...
~~~

此时 Ayan 启动完毕且服务加载成功。

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
    1. 在解释器 drop 后（也即任何一个 Env 中都已卸载 PythonInterface 服务后）， c++ 中能不存在任何对 python 对象的引用， 否则会 crush , 请保证对象的生命周期短于解释器的生命周期。
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
