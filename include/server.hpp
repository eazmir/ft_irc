#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <vector>
#include <csignal>
#include <arpa/inet.h>
#include <sstream>
#include "authentication.hpp"
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
#include <cstdio>
#include <fcntl.h>
#include <map>
#include <set>

//////////////////////////////////////////////////////////
extern bool g_status;
///////////////////////////////////////////////////////////
// Forward declaration
class managerchannel;
class authentication;
///////////////////////////////////////////////////////////
#define BUFFER_SIZE 1024
#define MAX_CLIENT 1000
///////////////////////////////////////////////////////////
struct client
{
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    int         fd;          // File descriptor (socket)
    int         status;      // Status code, connection=1 | desconnection=0
    int         port;        // clinet port
 
    bool        pass_ok;     // Password validated
    bool        user_ok;     // USER command received
    bool        nick_ok;     // NICK command received
    bool        first;
    bool        regestred;   // Fully registered (passed all checks)
    std::string     ip;      // server ip
    std::string buffer;
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
        authentication auth;
        //////////////////////////////////////////////////  
        struct sockaddr_in _addr;
        std::vector<pollfd> _pfds;
    public:
        server();
        server(int port,std::string password);
        ~server();
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
        std::string extract_data(client &c);
        ////////////////////////////////////////////////////
        void clean();
};
void signalhandler();
void bye();
#endif