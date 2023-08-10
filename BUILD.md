# Build   
Ayan 使用 [xmake](https://xmake.io/#/) 构建     

以下操作需要系统中安装 xmake. 
  
## 1. clone 仓库, 并附带更新所有 submodule   
~~~
$ git clone https://github.com/Guo-Shiyu/AyanBot.git  
$ cd AyanBot  
$ git submodule update --init --recursive 
~~~

## 2. 配置自定义的编译选项并构建  
查看 xmake.lua, 根据自己的需要选择工具链, 输出目录, 和扩展接口.    
之后也可以随时修改该文件来重设配置.   

~~~
# (可选) 切换到 debug 构建, 默认是 release 
$ xmake f -m debug  

# 构建
$ xmake 
~~~ 

注意， 如果是第一次构建, 需要先生成依赖 `hv`, `fmt`,  在命令框中选择 `Y`  或使用 `xmake -y` 代替上述构建命令即可.

更多 xmake 的功能参见 xmake 的官方文档.    

## 4. 运行    
  在默认配置下, build 目录下的 app 目录中包含简单的 example,  lib 目录下包含构建出的库文件.     
 传入 Onebot 协议适配器工作的 ip 和 端口号即可开始工作.    
 ~~~
 $ xmake r HelloAyan <Onebot WS Server IP> <Port>  
 # e.g. xmake r HelloAyan 127.0.0.1 15698
 ~~~
 如果一切顺利, 将会在命令行中看到  

 ~~~
 hello ayan!   
 ~~~

## 5. 配置自己的账号和机器人名字  
修改 src/main.cpp 中的配置, 即可实现自己的机器人   