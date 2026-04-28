/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: haitaabe <haitaabe@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/18 11:09:17 by eazmir            #+#    #+#             */
/*   Updated: 2026/04/17 11:43:27 by haitaabe         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/server.hpp"
#include "../include/client.hpp"
#include "../include/utls.hpp"

server::~server()
{
    delete this->channel;
}

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
        throw  std::runtime_error("Error: Failed to create socket");
    }
}

void server::start_listning()
{
    if (listen(this->_fd_server, 10) < 0)
    {
        throw  std::runtime_error("Error: Failed to listen on socket");
    }
    std::cout << "[" << Utils::getTime() << "] Server listening [::0]:" << _port << " (socket " << this->_fd_server << ")" << std::endl;
}

void server::bind_socket()
{
    int opt = 1;
    setsockopt(this->_fd_server, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (bind(this->_fd_server,(struct sockaddr*)&_addr,sizeof(_addr)) < 0)
    {
        throw  std::runtime_error("Error: Failed to bind socket to port ");
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

server::server(int port,std::string password):
_port(port),
_password(password)
{
    channel = new managerchannel(_clients,password); 
    this->init();
    this->handleEvent();
}

// void server::handleEvent()
// {
//     while (true)
//     {
//         int ret = poll(_pfds.data(), _pfds.size(), 1000);  // ✅ 1s timeout
        
//         if (ret < 0)
//         {
//             std::cerr << "poll() error\n";
//             break;
//         }
//         if (ret == 0)
//             continue;  // ✅ timeout, no events, loop again

//         for (size_t i = 0; i < _pfds.size(); i++)
//         {
//             if (!(_pfds[i].revents & POLLIN))
//                 continue;

//             if (_pfds[i].fd == _fd_server)
//             {
//                 this->accept_connection();  // ✅ no index skip needed
//             }
//             else
//             {
//                 size_t prev_size = _pfds.size();
//                 this->recv_data(i);
                
//                 // ✅ if client was removed, adjust index
//                 if (_pfds.size() < prev_size)
//                     i--;
//             }
//         }
//     }
// }

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
    // struct sockaddr_in client_addr;
    // socklen_t len = sizeof(client_addr);

    // int client_fd = accept(_fd_server, (struct sockaddr*)&client_addr, &len); 
     int client_fd = accept(_fd_server,NULL,NULL);
    _poll.fd = client_fd;
    _poll.events = POLLIN;
    _pfds.push_back(_poll);
    ////////////////////////////////////////////////////////////////
   std::cout << "[" << Utils::getTime() << "] Accepted Connection from: 127.0.0.1:" << _port << " (connection " << client_fd << ")" << std::endl;
    ////////////////////////////////////////////////////////////////
    __client.fd = client_fd;
    __client.port = _port;
    // __client.ip = inet_ntoa(client_addr.sin_addr);
    __client.ip = "127.0.0.1";
    __client.pass_ok = false;
    __client.user_ok = false;
    __client.nick_ok = false;
    __client.regestred = false;
    __client.status = false;
    __client.first = true;
      __client.hostname = Utils::getMachineName();
    /////////////////////////////////////////////////////////////////
    Utils::sendWelcome(client_fd);
    _clients[client_fd] = __client;
}


void server::recv_data(size_t &index)
{
    int fd = _pfds[index].fd;

    if (fd == _fd_server)
        return;

    std::map<int, client>::iterator it = _clients.find(fd);
    if (it == _clients.end())
        return;

    char tmp[1024];
    int n = recv(fd, tmp, sizeof(tmp) - 1, 0);
    if (n <= 0)
    {
        this->disconnect_client(index);
        this->status = true;
        return;
    }
    
    tmp[n] = '\0';
    it->second.buffer.append(tmp, n);
    std::string line;
    while (!(line = Extract_data(it->second)).empty())
    {
        channel->handle_input(line, it->second);
        if (_clients.find(fd) == _clients.end())
        {
            this->status = true;
            break;
        }
    }
}

// void server::recv_data(size_t &index)
// {
//     int n;
//     char tmp[1024];

//     int fd = _pfds[index].fd;

//     n = recv(fd, tmp, sizeof(tmp) - 1, 0);
//     if (n <= 0)
//     {
//         this->disconnect_client(index);
//         this->status = true;
//         return;
//     }
//     if (fd == _fd_server) // skip server socket
//         return;
//     // Find client in map
//     std::map<int, client>::iterator it = _clients.find(fd);
//     if (it == _clients.end())
//         return;
//   _clients[index].buffer.append(tmp, n);
//     std::string line;
//    while (!(line = Extract_data(it->second)).empty())
//     {
//         channel->handle_input(line, it->second);
//         if (_clients.find(fd) == _clients.end()) 
//         {
//             this->status = true;
//             break; 
//         }
//     }
// }

void server::disconnect_client(size_t &index)
{
    int fd = _pfds[index].fd;

    std::map<int, client>::iterator it = _clients.find(fd);
    if (it != _clients.end())
    {
        channel->handleQuit("QUIT :Remote host closed the connection", it->second);
    }
    close(fd);
    _clients.erase(fd);
    _pfds.erase(_pfds.begin() + index);
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

