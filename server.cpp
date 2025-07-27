#include "udp_server.h"
#include "timer.h"
#include "logger.h"
#include "json-config.h"
#include <thread>
#include <iostream>
#include <string>
#include <bitset>
#include <list>
#include <mutex>
#include <condition_variable>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <atomic>

using json = nlohmann::json;

std::string bcdToImsi(const std::string& bcd)
{
    std::string imsi;
    std::string bin_digit;
    std::bitset<4> nibble;

    for (int i = 0; i < bcd.size(); i += 4)
    {
        bin_digit = bcd.substr(i, 4);
        std::bitset<4> nibble(bin_digit);
        imsi += std::to_string(nibble.to_ulong());
    }

    return imsi;
}

struct Session 
{
    std::string imsi;
    uint32_t expired_time;
    bool active;
};

json_config conf("config_pgw.json");
Logger logger(conf.get_string_from_json("log_file"));

std::list<Session> sessions;
std::mutex sessions_mtx;
std::condition_variable session_cv;
std::atomic server_active = true;

auto check_subscriber(const std::string& imsi) 
{
    std::lock_guard<std::mutex> lock(sessions_mtx);

    return std::find_if(sessions.begin(), sessions.end(), 
        [&imsi](const Session& s)
        {
            return s.imsi == imsi;
        }
    );
}

void http_server_running()
{
    Timer timer;
    httplib::Server svr;

    svr.Post("/api/check_subscriber", [&timer](const httplib::Request& req, httplib::Response& res) 
    {
        try 
        {
            auto json_data = json::parse(req.body);
            std::string imsi = json_data["imsi"].get<std::string>();
            
            auto subscriber = check_subscriber(imsi);
            bool status;

            {
                std::lock_guard<std::mutex> lock(sessions_mtx);
                status = (subscriber != sessions.end());
            }

            json response = {
                {"status", "success"},
                {"imsi", imsi},
                {"activity", !status ? "not active" : "active"}
            };

            res.set_content(response.dump(), "application/json");
            
            std::cout << timer.getTimestamp() << " [INFO] Checked subscriber: " << imsi << " (activity: " << (status ? "active)" : "not active)") << std::endl;
        }
        catch (const std::exception& e)
        {
            json error_response = {
                {"status", "error"},
                {"message", e.what()}
            };
            
            res.status = 400;
            res.set_content(error_response.dump(), "application/json");
            
            std::cout << timer.getTimestamp() << " [CRIT] Error checking subscriber: " << e.what() << std::endl;
        }
    });

    svr.Post("/api/stop", [&timer, &svr](const httplib::Request&, httplib::Response& res) 
    {
        json response = {
            {"status", "success"},
            {"message", "Server is shutting down"}
        };
        
        res.set_content(response.dump(), "application/json");
        
        std::cout << timer.getTimestamp() << " [INFO] HTTP Server finished work" << std::endl;
        svr.stop();
        server_active = false;
        return;
    });

    svr.Get("/",   
        [](const httplib::Request&, httplib::Response& res) 
        {
            res.set_content("                        PGW WEBPANEL\n"
                            "POST /api/check_subscriber - Check subscriber status\n"
                            "POST /api/stop - Stop server (Think twice before using!!!)", "text/plain");
        }
    );

    std::cout << timer.getTimestamp() << " [INFO] Starting server on http://localhost:8080" << std::endl;

    svr.listen("localhost", 8080);
}

