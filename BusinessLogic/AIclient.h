#pragma once
#include <string>

class AIclient
{
public:
    AIclient(const std::string& apiKey);
    std::string chat(const std::string& UserMessage);
private:
    std::string _apiKey;
    static size_t writeCallback(void* contents, size_t size , size_t nemb , std::string* output);
};