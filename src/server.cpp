/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eazmir <eazmir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/18 11:09:17 by eazmir            #+#    #+#             */
/*   Updated: 2026/02/19 16:48:42 by eazmir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/server.hpp"
#include "../include/client.hpp"

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
}

void server::start_listning()
{
    listen(this->_fd_server,10);
}

void server::bind_socket()
{
    bind(this->_fd_server,(struct sockaddr*)&_addr,sizeof(_addr));
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
    channel = new managerchannel(_client);
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
    _client.push_back(__client);
}


void server::recv_data(size_t &index)
{
    int n;
    char buffer[1024];

    n = recv(_pfds[index].fd,buffer,sizeof(buffer) - 1,0);
    if (n <= 0)
    {
        this->disconnect_client(index);
        this->status = true;
        return;
    }
    if (index == 0)
        return;
    size_t client_index = index - 1;
    _client[client_index].buffer += std::string(buffer,n);
    std::string line;
    while (!(line = Extract_data(_client[client_index])).empty())
    {
        channel->handle_input(line, _client[client_index]);
    }
}

void server::disconnect_client(size_t &index)
{
    close(_pfds[index].fd);
    _pfds.erase(_pfds.begin() + index);
    if (index > 0)
        _client.erase(_client.begin() + (index - 1));
    std::cout << "Client disconnected" << std::endl;
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
