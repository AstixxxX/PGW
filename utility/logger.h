#pragma once

#include <fstream>
#include <string>
#include <iostream>
#include <mutex>
#include "timer.h"

class Logger 
{
public:
    enum class Level 
    {
        INFO,
        WARN,
        ERR,
        CRIT
    };

    Logger(const std::string& log_path) : log_file(log_path, std::ios::app) 
    {
        if (!log_file.is_open()) 
            throw std::runtime_error("Failed to open log file: " + log_path);
        
    }

    ~Logger() 
    {
        if (log_file.is_open()) 
            log_file.close();
    }

    void log(Level level, const std::string& message) 
    {
        std::lock_guard<std::mutex> lock(log_mutex);
        std::string level_str;
        
        switch (level) 
        {
            case Level::INFO:  
                level_str = "[INFO] "; 
                break;

            case Level::WARN:  
                level_str = "[WARN] "; 
                break;

            case Level::ERR:   
                level_str = "[ERR]  "; 
                break;

            case Level::CRIT:  
                level_str = "[CRIT] "; 
                break;
        }

        std::string log_entry = timer.getTimestamp() + " " + level_str + message;
        
        if (log_file.is_open()) 
            log_file << log_entry << std::endl;
    }

    void info(const std::string& message)     { log(Level::INFO, message); }
    void warn(const std::string& message)     { log(Level::WARN, message); }
    void error(const std::string& message)    { log(Level::ERR, message); }
    void critical(const std::string& message) { log(Level::CRIT, message); }

private:
    std::fstream log_file;
    std::mutex log_mutex;
    Timer timer;
};

class CDR_Logger
{
public:
    CDR_Logger(const std::string& log_path) : log_file(log_path, std::ios::app) 
    {
        if (!log_file.is_open()) 
            throw std::runtime_error("Failed to open log file: " + log_path);
    }

    ~CDR_Logger() 
    {
        if (log_file.is_open()) 
            log_file.close();
    }

    void cdr(const std::string& imsi, const std::string& message) 
    {
        std::lock_guard<std::mutex> lock(log_mutex);

        std::string log_entry = timer.getTimestamp() + " IMSI=" + imsi + " CDR_TYPE=" + message;
        
        if (log_file.is_open()) 
            log_file << log_entry << std::endl;
    }

private:
    std::fstream log_file;
    std::mutex log_mutex;
    Timer timer;
};