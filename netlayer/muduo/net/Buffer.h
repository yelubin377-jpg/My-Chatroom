#pragma once
#include <vector>
#include <assert.h>
#include <string.h>
#include <unistd.h>

#include <string>
#include <cstring>

namespace muduo
{
namespace net
{

class Buffer
{
public:
    static const size_t CheapPrepend = 8;
    static const size_t InitialSize = 1024;
    
    Buffer()
: _buffer(CheapPrepend + InitialSize)
, _readerIndex(CheapPrepend)
, _writerIndex(CheapPrepend){};
    size_t readableBytes();
    size_t writableBytes();
    size_t prependableBytes();
    const char* peek() const;
    void retrieve(size_t len);
     char* beginWrite();
    void hasWritten(size_t len);
     void ensureWritable(size_t len);
    void append(const char* data , size_t len);
    void append(const void* data , size_t len);
    void retrieveAll();
    void retrieveUntil(const char* end);
    std::string retrieveAsString(size_t len);
   
    ssize_t readFd(int fd , int* savedErrno);

private:
    std::vector<char> _buffer; 

    size_t _readerIndex;
    size_t _writerIndex;
};}}
