#include <hiredis/hiredis.h>
#include "redisClient.h"
redisClient :: redisClient(const std::string& host,int port)
    :_host(host)
    ,_port(port)
    ,_ctx(nullptr)
{
}
redisClient::~redisClient()
{
    if(_ctx)
    {
        redisFree(_ctx);
        _ctx = nullptr;
    }       
}


bool redisClient::connect()
{
    std::lock_guard<std::mutex> lock(_mutex);
    _ctx = redisConnect(_host.c_str(),_port);
    if(_ctx == nullptr || _ctx->err)
    {
        if(_ctx)
        {
            std::cerr<< "Redis error:"<< _ctx->errstr << std::endl;
            redisFree(_ctx); 
            _ctx = nullptr;
        }
        return false;
    }
    return true;
}

//
bool redisClient::set(const std::string& key,const std::string& value)
{
    std::lock_guard<std::mutex> lock(_mutex);
    if(!_ctx) return false;
    redisReply* reply = (redisReply*)redisCommand(_ctx,"SET %b %b",
                                                  key.c_str(),key.size(),value.c_str(),value.size());
    if(!reply || reply->type != REDIS_REPLY_STATUS)
    {
        if(reply) freeReplyObject(reply);
        return false;
    }
    if (reply) freeReplyObject(reply);
    return true;
}



// HSET key field value
bool redisClient::hset(const std::string& key,const std::string& field,const std::string& value)
{
     std::lock_guard<std::mutex> lock(_mutex);
    if(!_ctx) return false;
    redisReply* reply = (redisReply*)redisCommand(_ctx,"HSET %b %b %b",
                                                  key.c_str(),key.size(),field.c_str(),field.size(),value.c_str(),value.size());
    if(!reply || reply->type != REDIS_REPLY_INTEGER)
    {
        if(reply) freeReplyObject(reply);
        return false;
    }
    freeReplyObject(reply);
    return true;
}





//HGET key field
std::string redisClient::hget(const std::string& key,const std::string& field)
{
    std::lock_guard<std::mutex> lock(_mutex);
    if(!_ctx) return "";
    redisReply* reply = (redisReply*)redisCommand(_ctx,"HGET %b %b",
                                                  key.c_str(),key.size(),field.c_str(),field.size());
    if(!reply || reply-> type != REDIS_REPLY_STRING)
    {
        if(reply) freeReplyObject(reply);
        return "";
    }
    std::string result(reply->str,reply->len);
    freeReplyObject(reply);
    return result;
}

bool redisClient::del(const std::string& key)
{
    std::lock_guard<std::mutex> lock(_mutex);
    if(!_ctx) return false;
    redisReply* reply = (redisReply*)redisCommand(_ctx,"DEL %b",
                                                  key.c_str(),key.size());
    if(!reply || reply->type != REDIS_REPLY_INTEGER)
    {
        if(reply) freeReplyObject(reply);
        return false;
    }
    freeReplyObject(reply);
    return true;
}

bool redisClient::hdel(const std::string& key , const std::string& field)
{
    std::lock_guard<std::mutex> lock(_mutex);
    if(!_ctx) return false;
    redisReply* reply = (redisReply*)redisCommand(_ctx,"HDEL %b %b",
                                                    key.c_str(),key.size(),field.c_str(),field.size());
    if(!reply || reply-> type != REDIS_REPLY_INTEGER)
    {
        if(reply) freeReplyObject(reply);
        return false;
    }
    freeReplyObject(reply);
    return true;
}

bool redisClient::exists(const std::string& key)
{
    std::lock_guard<std::mutex> lock(_mutex);
    if(!_ctx) return false;
    redisReply* reply = (redisReply*)redisCommand(_ctx,"EXISTS %b",
                                                    key.c_str(),key.size());
    if(!reply || reply->type != REDIS_REPLY_INTEGER)
    {
        if(reply) freeReplyObject(reply);
        return false;
    }
    int result = reply->integer;
    freeReplyObject(reply);
    return result  == 1;
}

bool redisClient::expire(const std::string& key,int seconds)
{
    std::lock_guard<std::mutex> lock(_mutex);
    if(!_ctx) return false;
    redisReply* reply = (redisReply*)redisCommand(_ctx,"EXPIRE %b %d",
                                                  key.c_str(), key.size(),seconds);
    if(!reply || reply->type != REDIS_REPLY_INTEGER)
    {
        if(reply) freeReplyObject(reply);
        return false;
    }
    freeReplyObject(reply);
    return true;
}

bool redisClient::sadd(const std::string& key ,  const std::string& member)
{
    std::lock_guard<std::mutex> lock(_mutex);
    if(!_ctx) return false;
    redisReply* reply = (redisReply*)redisCommand(_ctx,"SADD %b %b",
                                                  key.c_str(),key.size(),member.c_str(),member.size());
    if(!reply || reply->type != REDIS_REPLY_INTEGER)
    {
        if(reply) freeReplyObject(reply);
        return false;
    }
    freeReplyObject(reply);
    return true;
}

bool redisClient::srem(const std::string& key , const std::string& member)
{
    std::lock_guard<std::mutex> lock(_mutex);
    if(!_ctx) return false;
    redisReply* reply = (redisReply*)redisCommand(_ctx,"SREM %b %b",
                                                    key.c_str(),key.size(),member.c_str(),member.size());
    if(!reply || reply->type != REDIS_REPLY_INTEGER)
    {
        if(reply) freeReplyObject(reply);
        return false;
    }
    freeReplyObject(reply);
    return true;
} 

