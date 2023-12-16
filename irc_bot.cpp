#include "IRCBot.hpp"
#include <iostream>
#include <cstdlib>

int	main(int argc, char **argv)
{
	if (argc < 3)
	{
		std::cout << "Usage: " << argv[0] << " <server> <port> [<password>]" << std::endl;
		return (1);
	}
	
	long port_num = std::atol(argv[2]);
	IRCBot	bot(argv[1], port_num);
	if (argc > 3)
		bot.authenticateConnection(argv[3]);
	bot.registerAsUser("ParrotBot", "parrot", "Parrot");
	while (bot.parrotPRIVMSG())
		;
	return (0);
}
