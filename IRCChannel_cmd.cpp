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

void	IRCServer::C_handleWHO(IRCUser &user, const IRCMessage &msg)
{
	std::map<std::string, IRCChannel>::iterator	find_channel;
	std::set<std::string>						user_list;
	std::set<std::string>::iterator				it;
	std::string									user_str;

	std::cout << "msg.params[0] is" + msg.params[0] << std::endl;
	find_channel = channels.find(msg.params[0]);
	user_list = find_channel->second.getUsers();
	for(it = user_list.begin();it != user_list.end(); ++it)
	{
		user_str += *it;
		user_str += " ";
	}
	std::string reply = RPL_NAMREPLY(servername, user.getNickname(), "=", msg.params[0], user_str);
	user.queueSend(reply.c_str(), reply.size());
}