# include "IRCServer.hpp"
# include "IRCNumericReplies.hpp"
# include "IRCUser.hpp"
#include <cstdlib>

void	IRCServer::c_key(IRCUser &user, const IRCMessage &msg, IRCChannel &target)
{
	std::string	reply;

	if (msg.params.size() < 3)
	{
		reply = ERR_NEEDMOREPARAMS(servername, user.getNickname(), msg.command);
		std::cout << RED << target.getName() + ": Need more parameters" << DEF_COLOR << std::endl;
	}
	else if(target.isKeyset())
	{
		reply = ERR_KEYSET(servername, user.getNickname(), target.getName());
		std::cout << RED << target.getName() + ": Key is already set!" << DEF_COLOR << std::endl;
	}
	else
	{
		target.setKey(msg.params[2]);
		std::cout << GREEN << target.getName() + ": Channel Key is set!" << DEF_COLOR << std::endl;
	}
	if(!reply.empty())
		user.queueSend(reply.c_str(), reply.size());
}

bool isNumeric(const std::string& str) {
    for (std::string::const_iterator it = str.begin(); it != str.end(); ++it) 
	{
        if (!std::isdigit(*it)) 
            return(false);
    }
    return(true);
}

void	IRCServer::c_limit(IRCUser &user, const IRCMessage &msg, IRCChannel &target)
{
	std::string	reply;

	if (msg.params.size() < 3)
	{
		reply = ERR_NEEDMOREPARAMS(servername, user.getNickname(), msg.command);
		std::cout << RED << target.getName() + ": Need more parameters" << DEF_COLOR << std::endl;
	}
	else if(isNumeric(msg.params[2]))
	{
		target.setLimit(std::atoi(msg.params[2].c_str()));
		std::cout << GREEN << target.getName() + ": Limit of channel is set!" << DEF_COLOR << std::endl;
	}
	else
		std::cout << RED << target.getName() + ": limit can only be numeric chars" << DEF_COLOR << std::endl;
	if(!reply.empty())
		user.queueSend(reply.c_str(), reply.size());
}

void	IRCServer::c_oper(IRCUser &user, const IRCMessage &msg, IRCChannel &target)
{
	std::string	reply;

	if (msg.params.size() < 3)
		reply = ERR_NEEDMOREPARAMS(servername, user.getNickname(), msg.command);
	else if (!target.isUserInChannel(msg.params[2]))
	{
		std::cout << RED << target.getName() + ": User is not in channel" << DEF_COLOR << std::endl;
		reply = ERR_USERNOTINCHANNEL(servername, user.getNickname(), target.getName());
	}
	else
	{
		if(!target.isUserOper(msg.params[2]))
		{
			target.addOper(msg.params[2]);
			std::cout << GREEN << target.getName() + ": User is Oper now !" << DEF_COLOR << std::endl;
		}
		else
			std::cout << RED << target.getName() + ": User is ald a channel operator" << DEF_COLOR << std::endl;
	}
	if(!reply.empty())
		user.queueSend(reply.c_str(), reply.size());
}

void	IRCServer::c_banned(IRCUser &user, const IRCMessage &msg, IRCChannel &target)
{
	std::string	reply;

	if(!reply.empty())
		user.queueSend(reply.c_str(), reply.size());
}