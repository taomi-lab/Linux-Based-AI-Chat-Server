#ifndef CHATBOT_H
#define CHATBOT_H


#include <iostream>
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Bot {
public:
    Bot(){}

    std::string getChatResponse(const std::string& question, const std::string& model);

private:
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }
};

#endif
