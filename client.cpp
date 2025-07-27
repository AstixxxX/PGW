#include "udp_client.h"
#include "logger.h"
#include "json-config.h"
#include <string>
#include <iostream>
#include <bitset>

std::string imsiToBcd(const std::string& imsi) 
{
    std::bitset<4> a;
    std::string bcd;

    for (char ch : imsi)
    {
        a = ch;
        bcd += a.to_string();
    }

    return bcd;
}

int main()
{
    json_config conf("config_client.json");
    Logger logger(conf.get_string_from_json("log_file"));

    UDP_Client client(conf.get_string_from_json("server_ip"), std::stoi(conf.get_string_from_json("server_port")));
    char buffer[1024] {};

    if (!client.init())
        logger.critical("Client socket creation failed");
    
    std::string imsi;
    std::cout << "Enter IMSI:" << std::endl;
    std::cin >> imsi;
    
    while (true)
    {
        int t = client.sendDataToServer(imsiToBcd(imsi));

        if (t > 0)
            logger.info("IMSI succesfully send to the server");
        else
            logger.warn("IMSI don\'t send to the server");

        int l = client.receiveDataFromServer(buffer, 1024);

        if (l > 0)
        {   
            logger.info("Server send the status of IMSI");
            std::cout << "Status: " << std::string(buffer, l) << std::endl;
            break;
        }
        else
            logger.warn("No reply of the server");
    }

    return 0;
}