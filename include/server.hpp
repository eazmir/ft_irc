/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eazmir <eazmir@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/18 11:09:01 by eazmir            #+#    #+#             */
/*   Updated: 2026/02/20 17:03:43 by eazmir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string>
#include <poll.h>
#include <vector>
#include <algorithm>
#include <fcntl.h>
#include <map>
#include <set>

// Forward declaration
class managerchannel;

///////////////////////////////////////////////////////////
#define BUFFER_SIZE 1024
#define MAX_CLIENT 1000
///////////////////////////////////////////////////////////
struct client
{
    std::string buffer;      // don,t care about this, just for storing incoming data until we get a full line
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    int         fd;          // File descriptor (socket)
    int         status;      // Status code, can define 0=connected, 1=authenticated, etc.
    bool        pass_ok;     // Password validated
    bool        user_ok;     // USER command received
    bool        nick_ok;     // NICK command received
    bool        regestred;   // Fully registered (passed all checks)

    std::string nickname;    // Client's nickname
    std::string username;    // Client's username
    std::string hostname;    // Client's hostname (optional, from getpeername)
    std::string realname;    // Client's real name (from USER command)
};

//////////////////////////////////////////////////////////
class  server
{
    private:
        int _port;
        int _fd_server;
        bool status;
        ///////////////////////////////////////////////////
        std::string _password;
        std::map<int ,client> _clients;
        ///////////////////////////////////////////////////
         managerchannel *channel; 
        struct sockaddr_in _addr;
        std::vector<pollfd> _pfds;
    public:
        server();
        server(int port,std::string password);
        ///////////////////////////////////////////////////
        void handleEvent(void);
        void start_listning(void);
        void setup_address(void);
        void bind_socket(void);
        void create_socket(void);
        void setup_poll(void);
        void init(void);
        void accept_connection();
        void recv_data(size_t &index);
        void disconnect_client(size_t &index);
        std::string Extract_data(client &c);
};

#endif