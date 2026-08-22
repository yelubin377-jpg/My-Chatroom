#include "Router.h"
#include <muduo/base/Logging.h>


void Router::on(uint16_t type , MsgHandler handler) // 注册阶段，只允许添加，不允许删除，乱搞我map _handlers的东西
{
    _handlers[type] = std::move(handler);
}
void Router::dispatch(uint16_t type,
                      const muduo::net::TcpConnectionPtr& conn,
                      const MyProtoMsg& msg,
                      void* ctx)
{
    auto it = _handlers.find(type);
    if(it != _handlers.end())
    {
        it->second(conn,msg,ctx); //三链问：谁发的？发了什么？发给谁（服务器）？
    }
    else
    {
        LOG_INFO << "Router couldn't find type or type's handler boom!" << (unsigned)type;
    }
}