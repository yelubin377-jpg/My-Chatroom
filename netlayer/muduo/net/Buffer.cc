#include "Buffer.h"
#include <string>
#include <vector>
#include <stddef.h>
#include <sys/uio.h>
#include <errno.h>
#include <algorithm>

using namespace muduo::net;

size_t Buffer::readableBytes() {return _writerIndex - _readerIndex;}
size_t Buffer::writableBytes() {return _buffer.size() - _writerIndex;}
size_t Buffer::prependableBytes() {return _readerIndex;}

const char* Buffer::peek() const //找开头
{
    return &*_buffer.begin() + _readerIndex;
}
void Buffer::retrieve(size_t len)
{
    assert(len <= readableBytes());
    _readerIndex += len;
}//上2-读







//写
char* Buffer::beginWrite()
{
    return &*_buffer.begin() + _writerIndex;
}
void Buffer::hasWritten(size_t len)
{
    _writerIndex += len;
}


void Buffer::ensureWritable(size_t len)
{
    if (writableBytes() < len)
    {
        if (writableBytes() + prependableBytes() < len + CheapPrepend) //预留+已消费+可写和起来不够
        {
            _buffer.resize(_writerIndex + len);
        }
        else
        {
            size_t readable = readableBytes();
            std::copy(peek(), peek() + readable,
                      &*_buffer.begin() + CheapPrepend);
            _readerIndex = CheapPrepend;
            _writerIndex = _readerIndex + readable;
        }
    }
}
void Buffer::append(const char* data, size_t len) //重载
{
    ensureWritable(len);
    std::copy(data, data + len, beginWrite());
    hasWritten(len);
}
void Buffer::append(const void* data, size_t len)
{
    append(static_cast<const char*>(data), len);
}






void Buffer::retrieveAll()
{
    _readerIndex = CheapPrepend;
    _writerIndex = CheapPrepend;
}

void Buffer::retrieveUntil(const char* end)
{
    assert(peek() <= end);
    assert(end <= beginWrite());
    retrieve(end - peek());
}

std::string Buffer::retrieveAsString(size_t len)
{
    std::string result(peek(), len);
    retrieve(len);
    return result;
}






ssize_t Buffer::readFd(int fd, int* savedErrno)
{
    char extrabuf[65536];
    struct iovec vec[2];
    const size_t writable = writableBytes();
    vec[0].iov_base = beginWrite();
    vec[0].iov_len = writable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);

    const ssize_t n = ::readv(fd, vec, 2);    // v=vector
    if (n < 0)
    {
        *savedErrno = errno;
    }
    else if (static_cast<size_t>(n) <= writable)
    {
        _writerIndex += n;
    }
    else
    {
        _writerIndex = _buffer.size();
        append(extrabuf, n - writable);
    }
    return n;
}
