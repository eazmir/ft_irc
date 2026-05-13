#include "../../include/channels/channel.hpp"

static void sendHelp(client &c)
{
    std::string response =
        ":Bot!bot@IRC PRIVMSG " + c.nickname +
        " :Bot commands: !play, !time, !fact, !help\r\n";
    send(c.fd, response.c_str(), response.size(), 0);
}

static void sayfact(client &c)
{
    std::string facts[] = {
        "Honey never spoils. Archaeologists have found edible honey in ancient Egyptian tombs.",
        "Octopuses have three hearts and blue blood.",
        "Bananas are berries, but strawberries are not.",
        "A day on Venus is longer than a year on Venus.",
        "Sharks existed before trees.",
        "The Eiffel Tower can grow taller in summer because metal expands with heat.",
        "There are more possible chess games than atoms in the observable universe.",
        "Wombat droppings are cube-shaped.",
        "The human body contains enough iron to make a small nail.",
        "A group of flamingos is called a flamboyance."
    };

    int index = rand() % (sizeof(facts) / sizeof(facts[0]));

    std::string response =
        ":Bot!bot@IRC PRIVMSG " + c.nickname +
        " :" + facts[index] + "\r\n";

    send(c.fd, response.c_str(), response.size(), 0);
}

static void playGame(client &c)
{
    std::string options[] = {"Rock", "Paper", "Scissors"};
    int index = rand() % 3;

    std::string response =
        ":Bot!bot@IRC PRIVMSG " + c.nickname +
        " :Let's play Rock, Paper, Scissors! I choose: " +
        options[index] + "!\r\n";

    send(c.fd, response.c_str(), response.size(), 0);
}

static void whatTime(client &c)
{
    time_t now = time(NULL);
    char *timeStr = ctime(&now);

    if (timeStr)
    {
        std::string current(timeStr);

        if (!current.empty() && current[current.length() - 1] == '\n')
            current.erase(current.length() - 1);

        std::string response =
            ":Bot!bot@IRC PRIVMSG " + c.nickname +
            " :Current time: " + current + "\r\n";

        send(c.fd, response.c_str(), response.size(), 0);
    }
}

void managerchannel::handleBotCommand(client &sender, const std::string &command)
{
    if (command == "help" || command == "HELP")
        sendHelp(sender);
    else if (command == "fact" || command == "FACT")
        sayfact(sender);
    else if (command == "play" || command == "PLAY")
        playGame(sender);
    else if (command == "time" || command == "TIME")
        whatTime(sender);
    else
    {
        std::string response =
            ":Bot!bot@IRC PRIVMSG " + sender.nickname +
            " :Unknown command. Try !help\r\n";
        send(sender.fd, response.c_str(), response.size(), 0);
    }
}