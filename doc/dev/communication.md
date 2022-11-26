# 与 OneBot 协议适配器的通信
OneBot 协议适配器作为 WebSocket 服务端，每个 Bot 实例作为客户端， 持有一条与服务端的连接。   

## 建立连接 
在通过 BotBuilder 构造 Bot 时，OneBot 适配器的地址通过以下函数传入：

~~~ C++
    // class BotBuilder：
    BotBuilder &connect(std::string_view &&ip, std::string_view &&port);
	BotBuilder &connect(std::string_view &&url);    // url = "ws://127.0.0.1:6700/"
~~~

建立连接的过程延迟到 调用 Bot::start() 后， 在 Bot 构造后调用 start() 函数前， 并没有建立连接.

## 释放连接

WebSocket 连接是以 channel 的形式提供的, 而 Bot 中 对外提供 channel 的接口如下:

~~~ C++
    // class Bot:
    std::shared_ptr<hv::WebSocketChannel> &channel();
~~~

也就是当程序中任意一处还持有该 shared_ptr, 那么连接就不会被释放, 即便对应 bot 已经被销毁.


## 通信(被动)
在 bot 每次接收到 WebSocket 的数据包时, 都会假定这是一个完整的包(即可以被解析为完整 json 的 packet),  并以此完整包调用 Bot::on_message 函数, 因此在网络情况不佳的情况下可能出现包不完整导致 Ayan 不能正常工作.   

当调用一次 on_message 包时, 如果判断该包为 调用 OnrBot API 的返回结果, 那么转交 api-call 模块处理(见下文 通信(主动)). 

如果判断该包为正常的 事件推送, 那么在 parse 成对应事件结构体后转交 service-system 模块处理.

## 通信(主动) api-call  
主动通信即为在 WecSocket 连接上主动发送一个特定格式的数据包. 用户可以获得 raw channel, 但不建议自行发送消息, 而是调用 Bot::api() 函数获取一个 ApiWarpper 对象进行操作.   

+ ApiWarpper 是对 ApiCall 的进一步封装, 提供了 OneBot API 级别的接口, 根据参数自动填充数据包的字段. 

+ ApiCall 是一个较为低级的, 数据包级别的对主动通信的封装, 其内部持有一个 channel 指针和一个 json 数据, 提供了数个接口来填充对应的字段.  

不建议用户使用除 ApiWarpper 外其他任何通讯手段.   

在一个 API 调用过程中, 首先生成 ApiWarpper 对象, 调用其接口(见 apiimpl.h ), 在数据包填充完毕后通过 WebSocket 推送到 OneBot 协议适配器. 调用返回的结果将被接口中传入的 hook 参数使用, 或是在无 hook 参数时被丢弃. 这一切都是自动完成的, 用户只需要在 API 调用过程中传入参数和回调函数（非必须）即可.

