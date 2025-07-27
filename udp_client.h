#pragma once

#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

class UDP_Client 
{
public:
    UDP_Client(const std::string& server_ip, int port);
    ~UDP_Client();
    
    bool init();
    ssize_t sendDataToServer(const std::string& data);
    ssize_t receiveDataFromServer(char* buffer, size_t buffer_size);
    int getSocketFD();
    
private:
    int sockfd_;
    sockaddr_in server_addr_;
    std::string server_ip_;
    int port_;
};