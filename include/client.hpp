#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <vector>

// Forward declaration
struct client; // forward declaration

class managerchannel
{
private:
    std::vector<client> &_clients; // reference to server's client vector

public:
    managerchannel(std::vector<client> &clients);
    // ~managerchannel() {}

    void handle_input(const std::string &input, client &c);
    void create_channel(const std::string &name);
    void delete_channel(const std::string &name);
    void add_client_to_channel(const std::string &channel_name, client &c);
    void remove_client_from_channel(const std::string &channel_name, client &c);
    void broadcast_message(const std::string &channel_name,
                           const std::string &message,
                           client &sender);
};


#endif
