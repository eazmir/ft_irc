#include "../../include/server.hpp"
#include "../../include/client.hpp"
#include "../../include/utls.hpp"

void handesignals(int s)
{
    (void)s;
    g_status = false;
}

void signalhandler()
{
    signal(SIGINT,handesignals);
    signal(SIGQUIT,handesignals);
}