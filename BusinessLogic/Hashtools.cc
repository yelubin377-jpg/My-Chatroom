#include "Hashtools.h"

#include <string>
//#include <random>
#include <openssl/rand.h> 
#include <cstdio>
#include <openssl/sha.h>
std::string generateSalt()
{

    //std::random_device rd;
    //std::mt19937 gen(rd());
    //std::uniform_int_distribution<> dis()
    unsigned char salt[16];
    RAND_bytes(salt,sizeof(salt));
    char hex[33];
    for(size_t i = 0;i < 16; i++)
    {
        snprintf(hex + i*2 , 33 - i*2 , "%02x" , salt[i]);
    }
    hex[32] = '\0';
    // snprintf(目标写几个字位置 ， 还能节 ， "%02x" , 当前字节)   总共需要33个   32+1
    return std::string(hex);  
    // 教训：不能直接返回char值，char hex[33]是栈上变量，函数一结束就会消失
    //，没办法把生成的盐值传递过去, 往存在堆上的string 上面复制一份
    //为什么一开始要用栈 ， 是因为栈直接自动管理内存不用我手动去管 ，尽管std::string也是自动管理de

}

std::string sha256(const std::string& password , const std::string& salt) //SHA256(输入数据指针 ， 数据长度 ， 输出缓冲区)
{
    std::string together = password + salt;
    unsigned char middle[32]; //中间过渡，存SHA转化的
    SHA256((const unsigned char*)together.data(),together.size(),middle);          //返回 char*，SHA256 要 const unsigned char* 
    //char HashWord[100];
    //char HashWord[32];
    char HashWord[65];
    //永远输出32字节 ->32*2+1 = 65hex
    for(size_t i = 0 ; i < 32  ; i++)
    {
        snprintf(HashWord + i*2 , 65 - i*2 , "%02x" , middle[i]);
    }
    return std::string(HashWord);
}