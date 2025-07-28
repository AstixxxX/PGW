#include "udp_client.h"
#include <iostream>

UDP_Client::UDP_Client(const std::string& server_ip, int port) : server_ip_(server_ip), port_(port), sockfd_(-1), server_addr_{} {}

bool UDP_Client::init() 
{
    sockfd_ = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd_ < 0) 
        return false;

    fcntl(sockfd_, F_SETFL, fcntl(sockfd_, F_GETFL, 0) | O_NONBLOCK); 

    server_addr_.sin_family = AF_INET;
    server_addr_.sin_port = htons(port_);
    
    return inet_pton(AF_INET, server_ip_.c_str(), &server_addr_.sin_addr) > 0;
}

ssize_t UDP_Client::sendDataToServer(const std::string& data) 
{
    return sendto(sockfd_, data.c_str(), data.length(), 0, (sockaddr*)&server_addr_, sizeof(server_addr_));
}

ssize_t UDP_Client::receiveDataFromServer(char* buffer, size_t buffer_size)
{
    return recvfrom(sockfd_, buffer, buffer_size, 0, (sockaddr*)&server_addr_, (socklen_t*)&server_addr_);
}

int UDP_Client::getSocketFD()
{
    return sockfd_;
}

UDP_Client::~UDP_Client() 
{
    if (sockfd_ >= 0) 
        close(sockfd_);
}