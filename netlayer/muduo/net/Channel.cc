#include "Channel.h"
#include "EventLoop.h"
using namespace muduo::net;
void Channel::update()
{
    _loop->updateChannel(this);
}
void Channel::remove()
{
    _loop->removeChannel(this);
}
void Channel::handleEvent()
{

    if (_reallyevents & _YReadEvent)
    {
        if (_readCallback) _readCallback();
    }
    if(_reallyevents & _YWriteEvent)
    {
        if (_writeCallback) _writeCallback();
    }
    if(_reallyevents & EPOLLERR)
    {
       if(_errorCallback) _errorCallback();
    }
        if (_YCloseEvent & _reallyevents) 
    {
        if (_closeCallback) _closeCallback();
        return;
    }

}

