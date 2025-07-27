#pragma once

#include <fstream>
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

class json_config
{
public:
    json_config(const std::string& config_json_path) : json_path(config_json_path)
    {
        std::fstream json_file(json_path);

        if (!json_file.is_open()) 
            throw std::runtime_error("Failed to open log file: " + json_path);

        json_file.close();
    }

    std::string get_string_from_json(const std::string& key) 
    {
        try 
        {
            std::fstream json_file(json_path, std::ios::in);

            if (!json_file.is_open()) 
                throw std::runtime_error("Could not open file: " + json_path);
            
            json data = json::parse(json_file);

            if (!data.contains(key)) {
                throw std::runtime_error("Key '" + key + "' not found in JSON");
            }

            if (!data[key].is_string()) {
                throw std::runtime_error("Value for key '" + key + "' is not a string");
            }

            json_file.close();
            return data[key].get<std::string>();
        }
        catch (const json::parse_error& e) 
        {
            throw std::runtime_error("JSON parse error: " + std::string(e.what()));
        }
        catch (const json::type_error& e) 
        {
            throw std::runtime_error("JSON type error: " + std::string(e.what()));
        }
    }

private:
    std::string json_path;
};