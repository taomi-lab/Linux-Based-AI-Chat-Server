#include "ChatBot.h"

std::string Bot::getChatResponse(const std::string& question, const std::string& model="gpt-3.5-turbo") {
        CURL* curl;
        CURLcode res;
        std::string readBuffer;

        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();
        if (curl) {
            struct curl_slist* headers = NULL;
            std::string apiKey = "sk-X72OT7bYk5rq07zHB192E2B57251482cA4C940C5EdD60f2c"; 
            headers = curl_slist_append(headers, ("Authorization: Bearer " + apiKey).c_str());
            headers = curl_slist_append(headers, "Content-Type: application/json");

            json requestBody = {
                {"model", model},
                {"messages", {
                    {{"role", "user"}, {"content", question}}
                }}
            };
            std::string data = requestBody.dump();

            curl_easy_setopt(curl, CURLOPT_URL, "https://www.blueshirtmap.com/v1/chat/completions");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
            res = curl_easy_perform(curl);

            if (res != CURLE_OK) {
                std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            } else {
                try {
                    json jsonResponse = json::parse(readBuffer);
                    if (jsonResponse.contains("choices") && !jsonResponse["choices"].empty() &&
                        jsonResponse["choices"][0].contains("message") &&
                        jsonResponse["choices"][0]["message"].contains("content")) {
                        std::string content = jsonResponse["choices"][0]["message"]["content"];
                        return content;
                    } else {
                        std::cerr << "Unexpected JSON structure: " << jsonResponse.dump() << std::endl;
                    }
                } catch (json::parse_error& e) {
                    std::cerr << "Failed to parse the JSON response: " << e.what() << std::endl;
                }
            }
            curl_easy_cleanup(curl);
            curl_slist_free_all(headers);
        }
        curl_global_cleanup();
        return "";
    }