# include "IRCServer.hpp"
# include "IRCUser.hpp"
# include "IRCNumericReplies.hpp"
# include <iostream>
#include <cstdlib>

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

void	IRCServer::C_handlePART(IRCUser &user, const IRCMessage &msg)
{
	std::string									reply;
	std::string									part_msg = user.getNickname();
	std::map<std::string, IRCChannel>::iterator	channel_it;
	std::vector<std::string>					target_channels;
	std::vector<std::string>::iterator			target_it;
	std::string									PART;

	if(msg.params.empty())
		reply = ERR_NEEDMOREPARAMS(servername, user.getNickname(), msg.command);
	else
	{
		target_channels = split(msg.params[0], ',');
		/*	Checks if <part msg> is available or not	*/
		if(msg.params.size() > 1)
			part_msg = msg.params[1];
		/*	Iterates thru all of the channel names that user wanna part */
		for(target_it = target_channels.begin(); target_it != target_channels.end(); ++target_it)
		{
			channel_it = channels.find(*target_it);
			/*	Check if channelname is valid	*/
			if(channel_it == channels.end())
				reply = ERR_NOSUCHCHANNEL(servername, user.getNickname(), *target_it);
			else
			{
				/*	Checks if user is in channel or not */
				if(!channel_it->second.isUserInChannel(user.getNickname()))
				{
					reply = ERR_NOTONCHANNEL(servername, user.getNickname(), channel_it->second.getName());
					user.queueSend(reply.c_str(), reply.size());
					reply.empty();
				}
				else
				{
					channel_it->second.removeUser(user.getNickname());
					if(channel_it->second.isUserOper(user.getNickname()))
						channel_it->second.removeOper(user.getNickname());
					PART = ":" + user.getNickname() + " PART " + channel_it->second.getName() + " " + part_msg + "\r\n";
					user.queueSend(PART.c_str(), PART.size());
					broadcastToChannel(channel_it->second.getName(), PART);
				}
			}
		}
		
	}
	if (!reply.empty())
			user.queueSend(reply.c_str(), reply.size());
}

void	IRCServer::C_handleMODE(IRCUser &user, const IRCMessage &msg)
{
	(void)user;
	(void)msg;
	std::map<std::string, IRCChannel>::iterator	channel_it;
	std::string									reply;
	std::cout << RED << "Inside Chnnel Modes" << DEF_COLOR << std::endl;

	channel_it = channels.find(msg.params[0]);
	if (channel_it == channels.end())
		reply = ERR_NOSUCHCHANNEL(servername, user.getNickname(), msg.params[0]);
	else if (msg.params.size() == 1)
		reply = RPL_CHANNELMODEIS(servername, user.getNickname(), msg.params[0], ":+kl");
	if(!reply.empty())
		user.queueSend(reply.c_str(), reply.size());
	
}
