#include <iostream>
#include <ctime>
#include <unistd.h>

struct Timer
{
    std::string getTimestamp()
    {
        std::time_t time_now_epoch = std::time(nullptr);
        std::tm time_now = *std::localtime(&time_now_epoch); 
        char buffer[24];
        std::strftime(buffer, sizeof(buffer), "[%d-%m-%Y %H:%M:%S]", &time_now);
        return std::string(buffer, 24);
    }
    
    int getTime()
    {
        std::time_t time_now_epoch = std::time(nullptr);
        return time_now_epoch;
    }
};