# Service System 
Service System 是 Ayan 的核心系统， 其性质和工作过程（甚至是实现）都与 llvm 中 Pass Manager Module 有相近之处。

## 事件推送与事件类型 
OneBot 协议适配器会通过 WebSocket 向 Ayan 推送事件， 事件在 Ayan 中以 variant(见 event.h) 的形式存储， 利用模式匹配(见 fwd.h 中的 Overloaded)进行服务与事件类型的匹配， 保证服务只处理自身类型一致的事件。   
事件包括四大类： Message、Notice、Request、Meta。 已经在 event.h 中定义。

## 事件处理流程   
在机器人接收到一个事件后， 会按照服务名称的字典序依次调用与事件类型一致的已订阅服务， 具体到单个服务，则为：

1. 判断是否触发该服务， 接口为 trig()
2. 若返回为 true， 则调用 act() 接口

详见 service.h UniverseService 类 handle 函数

## 服务链与服务属性    
服务有自身的属性， 通常为 bool 值， 表明服务的某些特性， 例如 Consumptive 是否会在服务完毕后消耗掉该事件.

在交由 Service System 处理之前， 会将事件包装为 std::optional\<Event> 的形式，若服务中 Consumptive 一项为 true， 则会将 option 对象置为空， 并不再继续调用后续 service 的 trig 与 act 函数， 即终止该事件链。 

此外服务还有其他属性， 这些属性都与服务的一些接口有关 例如：
+  可关闭 
+  默认开启状态

开启/关闭服务相关逻辑位于 UniverseService 类 attempt 函数中 

## Service System 的实现

service system 的实现使用了 CRTP 继承, 这与 llvm 中的 New Pass Manager System 是一致的。  
其中凡是继承自 ServiceConcept 类的类（也即符合服务概念的类）都可以以指针的形式被加载到 Bot 与 Env 的 Service 列表中。也会被依次触发。  
默认的四种服务性质类似于 llvm 中的 三种 Pass， 只是满足 ServiceConcept 要求的 一种情况， UniverseService 只是抽象了四种服务，进一步规范了四种服务的共同行为。

同时处理多种事件的 MixinService 类则是 Mixin 继承自四种服务类，与普通服务除服务类型更多外并无区别。  

关于 Service 的用户接口， 详见 UserManual.md 中 Service 一节。
