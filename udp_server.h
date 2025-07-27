#pragma once

#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

class UDP_Server 
{
public:
    UDP_Server(const std::string& server_ip, int port);
    ~UDP_Server();
    
    bool init();
    ssize_t sendDataToClient(const std::string& data);
    ssize_t receiveDataFromClient(char* buffer, size_t buffer_size);
    int getSocketFD();
    
private:
    int sockfd_;
    int port_;
    sockaddr_in server_addr_;
    sockaddr_in client_addr_;
    socklen_t client_addr_len_;
    std::string server_ip_;
};