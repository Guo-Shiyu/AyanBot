# Some 

多个bot共用一个environment

~~~ cpp
struct Environment
{
    ThreadPool* _tp;
    std::map<Event>
}
~~~

# Expect



~~~ cpp

using MsgView = const Message&;

template<typename T>
struct BaseTrigger
{
    bool touch(MsgView msg);    
    
};

template<typename T>
using Trigger = BaseTrigger<T>;

struct HelloTrigger : public Trigger<HelloTrigger>
{

};


int main() 
{
    auto bot = AyanBot::from<NetAddress>("127.0.0.1", "6700");
    bot.registe_trigger<HelloTrigger>();

}

~~~