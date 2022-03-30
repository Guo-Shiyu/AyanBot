# Echo 
Echo 服务将会原封不动地回复任何私聊消息， 无论是文本、图像、还是表情或语音。 

声明 Echo Service 的代码如下：

~~~ c++
#pragma once

#include "../core/service.h"
#include "../core/apicall.h"

class EchoService : public MessageService<EchoService>
{
public:
	std::string_view name() const override
	{
		return "Echo Service";
	}

	bool trig(Bot& bot, PrivateMessage& msg);
	void act(Bot& bot, PrivateMessage& msg);
};
~~~

比起 [HelloService](1-Hello.md), Echo 的定义多了 trig 与 act 两个函数。这是因为 Service System 的处理流程需要的。  

假定一个 bot 注册了多种服务， 这些服务生效的时间点除了之前介绍的加载时（on_load）, 卸载时（on_unload）以外， 还包括每次收到对应类型 [事件](https://github.com/botuniverse/onebot-11/tree/master/event) 时被触发。  

EchoService 继承自 MessageService, 表明该类处理 Message Event, 于是它会自动忽略其他事件， 并且对每一个 Message Event 调用 trig 方法， 若 trig 方法返回为 true, 则会接着以该事件为参数调用 act 方法。 由于 两个函数的参数都以引用传递， 因此 trig 和 msg 的参数是同一个事件对象。

设置 trig 函数的目的在于， 分离条件动作的触发条件和触发动作。由于我们需要对所有消息都回显， 并没有过滤条件， 因此 trig 函数设置为固定返回 true 即可。

在这个例子中， 我们的动作是将消息原封不动的送回发送者， 所以 act 函数应当为：  
~~~ c++
    void act(Bot& bot, PrivateMessage& msg)
	{
		bot.api().send_private_msg(msg.sender.user_id, msg.message);
	}
~~~

PrivateMessage 代表着所有的私聊消息， 可能来自陌生人/好友/群临时会话。如果需要同样回显所有的 群消息， 那么不需要再定义额外的类，因为 group message 的 trig、 act 函数也在 MessageEvent 接口中。 所以在类中额外定义如下两个函数：  

~~~ c++
    bool trig(Bot& bot, GroupMessage& msg)
	{
		return true;
	}

    void act(Bot& bot, GroupMessage& msg)
	{
		bot.api().send_group_msg(msg.group_id, msg.message);
	}
~~~

这样就完成了对群消息的回显。  

注意： 只有 Message Event 的接口下细分出了 Private / Group 两种类型， 因为后者比前者多携带群信息/群成员信息等字段， 而处理其他 Event 的 Service 则没有再向下细分， 均直接以 variant 呈现出所有的可能情况。  

处理其他事件， 请看接下来的 [Request](3-Request.md) [Notice](4-Notice.md) [Meta](5-Meta.md) 

想要同时处理多种事件， 请看 [MixinS](6-Mixin.md)  