bool redisClient::lpush(const std::string& key , const std::string& value)
{
    std::lock_guard<std::mutex> lock(_mutex);
    if(!_ctx) return false;
    redisReply* reply = (redisReply*)redisCommand(_ctx,"LPUSH %b %b",
                                                  key.c_str(),key.size(),value.c_str(),value.size());
    if(!reply || reply->type != REDIS_REPLY_INTEGER)
    {
        if(reply) freeReplyObject(reply);
        return false;
    }
    freeReplyObject(reply);
    return true;
}

bool redisClient::ltrim(const std::string& key, int start , int stop)
{
    std::lock_guard<std::mutex> lock(_mutex);
    if(!_ctx) return false;
    redisReply* reply = (redisReply*)redisCommand(_ctx,"LTRIM %b %d %d",
                                                  key.c_str(),key.size(),start,stop);
    if(!reply || reply->type != REDIS_REPLY_STATUS)
    {
        if(reply) freeReplyObject(reply);
        return false;
    }
    freeReplyObject(reply);
    return true;
}

std::string redisClient::get(const std::string& key)
{
    std::lock_guard<std::mutex> lock(_mutex);
    if(!_ctx) return "";
    redisReply* reply = (redisReply*)redisCommand(_ctx,"GET %b",
                                                  key.c_str(),key.size());
    if(!reply || reply->type != REDIS_REPLY_STRING)
    {
        if(reply) freeReplyObject(reply);
        return "";
    }
    std::string result(reply->str,reply->len);
    freeReplyObject(reply);
    return result;
}


bool redisClient::sismember(const std::string& key,const std::string& member)
{
    std::lock_guard<std::mutex> lock(_mutex);
    if(!_ctx) return false;
    redisReply* reply = (redisReply*)redisCommand(_ctx,"SISMEMBER %b %b",key.c_str(),key.size()
                                                                ,member.c_str(),member.size());
    if(!reply || reply->type != REDIS_REPLY_INTEGER)
    {
        if(reply) freeReplyObject(reply);
        return false;
    }
    int result = reply -> integer;
    freeReplyObject(reply);
    return result == 1;
}
bool redisClient::hexists(const std::string& key,const std::string& field)
{
    std::lock_guard<std::mutex> lock(_mutex);
    if(!_ctx) return false;
    redisReply* reply = (redisReply*)redisCommand(_ctx,"HEXISTS %b %b",key.c_str(),key.size(),field.c_str(),field.size());
    if(!reply || reply->type != REDIS_REPLY_INTEGER)
    {
        if(reply) freeReplyObject(reply);
        return false;
    }
    int result = reply-> integer;
    freeReplyObject(reply);
    return result == 1;
}

typedef std::vector<std::string> VectorString;
VectorString redisClient::smembers(const std::string& key)
{
    std::lock_guard<std::mutex> lock(_mutex);
    VectorString result;
    if(!_ctx) return result;
    redisReply* reply = (redisReply*)redisCommand(_ctx,"SMEMBERS %b",key.c_str(),key.size());
    if(!reply || reply->type != REDIS_REPLY_ARRAY)
    {
        if(reply) freeReplyObject(reply);
        return result;
    }
    for(size_t i = 0;i < reply->elements ; i++)
    {
        result.push_back(std::string(reply->element[i]->str,reply->element[i]->len));
    }
    freeReplyObject(reply);
    return result;
}

VectorString redisClient::lrange(const std::string& key , int start, int stop)
{
    std::lock_guard<std::mutex> lock(_mutex);
    VectorString result;
    if(!_ctx) return result;
    redisReply* reply = (redisReply*)redisCommand(_ctx,"LRANGE %b %d %d",key.c_str(),key.size(),start,stop);
    if(!reply || reply->type != REDIS_REPLY_ARRAY)
    {
        if(reply) freeReplyObject(reply);
        return result;
    }
    for(size_t i = 0; i < reply-> elements ; i++)
    {
        result.push_back(std::string(reply->element[i]->str,reply->element[i]->len));
    }
    freeReplyObject(reply);
    return result;
}

int redisClient::llen(const std::string& key)
{
    std::lock_guard<std::mutex> lock(_mutex);
    if(!_ctx) return 0;
    redisReply* reply = (redisReply*)redisCommand(_ctx,"LLEN %b",key.c_str(),key.size());
    if(!reply || reply->type != REDIS_REPLY_INTEGER)
    {
        if(reply) freeReplyObject(reply);
        return 0;
    }
    int len = reply -> integer;
    freeReplyObject(reply);
    return len;
}

std::string redisClient::incr(const std::string& key)
{
    std::lock_guard<std::mutex> lock(_mutex);
    if(!_ctx) return "";
    redisReply* reply = (redisReply*)redisCommand(_ctx,"INCR %b",key.c_str(),key.size());
    if(!reply || reply->type != REDIS_REPLY_INTEGER)
    {
        if(reply) freeReplyObject(reply);
        return "";
    }
    std::string result = std::to_string(reply->integer);
    freeReplyObject(reply);
    return result;
}

