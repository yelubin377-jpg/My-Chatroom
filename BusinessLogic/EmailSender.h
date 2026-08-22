#pragma once
#include <string>

class EmailSender
{
public:
    EmailSender(const std::string& fromEmail , const std::string& authCode);
    bool send(const std::string& to , const std::string& subject , const std::string& body);
private:
    std::string _fromEmail;
    std::string _authCode;
    static size_t readCallback(char* buf , size_t size , size_t nemb , std::string* data);
};