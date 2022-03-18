# AyanBot

Ayan 是基于 Modern C++ 编写的轻量，异步，易于扩展QQ机器人框架。其底层与实现了 [OneBot-v11](https://github.com/botuniverse/onebot-11) 标准的协议适配器进行交互， 为上层实现需求的开发者提供易于使用的接口。  


# 设计目标  
+ 易于开发和部署  
    使开发者能专注于功能的实现，使用者能方便地进行功能管理和权限控制， 最终达到任何用户都能灵活便捷地满足个人需求的效果。  

    如果你想快速部署自己的 Bot， 参见 [快速开始]()  
    如果你想开发个性化的的功能， 参见 [服务样例]()， [接口手册]()

+ 轻量高效    
    无论是开发者还是使用者均不必为他们不需要的部分付出任何额外的心智成本， 或是时间空间上的代价， 一切都是“零开销抽象”的。   

    你可以查看 [开发者文档]() 来了解 Ayan 是如何实现这一目标的。  

# 概览  
Ayan 中的核心概念包括以下几点：
+ `Env`    
    Env 中集成了不同机器人中公用部分，包括用于完成异步、定时任务的线程池，管理员权限的控制以及对各种`Service`和日志流的管理。 

+ `Bot`   
    每个 Bot 持有一条 WebSocket 连接并与对应的与 onebot 协议适配器进行交互， 向上提供 onebot API. 通过订阅 `Env` 中的 `Service` 进行个性化的服务配置。

+ `Service`   
    Service 负责完成各种具体功能， 不同种类的 Service 只会响应自己负责的 `Event`, 彼此不会干扰。 Service 通过实现不同的 [Service Trait]() 来控制自身加载时、响应时、甚至空闲时的行为。 Service 只会加载于 Env 中， 被 Bot 订阅后才会在 Bot 上生效。    

除此之外， 还有一些概念基于已有的 onebot 标准， 例如 [Event](), [Message]()，在 Ayan 中也有同样的封装。

# 文档  
文档参见 [Doc]()  
以及，交流QQ群：933327998   

# 特性
Ayan 的特性来自于其 Design Goal。 开发者和使用者的体验是最为重要的， 轻量高效是其次重要的。  

+ `Easy-to-use`  
    借助于 Modern C++ 的特性与设计方法， 那些可能被频繁使用的接口都尽量地润色到符合（个人认为）的人体工学。

    例如： 大量使用建造者模式的构造方法
    ~~~ c++
    // 创建复杂的  Env 对象 

    auto env = Env::from()
        // 线程池大小为 4
        .with_thread_num(4)     
        
        // 命名环境为 Global
        .with_name("Global")    
        
        // 所有使用该环境的 bot 都将该账号视为管理员权限
        .with_admin(987654321)  
        
        // 重定向该环境中所有bot的日志流到文件 GlobalLog.txt 中
        .redirect_log(new std::ofstream{ "GlobalLog.txt" }) 
        
        // 即便不调用上述方法修饰，直接调用 init 也会得到默认设置的 Env 对象
        .init();

    static_assert(std::is_same<decltype(env), std::shared_ptr<Env>>)


    auto bot = Bot::from(env)
        // 连接到 onebot 协议适配器的 WebSocket Server 监听地址
        .connect(127.0.0.1, 6700)

        // 命名， 可以不命名将使用默认名 Ayan
        .with_name("Exp Bot")

        // 重定向自身的错误流到 cout, （默认为 cerr）
        // 由于没有重定向日志流， 因此其日志将输出到环境的日志流即 GlobalLog.txt 中
        .redirect_err(&std::cout)
        .init();

    static_assert(std::is_same<decltype(bot), std::shared_ptr<Bot>>)

    auto simple_msg = Message{ "hello!" };

    Message complex_msg = MsgBuilder::from("hi!")
            .text(" hiahiahia ")    // 纯文本
            .text(L"你好")
            .text(u8"β  δ")
            .face(15)               // QQ表情id
            .at(12345678)           // at某个人
            .build();

    Message image_msg = MsgBuilder::from()
            .image_local("C:\\Users\\Picture\1.png")    // 本地图片
            .image_url("https://www.baidu.com/2.jpg")   // 网络图片
            .build();

    // 使用本地文件发送语音消息
    Message record_msg = MsgBuilder::from()
            .record_local("C:\\Users\\Music\1.mp3")
            .build();

    ~~~ 
    
    借助于 format 的字符串格式化， 不仅用于日志， 也可以用于消息。
    ~~~ c++
    // 接上述代码
    {
        auto& bot = *bot;
        int i32 = 137;
        double f64 = 137.036;
        std::string str = "woc";
        std::string_view sv = time_now();

        bot.log("{0} {1}!", "hello", "world")   // 在 GlobalLog.txt 中输出
           .dbg("{}? {}!", i32, f64)            // 在 cout 流中以绿色输出 
           .err("{1}... {0}?", sv, str);        // 在 cout流中以红色输出
    }
    ~~~ 
    
    借助于辅助类对 variant 进行模式匹配
    ~~~ c++
    {
        using ADT = std::variant<int, size_t, std::string>;

        std::vector<ADT> segments = {-1, 137, "abc"};

        // Overloaded 定义见 fwd.h
        Overloaded match{
            [](int& val) 
        {
            std::cout << "int: " << val << std::endl;
        },
            [](size_t& val)
        {
            val += 1;
            val = val * 2;
            std::cout << "size_t: " << val << std::endl;
        },
            [](std::string& str)
        {
            std::cout << "str: " << str << std::endl;
        }};

        // pattern match
        for(auto& seg: segments)
            std::visit(match, seg);

    }
    ~~~

    链式 api 调用请见 `异步`

+ `异步`   
    Ayan 同时提供了基于 future 和 callback 的异步解决方案。

    例如， 在调用 onebot API 时
    ~~~ c++
    Bot bot;    // 假定已经初始化的某个 Bot 实例 

    Qid friend_ = 123456789;    // 某个好友的QQ号
    Qid group = 234567890;      // 某个群的QQ号 

    Message msg = Message("hahahah");

    // 同步地获取 api 调用结果
    {
        // 这个调用将会阻塞整个线程， 直到获取结果
        MsgId sync_id = bot.api()
                .send_private_msg(friend_, msg)
                .get();
    }

    // 基于 future 的异步
    {
        // 这个调用不会阻塞线程
        std::future<MsgId> fu_id = bot.api().send_private_msg(friend_, msg);
        
        //这里继续执行其他任务... 

        // 调用future::get() 方法， 将会阻塞线程直到得到 api 调用的返回值
        MsgId id = fu_id.get();
    }

    // 基于 callback 的异步
    {
        // 同时发送群消息和私聊消息， 拿到发送完的消息的 id 输出在日志
        auto on_finish = [&bot](MsgId id)
        {
            bot.log("Send Message:{} complete", id);
        };

        bot.api().send_group_msg({ group, msg }, on_finish)
           .api().send_private_msg({ friend_, msg }, on_finish);
        // 和 future 异步 api 的区别仅仅是将 api 所需的参数打包成 tuple, 并额外传入一个 callback 参数而已

        // 当然也可以仅仅是为了使用链式调用而不传入回调参数， 因为 api 提供了什么都不做的默认参数
        bot.api().send_group_msg({ group, msg })
           .api().send_private_msg({ friend_, msg });
        
        // 或是发送消息的同时执行耗时的操作
        auto on_ret = [](auto& call_result) // 泛型lambda
        {
            std::ofstream file { "C:\\local\\FriendList.txt" };
            for(auto& fd: call_result)
            {
                const char* fmt = "id:{} name:{}\n";
                std::string line = std::format(fmt, fd.user_id, fd.nickname);  
                file << line;
            }
            file.close();
        }

        bot.api().send_group_msg({ group, msg })
           .api().send_private_msg({ friend_, msg })
           // get_griend_list 不需要任何参数，只传入 callback 即可
           .api().get_friend_list(on_ret);  

        // 由此种方式进行的调用是不保证操作顺序的（真 · 异步）， 因此无法保证 bot 发送私聊消息和群消息的先后顺序
    }
    ~~~ 

    此外， bot修饰了定时器接口：
    ~~~ cpp
        using namespace hv;

        // 十秒后执行的超时定时器
        bot->set_timeout(10 * 1000, [=](TimerID timer)
        {
            bot->send_private_msg({qqid, msg});
            // do something
        });

        // 每分钟执行一次的重复定时器
        using std::chrono_literials;
        bot->set_interval(count_ms(1min), [=](TimerID timer)
        {
            // do something
        });

        // 重置定时器以及取消定时器
        // 设置一个 1h5m7s 后启动的定时器, 返回值为线程池中线程指针与定时器id
        TimerTrace trace = bot->set_timeout(accmulate_ms(1h, 5min, 7s), [=](auto _)
        {
            bot.dbg("-,-");
        })

        // 一段时间后重新设置该定时器, 需要重新等待 1h5m7s
        bot->reset_timer(trace);


        // 取消上述定时器
        bot->kill_timer(trace);
    ~~~
    
+ `高效`    ~~现在将高效列为feature有些为时过早~~   
    由于缺少测试， 目前已知数据为：  
    + 加载全部 example service 无脚本语言接口的情况下， release 版内存占用低于 1mb 
    + 开启 python interface (裸解释器) 将升高 7-10mb (随脚本文件功能而变化)  
    + 每开启一个 lua interface (裸解释器) 将升高 约 1mb (随脚本文件功能而变化)   

+ `丰富的基础组件`   
这主要归功于开源网络库 [libhv](https://github.com/ithewei/libhv) 提供的各种基础组件，其次 Ayan 也封装了一些更为常用的工具， 位于 `src/core/utility` 中， 关于如何使用工具， 参见[test/utility]().  
    + libhv 的 [接口手册](https://blog.csdn.net/GG_SiMiDa/article/details/103976875), [各种使用教程](https://hewei.blog.csdn.net/article/details/113733758?spm=1001.2014.3001.5502)

+ `脚本语言接口`    
    Ayan 提供了两种脚本语言的接口: python & lua, 均默认不开启。
    ## python   
    通过 pybind11 嵌入 python 解释器，支持引入第三方库  
    1. 修改 xmake.lua 中`AYAN_PYTHON_INTERFACE` 为 `true` 并设置 `FOREIGN_INTERPRETER_PATH` 为本地的解释器路径
        （通常为 Anaconda 中 python.exe 所在路径， 以方便引入外部库）
    2. 在任一 Env 中提供此项服务以完成解释器的加载,   bot `无需` 像普通服务一样进行订阅， 即可在任意代码中使用如下接口进行访问
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

        py::eval_file("script.py");
    ~~~

    可以通过查询 pybind11 的[文档](https://pybind11.readthedocs.io/_/downloads/en/latest/pdf/)来获得更多用法.  
    ### 注意： 
    由此种方法开启的解释器仍然受 GIL 的限制， 因此所有代码中使用的解释器均为同一个全局解释器，且版本与外部解释器路径下的版本相同。   
    此外还存在以下限制：  
    1. 在解释器 drop 后（也即任何一个 Env 中都已卸载 PythonInterface 服务后）， c++ 中能不存在任何对 python 对象的引用， 否则会 crush , 请保证对象的生命周期短于解释器的生命周期。
    2. 在多线程并发争夺 解释器所有权 时， 需要自行加锁， Ayan 本身不保证解释器的线程安全。       


    ## Lua
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

# Road Map
+ 跨平台
+ Lua Extension API
+ 更加完善的文档及测试用例
+ Service Market

# Q&A 

+ `Q: 为什么这个框架叫 Ayan？`   
A:  QQ机器人实现任何功能都需要对包括消息在内的各种事件无一遗漏地进行检查并作出反应，与“察言观色”这一成语语义相合。 同时，“阿言” 是一位我很喜欢的动漫角色的昵称， 其绝技名也恰好为 “察言观色”. 因此借人物昵称命名机器人框架为阿言。 

+ `Q: 为什么不向脚本语言中注册 ayan 的接口来使用脚本语言开发服务？`  
A: 设计上， Python 接口用于引入 python 社区中丰富的第三方库， 以完成各种 “未曾设想的功能”而非处理消息回复等逻辑， 因此并未向其中注册 Ayan 的 api 来进行扩展。 如果你想全部使用 python 来完成服务开发，  [nonebot2](https://github.com/nonebot/nonebot2) 是一个更好的选择.  
而 Lua 接口则是专门为了解决编译型语言的不够灵活， 服务开发不够快速敏捷等问题而引入的， 但现阶段 lua extension api 的形式尚未确定, 后续才会完善。

