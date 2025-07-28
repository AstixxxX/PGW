#include "udp_server.h"
#include "../utility/timer.h"
#include "../utility/logger.h"
#include "../utility/json-config.h"
#include "../utility/imsi_utils.h"
#include <thread>
#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <mutex>
#include <condition_variable>
#include "../utility/httplib.h"
#include <nlohmann/json.hpp>
#include <atomic>

using json = nlohmann::json;

json_config conf("config_pgw.json");
CDR_Logger cdr_logger(conf.get_string_from_json("cdr_file"));
Logger logger(conf.get_string_from_json("log_file"));

struct Session 
{
    std::string imsi;
    uint32_t expired_time;
    bool active;
};

bool blacklist_imsi(const std::string& blacklist_path, const std::string& cur_imsi)
{
    std::fstream file(blacklist_path, std::ios::in);

    if (!file.is_open())
    {
        logger.error("Blacklist unavaliable for the path: " + blacklist_path);
        logger.warn("Possible blocked IMSI can be reach the network: " + cur_imsi);
        return false;
    }

    std::string black_imsi;

    while (std::getline(file, black_imsi))
        if (black_imsi == cur_imsi)
            return true;

    return false;
}

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

void http_server_running(const std::string& http_url, int http_port)
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
            logger.info("Checked subscriber: " + imsi + " (activity: " + (status ? "active)" : "not active)"));
        }
        catch (const std::exception& e)
        {
            json error_response = {
                {"status", "error"},
                {"message", e.what()}
            };
            
            res.status = 400;
            res.set_content(error_response.dump(), "application/json");
            
            logger.error("Subscriber cheching failed");
        }
    });

    svr.Post("/api/stop", [&timer, &svr](const httplib::Request&, httplib::Response& res) 
    {
        json response = {
            {"status", "success"},
            {"message", "Server is shutting down"}
        };
        
        res.set_content(response.dump(), "application/json");
        
        logger.info("HTTP Server finished work");
        svr.stop();
        std::cout << "\n////////////SHUTDOWN////////////" << std::endl << std::endl;
        std::cout << "HTTP_Server.............[FINISH]" << std::endl;
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

    logger.info("Starting server on http://" + http_url + ":" + std::to_string(http_port));
    std::cout << "HTTP_Server.............[ACTIVE]" << std::endl;
    svr.listen(http_url, http_port);
}

void udp_server_running(const std::string& server_ip, int udp_port, int timeout, const std::string& blacklist) 
{
    Timer timer;
    char buffer[1024]{};

    UDP_Server server(server_ip, udp_port);
    
    if (!server.init()) 
    {
        logger.critical("Server socket creation failed");
        server_active = false;
        return;
    }

    std::cout << "UDP_Server..............[ACTIVE]" << std::endl;

    int request_len;

    while (server_active) 
    {
        request_len = server.receiveDataFromClient(buffer, 64);

        if (request_len > 0) 
        {
            std::string imsi = bcdToImsi(std::string(buffer, request_len));

            if (!blacklist_imsi(blacklist, imsi))
            {
                auto session_find = check_subscriber(imsi);

                if (session_find == sessions.end()) 
                {
                    {
                        std::lock_guard<std::mutex> lock(sessions_mtx);
                        sessions.push_back({imsi, timer.getTime() + timeout, true});
                    }

                    session_cv.notify_one();
                    server.sendDataToClient("created");
                    cdr_logger.cdr(imsi, "CREATE");
                } 
                else 
                {
                    {
                        std::lock_guard<std::mutex> lock(sessions_mtx);
                        session_find->expired_time = timer.getTime() + timeout;
                        session_find->active = true;
                    }

                    server.sendDataToClient("updated");
                    cdr_logger.cdr(imsi, "UPDATE");
                }
            }
            else
            {   
                server.sendDataToClient("rejected");
                cdr_logger.cdr(imsi, "REJECT");
            }
        }
    }

    logger.info("UDP Server finished work");
    std::cout << "UDP_Server..............[FINISH]" << std::endl;
    return;
}

void session_controller_running(const std::string& blacklist, int timeout, int graceful_offload_rate = 0) 
{
    Timer timer;
    std::cout << "Session_Controller......[ACTIVE]" << std::endl;

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

                    sessions.remove_if([](const Session& s) 
                    { 
                        if (!s.active)
                        {
                            cdr_logger.cdr(s.imsi, "STOP");
                            return true; 
                        }

                        return false;
                    });
                }
            }
        }
    }

    if (graceful_offload_rate > 0 && !sessions.empty()) 
    {
        logger.info("Graceful offload service started work");
        std::cout << "Graceful_Offload........[ACTIVE]" << std::endl;
        while (true) 
        {       
            if (sessions.empty()) 
                break;

            int total = sessions.size();
            int delete_count = std::max(1, total * graceful_offload_rate / 100);

            auto end_it = sessions.begin();
            std::advance(end_it, delete_count);
            std::for_each(sessions.begin(), end_it, [](const auto& it){ cdr_logger.cdr(it.imsi, "STOP");});
            sessions.erase(sessions.begin(), end_it);

            if (!sessions.empty()) 
                std::this_thread::sleep_for(std::chrono::seconds(timeout));
        }

        std::cout << "Graceful_Offload........[FINISH]" << std::endl;
        logger.info("Graceful offload service finished work");
    }

    logger.info("Session Controller finished work");
    std::cout << "Server_Controller.......[FINISH]" << std::endl;
    return;
}

int main()
{   
    std::cout << "      Packet Gateway (PGW)" << std::endl << std::endl;
    std::cout << "////////////WORKING////////////" << std::endl << std::endl;
    
    int timeout = std::stoi(conf.get_string_from_json("session_timeout_sec"));
    int graceful_offload_rate = std::stoi(conf.get_string_from_json("graceful_offload_rate"));
    int http_port = std::stoi(conf.get_string_from_json("http_port"));
    int  udp_port = std::stoi(conf.get_string_from_json("udp_port"));
    std::string udp_ip = conf.get_string_from_json("udp_ip");
    std::string http_url = conf.get_string_from_json("http_url");
    std::string blacklist = conf.get_string_from_json("blacklist");
   
    std::thread udp_server(udp_server_running, udp_ip, udp_port, timeout, blacklist);
    std::thread session_controller(session_controller_running, blacklist, timeout, graceful_offload_rate);
    std::thread http_server(http_server_running, http_url, http_port);

    udp_server.join();
    session_controller.join();
    http_server.join();

    return 0;
}