#include "../utility/logger.h"
#include "../utility/json-config.h"
#include "../utility/timer.h"
#include "../utility/imsi_utils.h"
#include "./network/udp_client.h"
#include <string>
#include <iostream>
#include <unistd.h>

int main()
{
    Timer timer;
    json_config conf("config_client.json");
    Logger logger(conf.get_string_from_json("log_file"));

    UDP_Client client(conf.get_string_from_json("server_ip"), std::stoi(conf.get_string_from_json("server_port")));
    char buffer[1024] {};

    if (!client.init())
    {
        logger.critical("Client socket creation failed");
        std::cerr << "Client socket creation failed" << std::endl;
        return -1;
    }
    
    std::string imsi;

    while (true)
    {
        std::cout << "Enter IMSI:" << std::endl;
        std::cin >> imsi;

        if (!valid_imsi(imsi))
        {
            std::cout << "IMSI contains only 15 digits" << std::endl;
            sleep(2);
            std::system("clear");
        }
        else   
            break;
    }

    int tx;
    int rx;

    tx = client.sendDataToServer(imsiToBcd(imsi));

    if (tx > 0)
        logger.info("IMSI succesfully send to the server");
    else
    {
        logger.critical("IMSI don\'t send to the server");
        std::cerr << "IMSI don\'t send to the server" << std::endl; 
        return -1;
    }

    int max_waiting_time = timer.getTime() + 5;

    while (max_waiting_time >= timer.getTime())
    {
        rx = client.receiveDataFromServer(buffer, 1024);

        if (rx > 0)
        {   
            std::string status = std::string(buffer, rx);
            logger.info("Server send the status of IMSI: " + status);
            std::cout << "Status: " << status << std::endl;
            break;
        }
    }

    if (rx <= 0)
    {
        logger.warn("No reply from the server");
        std::cout << "Server are too busy. Try again later" << std::endl;
    }

    return 0;
}