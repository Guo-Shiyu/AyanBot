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

# 概览  
Ayan 中的核心概念包括以下几点：
+ `Env`    
    Env 中集成了不同机器人中公用部分，包括用于完成异步、定时任务的线程池，管理员权限的控制以及对各种`Service`和日志流的管理。 

+ `Bot`   
    每个 Bot 持有一条 WebSocket 连接并与对应的与 onebot 协议适配器进行交互， 向上提供 onebot API. 通过订阅 `Env` 中的 `Service` 进行个性化的服务配置。

+ `Service`   
    Service 负责完成各种具体功能， 不同种类的 Service 只会响应自己负责的 `Event`, 彼此不会干扰。 Service 通过实现不同的 [Service Trait](doc/dev/service_system.md) 来控制自身加载时、响应时、甚至空闲时的行为。 Service 只会加载于 Env 中， 被 Bot 订阅后才会在 Bot 上生效。    

除此之外， 还有一些概念基于已有的 [onebot](https://github.com/botuniverse/onebot-11) 标准， 例如 Event, Message，在 Ayan 中也有同样的封装。

# 文档  
文档参见 [Doc](doc/README.md)  

交流QQ群：933327998   

# 特性

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
    
    借助于辅助类对 variant 进行 伪*模式匹配
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

    链式 api 调用  
    请见 `异步`

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
        // 和 future 异步 api 的区别仅仅是将 api 所需的参数打包成 tuple, 并额外传入一个 callback 参数

        // 当然也可以仅仅是为了使用链式调用而不传入回调参数， 因为 api 提供了什么都不做的默认参数
        bot.api().send_group_msg({ group, msg })
           .api().send_private_msg({ friend_, msg });
        
        // 或是发送消息的同时执行耗时的操作
        auto on_ret = [](auto& call_result)
        {
            std::ofstream file { "C:\\local\\FriendList.txt" };
            for(auto& fd: call_result)
            {
                const char* fmt = "id:{} name:{}\n";
                std::string line = std::format(fmt, fd.user_id, fd.nickname);  
                file << line;   // 耗时操作
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

    此外， bot 修饰了定时器接口：
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
这主要归功于开源网络库 [libhv](https://github.com/ithewei/libhv) 提供的各种基础组件，其次 Ayan 也封装了一些更为常用的工具， 位于 `src/core/utility` 中， 关于如何使用工具， 参见 [test/utility]().  
    + libhv 的 [接口手册](https://blog.csdn.net/GG_SiMiDa/article/details/103976875), [使用教程](https://hewei.blog.csdn.net/article/details/113733758?spm=1001.2014.3001.5502)

+ `脚本语言接口`    
    Ayan 提供了两种脚本语言的接口: python & lua, 均默认不开启。   
    参见 [开启脚本语言接口](doc/README.md/#开启脚本语言接口)

    ## python   
    通过 pybind11 绑定到外部 python 解释器，支持引入 python 外部库 
    
    ## Lua
    通过 sol3 嵌入 lua 解释器, 支持所有的 lua 标准库.
    

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

