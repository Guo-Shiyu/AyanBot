# 处理 Request 事件  
Request 事件只包括两类， 加群事件和加好友事件。     
主动添加朋友和群是敏感操作， 将永远不会被支持。

### 特别的， 利用 Overloaded 模板进行模式匹配  

均请见 example/PermitReq.h 