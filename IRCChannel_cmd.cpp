# include "IRCServer.hpp"
# include "IRCUser.hpp"
# include "IRCNumericReplies.hpp"
# include <iostream>
#include <cstdlib>

void	IRCServer::Channel_commands(IRCUser &user, const IRCMessage &msg)
{
	std::cout << RED << "inside Channel Commands" << DEF_COLOR << std::endl;
	(void)user;
	(void)msg;
	MemFuncPtr	f;

	try
	{
		f = chan_func_map.at(msg.command);
	}
	catch (std::out_of_range &e)
	{
		std::cerr << "Warning: Invalid command (" << msg.command << ")" << std::endl;
		return ;
	}
	// Call function
	(this->*f)(user, msg);
}