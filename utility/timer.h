#pragma once

#include <string>
#include <ctime>

struct Timer
{
    std::string getTimestamp()
    {
        std::time_t time_now_epoch = std::time(nullptr);
        std::tm time_now = *std::localtime(&time_now_epoch); 
        char buffer[22];
        std::strftime(buffer, sizeof(buffer), "[%d-%m-%Y %H:%M:%S]", &time_now);
        std::string answer = std::string(buffer, 22);
        answer.pop_back();
        return answer;
    }
    
    unsigned int getTime()
    {
        std::time_t time_now_epoch = std::time(nullptr);
        return time_now_epoch;
    }
};