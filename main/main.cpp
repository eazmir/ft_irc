#include "../include/server.hpp"

int	main(int argc, char *argv[])
{
	if (argc != 3)
		return (0);
	server server(std::atoi(argv[1]), argv[2]);
}