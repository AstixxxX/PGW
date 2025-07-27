#include <httplib.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>
#include <unistd.h>

using json = nlohmann::json;

void check_subscriber(httplib::Client& cli, const std::string& imsi) 
{
    json request_data = {
        {"imsi", imsi}
    };
    
    auto res = cli.Post("/api/check_subscriber", request_data.dump(), "application/json");
    
    if (res && res->status == 200) 
    {
        try 
        {
            auto response_json = json::parse(res->body);
            std::string active = response_json["activity"].get<std::string>();
            std::cout << "This session is " << active << std::endl;
            
        }
        catch (const std::exception& e) 
        {
            std::cerr << "JSON parse error: " << e.what() << std::endl;
        }
    }
    else 
    {
        if (res) 
        {
            std::cerr << "HTTP error: " << res->status << std::endl;
            std::cerr << "Response: " << res->body << std::endl;
        }
        else 
        {
            std::cerr << "Request failed" << std::endl;
        }
    }
}

void stop_server(httplib::Client& cli) 
{
    auto res = cli.Post("/api/stop", "", "application/json");
    
    if (res && res->status == 200) 
        std::cout << "Server stopped successfully" << std::endl;
    else 
    {
        if (res) 
        {
            std::cerr << "HTTP error: " << res->status << std::endl;
            std::cerr << "Response: " << res->body << std::endl;
        }
        else 
            std::cerr << "Request failed. Server is busy" << std::endl;
    }
}

int main() 
{
    httplib::Client cli("http://localhost:8080");

    while (true) 
    {
        std::cout << "WEBPANEL" << std::endl;
        std::cout << "1. Check subscriber" << std::endl;
        std::cout << "2. Stop server" << std::endl;
        std::cout << "3. Exit from webpanel" << std::endl;
        std::cout << "Choose option: " << std::endl << std::endl;
        
        int choice;
        std::cin >> choice;
        
        if (choice == 1) 
        {
            std::system("clear");
            std::string imsi;
            std::cout << "Enter IMSI: ";
            std::cin >> imsi;
            check_subscriber(cli, imsi);
        }
        else if (choice == 2) 
        {
            std::system("clear");
            stop_server(cli);
            break;
        }
        else if (choice == 3) 
        {
            std::system("clear");
            break;
        }

        sleep(3);
        std::system("clear");
    }
    
    return 0;
}