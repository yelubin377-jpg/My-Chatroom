#include "AIclient.h"
#include <curl/curl.h>
#include <json/json.h>

size_t AIclient::writeCallback(void* contents , size_t size , size_t nemb , std::string* output)
{
    size_t totalSize = size * nemb;
    output->append((char*)contents,totalSize);
    return totalSize;
}

AIclient::AIclient(const std::string& apiKey)
    : _apiKey(apiKey)
{
    curl_global_init(CURL_GLOBAL_ALL);

}

std::string AIclient::chat(const std::string& userMessage)
{
    CURL* curl = curl_easy_init();
    if(!curl) return "";

    std::string responseStr;
    curl_easy_setopt(curl , CURLOPT_URL , "https://api.deepseek.com/v1/chat/completions");
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers,"Content-Type: application/json");
    std::string authHeader = "Authorization: Bearer "+ _apiKey;
    headers = curl_slist_append(headers,authHeader.c_str());
    curl_easy_setopt(curl , CURLOPT_HTTPHEADER , headers);
    Json::Value requestBody;
    requestBody["model"] = "deepseek-chat";
    requestBody["stream"] = false;
    Json::Value message;
    message["role"] = "user";
    message["content"] = userMessage;
    requestBody["messages"].append(message);
    Json::FastWriter writer;
    std::string jsonBody = writer.write(requestBody);
    curl_easy_setopt(curl,CURLOPT_POSTFIELDS,jsonBody.c_str());
    curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,writeCallback);
    curl_easy_setopt(curl,CURLOPT_WRITEDATA,&responseStr);
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);

    if(res != CURLE_OK) return "";

    Json::Reader reader;
    Json::Value resp;
    if(!reader.parse(responseStr , resp)) return "";
    return resp["choices"][0]["message"]["content"].asString();
}

