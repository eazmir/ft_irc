#include "../../include/server.hpp"
#include "../../include/client.hpp"
#include "../../include/utls.hpp"

bool g_status = true;

server::~server()
{
    clean();
}

server::server():
    auth(_clients,_password)
{
    _password.clear();
    _port = 0;
    status = false;
    _fd_server = -1;
}

void server::setup_address()
{
    _addr.sin_family = AF_INET;
    _addr.sin_addr.s_addr = INADDR_ANY;
    _addr.sin_port = htons(_port);
}

void server::create_socket()
{
    _fd_server = socket(AF_INET, SOCK_STREAM, 0);
    if (_fd_server < 0)
    {
        throw  std::runtime_error("Error: Failed to create socket");
    }
}

void server::start_listning()
{
    if (listen(_fd_server, 10) < 0)
    {
        throw  std::runtime_error("Error: Failed to listen on socket");
    }
    std::cout << "[" << Utils::getTime() << "] Server listening [::0]:" << _port << " (socket " <<_fd_server << ")" << std::endl;
}

void server::bind_socket()
{
    if (bind(_fd_server,(struct sockaddr*)&_addr,sizeof(_addr)) < 0)
    {
        throw std::runtime_error("port: "+ Utils::to_str(_port) +" already in use");
    }
}

void server::setup_poll()
{
    pollfd __poll;

    __poll.fd = _fd_server;
    __poll.events = POLLIN;
    _pfds.push_back(__poll);
}

void server::init()
{
    setup_address();
    create_socket();
    bind_socket();
    start_listning();
    setup_poll();
}

server::server(int port,std::string password):
_port(port),
_password(password),
auth(_clients,password)
{
    init();
    channel = new managerchannel(_clients,password); 
    handleEvent();
}