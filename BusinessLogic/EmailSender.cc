#include "EmailSender.h"
#include <curl/curl.h>
#include <cstring>

size_t EmailSender::readCallback(char* buf , size_t size , size_t nemb , std::string* data)
{
    size_t want = size * nemb;
    size_t give = (want < data->size()) ? want : data->size();
    if(give)
    {
        memcpy(buf , data->c_str() , give);
        data->erase(0 , give);
    }
    return give;
}

EmailSender::EmailSender(const std::string& fromEmail , const std::string& authCode)
    : _fromEmail(fromEmail)
    , _authCode(authCode)
{
    curl_global_init(CURL_GLOBAL_ALL); 
}

bool EmailSender::send(const std::string& to , const std::string& subject , const std::string& body)
{
    CURL* curl = curl_easy_init();
    if(!curl) return false;

    curl_slist* rcpt = curl_slist_append(nullptr , to.c_str());

    std::string msg = "To: " + to + "\r\n"
                      "From: " + _fromEmail + "\r\n"
                      "Subject: " + subject 
                      + "\r\n"
                      "Content-Type: text/plain; charset=utf-8\r\n"
                      "\r\n" + body;


    curl_easy_setopt(curl , CURLOPT_URL , "smtps://smtp.qq.com:465");
    curl_easy_setopt(curl , CURLOPT_SSLVERSION , CURL_SSLVERSION_TLSv1_2);
    curl_easy_setopt(curl , CURLOPT_SSL_VERIFYPEER , 0L);
    curl_easy_setopt(curl , CURLOPT_SSL_VERIFYHOST , 0L);
    curl_easy_setopt(curl , CURLOPT_USERNAME , _fromEmail.c_str());
    curl_easy_setopt(curl , CURLOPT_PASSWORD , _authCode.c_str());
    curl_easy_setopt(curl , CURLOPT_LOGIN_OPTIONS , "AUTH=LOGIN");
    curl_easy_setopt(curl , CURLOPT_MAIL_FROM , _fromEmail.c_str());
    curl_easy_setopt(curl , CURLOPT_MAIL_RCPT , rcpt);
    curl_easy_setopt(curl , CURLOPT_UPLOAD , 1L);
    curl_easy_setopt(curl , CURLOPT_READFUNCTION , readCallback);
    curl_easy_setopt(curl , CURLOPT_READDATA , &msg);
    curl_easy_setopt(curl , CURLOPT_INFILESIZE , (long)msg.size());

    CURLcode res = curl_easy_perform(curl);
    if(res != CURLE_OK)//
    {//
        fprintf(stderr, "EmailSender: curl error %d: %s\n", res, curl_easy_strerror(res));//
    }//
    curl_slist_free_all(rcpt);
    curl_easy_cleanup(curl);

    return (res == CURLE_OK);
}