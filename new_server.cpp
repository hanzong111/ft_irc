#include "TestServer.hpp"
#include <iostream>

int	main(void)
{
	TestServer server("127.0.0.1", 51200);

	std::cout << "Starting server" << std::endl;
	server.startServer();

	return (0);
}
