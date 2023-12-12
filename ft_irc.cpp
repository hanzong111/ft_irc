#include "IRCServer.hpp"
#include <limits>
#include <cstdlib>
#include <string>
#include <iostream>

int	main(int argc, char **argv)
{
	if (argc != 3)
	{
		std::cout << "Usage: " << argv[0] << " <port> <password>" << std::endl;
		return (1);
	}

	try
	{
		long port_num = std::atol(argv[1]);
		if (port_num < 0 || port_num > std::numeric_limits<uint16_t>::max())
			throw std::invalid_argument("Invalid port number");
		
		IRCServer server("127.0.0.1", port_num);
		server.setConnPass(std::string(argv[2]));
		server.createChannel("#Test");
		std::cout << "Starting server" << std::endl;
		server.startServer();
		
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return (2);
	}

	return (0);
}
