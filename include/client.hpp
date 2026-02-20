#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <vector>
#include <map>
#include <set>

// Forward declaration
struct client; // forward declaration

class managerchannel
{
private:
     std::map<int, client> &_clients; 
public:
    managerchannel(std::map<int, client> &clients);
    // ~managerchannel() {}

    void handle_input(const std::string &input, client &c);
    void create_channel(const std::string &name);
    void delete_channel(const std::string &name);
    void add_client_to_channel(const std::string &channel_name, client &c);
    void remove_client_from_channel(const std::string &channel_name, client &c);
    void broadcast_message(const std::string &channel_name,
                           const std::string &message,
                           client &sender);
    void print_info();
};




#endif
