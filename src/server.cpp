/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eazmir <eazmir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/18 11:09:17 by eazmir            #+#    #+#             */
/*   Updated: 2026/02/20 17:02:06 by eazmir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/server.hpp"
#include "../include/client.hpp"
#include <cstdio>

server::server()
{
    this->_password.clear();
    this->_port = 0;
    this->status = false;
}

void server::setup_address()
{
    _addr.sin_family = AF_INET;
    _addr.sin_addr.s_addr = INADDR_ANY;
    _addr.sin_port = htons(_port);
}

void server::create_socket()
{
    this->_fd_server = socket(AF_INET, SOCK_STREAM, 0);
    if (this->_fd_server < 0)
    {
        std::cerr << "Error: Failed to create socket" << std::endl;
        exit(1);
    }
}

void server::start_listning()
{
    if (listen(this->_fd_server, 10) < 0)
    {
        std::cerr << "Error: Failed to listen on socket" << std::endl;
        perror("listen");
        exit(1);
    }
    std::cout << "Server listening on port " << _port << std::endl;
}

void server::bind_socket()
{
    int opt = 1;
    setsockopt(this->_fd_server, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (bind(this->_fd_server,(struct sockaddr*)&_addr,sizeof(_addr)) < 0)
    {
        std::cerr << "Error: Failed to bind socket to port " << _port << std::endl;
        perror("bind");
        exit(1);
    }
}

void server::setup_poll()
{
  pollfd __poll;

  __poll.fd = this->_fd_server;
  __poll.events = POLLIN;
  _pfds.push_back(__poll);
}

void server::init()
{
    this->setup_address();
    this->create_socket();
    this->bind_socket();
    this->start_listning();
    this->setup_poll();
}

server::server(int port,std::string password):_port(port),_password(password)
{
    channel = new managerchannel(_clients);
    this->init();
    this->handleEvent();
}

void server::handleEvent()
{
    while (true)
    {
        poll(_pfds.data(),_pfds.size(),-1);
        for (size_t client_index = 0; client_index < _pfds.size();client_index++)
        {
            if (this->_pfds[client_index].revents & POLLIN)
            {
                if (this->_fd_server == this->_pfds[client_index].fd)
                {
                    this->accept_connection();
                    client_index++;
                }
                else
                {
                    this->recv_data(client_index);
                    if (this->status)
                    {
                        this->status = false;
                        client_index--;
                    }
                }   
            }
        }
    }
}

void server::accept_connection()
{
    client  __client;
    pollfd  _poll;
    int     client_fd;
    
    
     client_fd = accept(_fd_server,NULL,NULL);
    _poll.fd = client_fd;
    _poll.events = POLLIN;
    _pfds.push_back(_poll);
    ////////////////////////////////////////////////////////////////
    std::cout << "New client connected. FD: " << client_fd << std::endl;
    ////////////////////////////////////////////////////////////////
    __client.fd = client_fd;
    __client.pass_ok = false;
    __client.user_ok = false;
    __client.regestred = false;
    /////////////////////////////////////////////////////////////////
    _clients[client_fd] = __client;
}


void server::recv_data(size_t &index)
{
    int n;
    char buffer[1024];

    int fd = _pfds[index].fd;

    n = recv(fd, buffer, sizeof(buffer) - 1, 0);
    if (n <= 0)
    {
        this->disconnect_client(index);
        this->status = true;
        return;
    }

    if (fd == _fd_server) // skip server socket
        return;

    // Find client in map
    std::map<int, client>::iterator it = _clients.find(fd);
    if (it == _clients.end())
        return;
    it->second.buffer += std::string(buffer, n);
    std::string line;
    while (!(line = Extract_data(it->second)).empty())
    {
        channel->handle_input(line, it->second);
    }
}

void server::disconnect_client(size_t &index)
{
    int fd = _pfds[index].fd;
    close(fd);
    _clients.erase(fd);
    _pfds.erase(_pfds.begin() + index);
    std::cout << "Client disconnected FD: " << fd << std::endl;
}

std::string server::Extract_data(client &c)
{
    std::string &buf = c.buffer;

    size_t pos = buf.find('\n');
    if (pos == std::string::npos)
        return "";

    std::string line = buf.substr(0, pos);
    buf.erase(0, pos + 1);

    // Remove trailing '\r' if present
    if (!line.empty() && line[line.size() - 1] == '\r')
        line.erase(line.size() - 1, 1);

    return line;
}