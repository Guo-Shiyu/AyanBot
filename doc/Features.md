# 易于使用   
1. 链式 api 调用
~~~ c++
    // 创建复杂的  Env 对象 

    auto env = Env::from()
        // 线程池大小为 4
        .with_thread_num(4)     
        
        // 命名环境为 Global
        .with_name("Global")    
        
        // 所有使用该环境的 bot 都将该账号视为管理员权限
        .with_admin(987654321)  
        
        // 重定向该环境中所有bot的日志流
        // 到文件 GlobalLog.txt 中
        .redirect_log(new std::ofstream{ "GlobalLog.txt" }) 
        
        // 即便不调用上述方法修饰，直接调用 init 
        // 也会得到默认设置的 Env 对象
        .init();

    static_assert(std::is_same<decltype(env), std::shared_ptr<Env>>())


    auto bot = Bot::from(env)
        // 连接到 onebot 协议适配器的 WebSocket Server 
        .connect(127.0.0.1, 6700)

        // 命名， 可以不命名将使用默认名 Ayan
        .with_name("Exp Bot")

        // 重定向自身的错误流到 cout, （默认为 cerr）
        .redirect_err(&std::cout)
        .init();

    static_assert(std::is_same<decltype(bot), std::shared_ptr<Bot>>())

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
    
2. 现代的字符串格式化工具

~~~ c++
    // 接上述代码
    {
        auto& bot = *bot;
        int i32 = 137;
        double f64 = 137.036;
        std::string str = "woc";
        std::string_view sv = time_now();

            // 在 GlobalLog.txt 中输出
        bot.log("{0} {1}!", "hello", "world")  

            // 在 cout 流中以绿色输出 
           .dbg("{}? {}!", i32, f64)    

           // 在 cout流中以红色输出    
           .err("{1}... {0}?", sv, str);        
    }
~~~ 
3. 面向 variant 进行（伪）模式匹配
~~~ c++

    using ADT =std::variant<int, size_t,std::string>;
    std::vector<ADT> segments ={-1, 137, "abc"};
    
    // Overloaded 定义见 fwd.h
    Overloaded match{
        [](int& val) 
    {
        std::cout << "int: " << val << std::endl;
    },
        [](size_t& val)
    {
        val += 1， val *= 2;
        std::cout << "size_t: "<< val << std::endl;
    },
        [](std::string& str)
    {
        std::cout << "str: " << str << std::endl;
    }};
    
    // pattern match
    for(auto& seg: segments)
        std::visit(match, seg);
    // 将会输出：
    // > int: -1
    // > size_t: 276
    // > str: abc

~~~ 

5. Ayan 集成了丰富的可能用到的基础工具组件。  
例如定时器接口：
~~~ cpp
    using namespace hv;
    
    // 十秒后执行的超时定器
    bot->set_timeout(10* 1000, [=](TimerIDtimer)
    {
        bot->send_privae_msg({qqid,msg});
    });

    // 每分钟执行一次的重复定时器
    using std::chrono_literias;
    bot->set_interval(count_ms(1min), [=](TimerID timer)
    {
        // do something
    });
    
    // 重置定时器以及取消时器
    // 设置一个 1h5m7s 启动的定时器, 

    TimerTrace trace = bot->set_timeout(
        accmulate_ms(1h,5min, 7s), 
        [=](auto_)
        {
            bot.dbg("-,-");
        });
    
    // 一段时间后重新设置定时器, 需要重新等待1h5m7s
    bot->reset_time(trace);
    
    // 取消上述定时器
    bot->kill_time(trace);
~~~    
6. 丰富的 API 支持 (TODO)   

    所有 onebot 11 标准在列的 API 都将会被支持， 但尚未全部实现。   
    
    而所有 onebot 12 标准的 API 都在支持计划中， 实现的时间待定。


# 异步 API 调用   

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

        // 调用future::get() 方法， 
        // 将会阻塞线程直到得到 api 调用的返回值
        MsgId id = fu_id.get();
    }

    // 基于 callback 的异步
    {
        // 同时发送群消息和私聊消息， 
        // 拿到发送完的消息的 id 输出在日志
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
