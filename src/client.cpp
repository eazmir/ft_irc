#include "../include/client.hpp"
#include "../include/utls.hpp"

managerchannel::managerchannel(std::map<int, client> &clients,const std::string &pass)
    : _clients(clients),
      auth(pass)
{
}

void managerchannel::handle_input(const std::string &input, client &c)
{
    this->auth.tryRegister(c,input);
    if (input.compare(0,5,"PRINT") == 0)
        Utils::printClientsInfo(_clients);
    if (input.compare(0,4,"JOIN") == 0 || input.compare(0,3,"MSG") == 0)
    {
        handleJoin(input,c);
    }
}

 void managerchannel::handleJoin(const std::string &input, client &c)
 {
    /////////////////////////////////////////////
    //-------parse string to single words--------
    std::stringstream ss(input);
    while (ss >> this->token)
        tokens.push_back(token);
    ////////////////////////////////////////////
    //check if input < 2 
    if (tokens.size() < 2)
        return;
    ////////////////////////////////////////////
    //check user is regestred or no
    if (!c.regestred)
        return;
    ///////////////////////////////////////////
    // Ahmed sends:  JOIN #general
    //                 │
    //                 ▼
    //   Does #general exist in server.channels?
    //                 │
    //       ┌─────────┴─────────┐
    //      YES                  NO
    //       │                   │
    //  check modes         CREATE IT now
    //  let ahmed in        make ahmed OP
    ///////////////////////////////////////////
    this->it = channels.find(channel_name);
    if (it ==  channels.end())
    {
        Channel *ch = new Channel();
        ch->limit = 0;
        ch->name = channel_name;
        ch->operators.push_back(c.fd);
        ch->members.push_back(c.fd);
        ch->topic = "";
        ch->password = "";
        ch->topic_restricted = false;
        channels[channel_name] = ch;
        this->ch = ch;
    }
    ///////////////////////////////////////////
   else
    {
        // if (ch->invite_only)
        // {
        //     std::string err = ":server 473 " + c.nickname + " " + channel_name + " :Cannot join channel (+i)\r\n";
        //     send(c.fd, err.c_str(), err.size(), 0);
        //     return;
        // }
        // if (!ch->password.empty())
        // {
        //     std::string given = (tokens.size() >= 3) ? tokens[2] : "";
        //     if (given != ch->password)
        //     {
        //         std::string err = ":server 475 " + c.nickname + " " + channel_name + " :Cannot join channel (+k)\r\n";
        //         send(c.fd, err.c_str(), err.size(), 0);
        //         return;
        //     }
        // }
        // if (ch->limit > 0 && (int)ch->members.size() >= ch->limit)
        // {
        //     std::string err = ":server 471 " + c.nickname + " " + channel_name + " :Cannot join channel (+l)\r\n";
        //     send(c.fd, err.c_str(), err.size(), 0);
        //     return;
        // }
        ch->members.push_back(c.fd);
        Message msg = parseMessage(input);
        std::cout << "command: " << msg.command << std::endl;
        std::cout << "args: "    << msg.args[0] << std::endl;
        std::cout << "msg: "     << msg.trailing << std::endl;

        if (msg.command == "MSG")  // ✅ fixed: was "MSG"
        {
            // if (msg.args.empty())
            //     return;
            // it = channels.find(msg.args[0]);
            // if (it == channels.end())
            //     return;
            ch = it->second;
            std::string text = msg.trailing;
            std::vector<int>::iterator jt;
            std::string full_msg = ":" + c.nickname +
                                "!" + c.username +
                                "@" + c.hostname +
                                " PRIVMSG " + ch->name +
                                " :" + text + "\r\n";
            for (jt = ch->members.begin(); jt != ch->members.end(); jt++)
            {
                std::cout<<"users:" <<*jt<<std::endl;
                if (*jt == c.fd)
                    continue;
                send(*jt, full_msg.c_str(), full_msg.size(), 0);
            }
        }
    }
 }

 Message parseMessage(const std::string &input)
{
    Message msg;

    std::string::size_type pos = input.find(':');

    std::string before;
    if (pos != std::string::npos)
        before = input.substr(0, pos);
    else
        before = input;

    std::string after;
    if (pos != std::string::npos)
        after = input.substr(pos + 1);

    std::istringstream iss(before);
    std::string token;

    bool first = true;
    while (iss >> token)
    {
        if (first)
        {
            msg.command = token;
            first = false;
        }
        else
            msg.args.push_back(token);
    }
    if (!after.empty())
        msg.trailing = after;
    return msg;
}