void udp_server_running(std::string& server_ip, int udp_port, int timeout, std::string& blacklist) 
{
    Timer timer;
    char buffer[1024]{};

    UDP_Server server(server_ip, udp_port);
    
    if (!server.init()) 
    {
        std::cerr << timer.getTimestamp() << " [CRIT] Server socket creation failed" << std::endl;
        return;
    }

    int request_len;

    while (server_active) 
    {
        request_len = server.receiveDataFromClient(buffer, 1024);

        if (request_len > 0) 
        {
            std::string imsi = bcdToImsi(std::string(buffer, request_len));
            auto session_find = check_subscriber(imsi);

            if (session_find == sessions.end()) 
            {
                {
                    std::lock_guard<std::mutex> lock(sessions_mtx);
                    sessions.push_back({imsi, timer.getTime() + timeout, true});
                }

                session_cv.notify_one();
                server.sendDataToClient("connect");
                std::cout << timer.getTimestamp() << " [INFO] Session started for IMSI: " << imsi << std::endl;
            } 
            else 
            {
                {
                    std::lock_guard<std::mutex> lock(sessions_mtx);
                    session_find->expired_time = timer.getTime() + timeout;
                    session_find->active = true;
                }

                server.sendDataToClient("continue");
                std::cout << timer.getTimestamp() << " [INFO] Session continued for IMSI: " << imsi << std::endl;
            }
        }
    }

    std::cout << timer.getTimestamp() << " [INFO] UDP Server finished work" << std::endl;
    return;
}

void session_controller_running(int timeout, int graceful_offload_rate = 0) 
{
    Timer timer;

    while (server_active) 
    {
        {
            std::unique_lock<std::mutex> lock(sessions_mtx);
            
            if (sessions.empty()) 
            {
                session_cv.wait_for(lock, std::chrono::milliseconds(10), []{ return !sessions.empty() || !server_active; });
                
                if (!server_active) 
                    break;
            }

            auto nearest_session = std::min_element(sessions.begin(), sessions.end(),
                [](const Session& a, const Session& b) {
                    return a.expired_time < b.expired_time;
                });

            if (nearest_session != sessions.end()) 
            {
                uint64_t wait_time = nearest_session->expired_time - timer.getTime();
                
                if (wait_time > 0) 
                    session_cv.wait_for(lock, std::chrono::milliseconds(wait_time));

                if (timer.getTime() >= nearest_session->expired_time && nearest_session->active) 
                {
                    nearest_session->active = false;
                    sessions.remove_if([](const Session& s) { return !s.active; });
                    std::cout << timer.getTimestamp() << " [INFO] Session expired for IMSI: " << nearest_session->imsi << std::endl;
                }
            }
        }
    }

    std::cout << timer.getTimestamp() << " [INFO] Graceful offload service starts working" << std::endl;

    if (graceful_offload_rate > 0 && !sessions.empty()) 
    {
        std::cout << timer.getTimestamp() << " [INFO] Graceful offload service starts working" << std::endl;
        
        while (true) 
        {       
            if (sessions.empty()) 
                break;

            int total = sessions.size();
            int delete_count = std::max(1, total * graceful_offload_rate / 100);

            auto end_it = sessions.begin();
            std::advance(end_it, delete_count);
            sessions.erase(sessions.begin(), end_it);

            if (!sessions.empty()) 
                std::this_thread::sleep_for(std::chrono::milliseconds(timeout));
        }

        std::cout << timer.getTimestamp() << " [INFO] Graceful offload service finished work" << std::endl;
    }

    std::cout << timer.getTimestamp() << " [INFO] Session Controller finished work" << std::endl;
    return;
}

int main()
{   
    int timeout = std::stoi(conf.get_string_from_json("session_timeout_sec"));
    int graceful_offload_rate = std::stoi(conf.get_string_from_json("graceful_offload_rate"));
    int http_port = std::stoi(conf.get_string_from_json("http_port"));
    int  udp_port = std::stoi(conf.get_string_from_json("udp_port"));
    std::string udp_ip = conf.get_string_from_json("udp_ip");
    std::string http_url = conf.get_string_from_json("http_url");
    std::string blacklist = conf.get_string_from_json("blacklist");
   
    // std::thread udp_server(udp_server_running, udp_ip, udp_port, timeout, blacklist);
    std::thread session_controller(session_controller_running, timeout, graceful_offload_rate);
    std::thread http_server(http_server_running);

    // udp_server.join();
    session_controller.join();
    http_server.join();

    return 0;
}