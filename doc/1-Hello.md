        这是本系列的第一篇教程， 在着手之前建议先阅读 [UserManual](UserManual.md) 有关 Service 的部分， 然后开始学习如何给自己的机器人编写自定义的服务。

# 编写第一个 HelloService   
HelloService 将会在被加载到一个 bot 上时， 在日志界面输出 “Welcome! now is ____ ”字样， 其中被下划线省略的部分表示精确到秒的当前日期。 同时给 bot 所有的管理员 QQ 私聊发送与日志同样的消息。  

所有的这些功能都被集中到一个类中. 为了方便我将其组织在一个 .hpp 文件中（当然也可以分离在 .h 和 .cpp 文件中）

~~~ c++
        #pragma once

        #include "../core/service.h"
        #include "../core/apicall.h"

/* 1 */ class Hello : public MessageService<Hello>
        {
        public:
/* 2 */ 	std::string_view name() const override;

/* 3 */ 	void on_load(Bot& bot);
        };
~~~

声明这样一个服务只需要十行， 其中，include 部分无需解释。 

其中标注了序号的部分 ：    
1. 定义自己的类， 而且这个类需要继承自 以自身为模板参数的 `MessageService` 类。    
现阶段不必关心 `MessageService` 有什么作用， 只要明白： 继承自一些特殊的类， 才能被视为一个可以被加载到 bot 和 env 并生效的 Service。 至于不同的父类有何功能将会在后续说明。    
注： 这样的继承方式称为 CRTP。 在正常情况下， 子类通过继承来扩展父类的功能， 而在 CRTP 情况下，父类通过继承扩展子类的功能, 有关 CRTP 在 Service System 中的更多应用请见[Service System](dev/service_system.md)。 

2. 以固定的格式声明一个必须存在的函数 name, 其返回值将作为 Service System 中服务的标识符， bot 需要通过 这个标识符来 确定加载和卸载服务。   
因此， 应保证这是一个纯函数， 也即每次调用都返回相同的值， 否则机器人可能不会正常工作。   

3. 这个函数将会在服务被加载时调用， 该函数的签名是固定不变的。   

那么 name 函数的实现就应该是:

~~~ c++
    std::string_view name() const override
	{
		return "Hello Service";
	}
~~~

但更好的办法是在 HelloService 类中定义一个静态变量： 
~~~ c++
    constexpr static const char* NAME = “Hello Service”;
~~~

并将其作为 name 函数的返回值， 这样的优点是外部程序可以以  
~~~ c++
    auto name = HelloService::NAME;
~~~
的形式访问到同样的结果， 而不用先得到 HelloService 类的实例或指针。 

对于 on_load 函数， 他的行为是：  
+ 在控制台上显示一条日志   
+ 给所有的管理员发送同样的消息   

于是其实现为：

~~~ c++
    void on_load(Bot& bot)  /* 1 */
	{
		std::string_view now = time_now();      /* 2 */
		bot.dbg("Welcome!, now is {0}", now);
		
		Message hello = std::string{ now };     /* 3 */
		for (Qid super : bot.admins())
		{
			bot.api().send_private_msg(super, hello);   /* 4 */
		}
	}
~~~

1. 由于一个 Serivce 可以被多个 bot 加载， 每个 bot 被加载时都会有不同的管理员列表或是其他信息， 因此该函数的参数 bot 就是加载了这个服务的 bot。
 
2. time_now 是 Ayan 的工具函数， 返回以 `yy-mm--dd hh:mm:ss` 形式格式化的当前时间。 返回 std::string_view 是因为大部分时候查看时间都是一个 read-only 的行为， 所以开辟了静态缓冲区而不是每次都申请堆空间构造 新的 std::string。   
紧接着， bot.dbg 将会以第一个参数作为格式化串来格式化后续的参数, 这是 C++20 的新标准库 \<format> 带来的特性， 在尚不支持该特性的平台上， 使用了第三方开源库 [fmt](https://github.com/fmtlib/fmt) 作为替代. 
使用 dbg 函数输出将会得到绿色的 DEBUG 日志（这很显眼）, 如果将 bot.dbg 换做 bot.log 将会得到同样的白色输出, 换做 err 则会得到红色输出.  

3. 通过 std::string 显式地构造一条 Message. 其他的构造方法请查看文档.  

4. 通过一个循环对所有的 管理员发送 这条消息. bot.admins 函数返回一个 std::set\<Qid>, Qid 即他们的 QQ号.  

你可以在类中新定义一个函数, 其签名为
~~~ c++
    void on_unload(Bot &bot);
~~~

这个函数将会在服务被 bot 卸载时调用, 请自行尝试.  

到此为止, 我们的 HelloService 就编写完成了, 但是距离使用 还差最后一步: 服务注册和订阅

~~~ c++
    auto env = Env::from()
                    .init();

    Qid admin = 123456789;

	auto bot = Bot::from(env)
                   .with_admin()
				   .connect("127.0.0.1", "6700")
				   .init();

	bot->start();

    env->supply<HelloService>();
    bot->subscribe("Hello Service");
    // 同样可以写成:
    // bot->subscribe(HelloService::NAME);

~~~

在最后两行中创建 Env 和 Bot 变量, 调用 env 的 supply 方法完成注册, 调用 bot 的 subscribe 方法完成订阅, 编译运行, 你就可以在 admin 的 QQ 和日志上同时看到 Welcome 字样了.

在注册这一步, 你可以写成如下形式:
~~~ c++
    env->supply<HelloService, true>();
~~~

第二个模板参数为 true 表明这个服务将默认为所有该 env 中的 bot 订阅而无需再手动调用 subscribe 方法. 

如果有些服务需要为不同的 env 提供不同的构造参数, 那么只需要在 supply 方法的参数中依次填入即可, 这些构造参数将被转发至服务的构造函数中.  

## 总结   
自定义 Service 就是新建一个类, 但有着以下要求:
1. 继承自某些特殊的类， 不同的父类意味着处理不同种类的消息.  
2. 这个类中必须提供一个名为 name 的纯函数.  
3. 视个人需要, 声明一些其他有着固定签名的函数, 这些函数分别会在特定的时机被 Service System 自动地调用. 当不需要其他服务时, 可以忽略.  

满足了这些要求的类就是一个 Service, 可由 Env 提供并由 bot 订阅, 只有订阅后的bot 才能够生效.

