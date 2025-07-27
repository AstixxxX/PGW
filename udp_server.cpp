#include "udp_server.h"
#include <iostream>
UDP_Server::UDP_Server(const std::string& server_ip, int port) : server_ip_(server_ip), port_(port), sockfd_(-1), server_addr_{}, client_addr_{} {}

bool UDP_Server::init() 
{
    sockfd_ = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd_ < 0) 
        return false;

    fcntl(sockfd_, F_SETFL, fcntl(sockfd_, F_GETFL, 0) | O_NONBLOCK);

    server_addr_.sin_family = AF_INET;
    server_addr_.sin_port = htons(port_);
    
    if (inet_pton(AF_INET, server_ip_.c_str(), &server_addr_.sin_addr) <= 0)
        return false;

    return bind(sockfd_, (sockaddr*)&server_addr_, sizeof(server_addr_)) == 0;
}

ssize_t UDP_Server::sendDataToClient(const std::string& data) 
{
    return sendto(sockfd_, data.c_str(), data.length(), 0, (sockaddr*)&client_addr_, client_addr_len_);
}

ssize_t UDP_Server::receiveDataFromClient(char* buffer, size_t buffer_size)
{
    client_addr_len_ = sizeof(client_addr_);
    return recvfrom(sockfd_, buffer, buffer_size, 0, (sockaddr*)&client_addr_, &client_addr_len_);
}

int UDP_Server::getSocketFD()
{
    return sockfd_;
}

UDP_Server::~UDP_Server() 
{
    if (sockfd_ >= 0) 
        close(sockfd_);
}