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
	{
		std::string	mode_str = "+";

		if(channel_it->second.isKeyset())
			mode_str += "k";
		if(channel_it->second.isTopicset())
			mode_str += "t";
		reply = RPL_CHANNELMODEIS(servername, user.getNickname(), msg.params[0], mode_str);
	}
	if(!reply.empty())
		user.queueSend(reply.c_str(), reply.size());
	
}

void	IRCServer::C_handleTOPIC(IRCUser &user, const IRCMessage &msg)
{
	std::string									reply;
	std::map<std::string, IRCChannel>::iterator	it;

	if(msg.params.empty())
		reply = ERR_NEEDMOREPARAMS(servername, user.getNickname(), msg.command);
	else
	{
		/*	Checks if channel exists	*/
		it = channels.find(msg.params[0]);
		if(it == channels.end())
			reply = ERR_NOSUCHCHANNEL(servername, user.getNickname(), msg.params[0]);
		else
		{
			/*	Checks if user is on channel	*/
			if(!it->second.isUserInChannel(user.getNickname()))
				reply = ERR_NOTONCHANNEL(servername, user.getNickname(), it->second.getName());
			else if(msg.params.size() == 1)
			{
				/*	SHOWS CONTENT OF TOPIC */
				/*	Checks if there is a TOPIC str anot */
				if(it->second.isTopicset())
					reply = RPL_TOPIC(servername, user.getNickname(), it->second.getName(), it->second.getTopic());
				else
					reply = RPL_NOTOPIC(servername, user.getNickname(), it->second.getName());
			}
			else
			{
				/*	Trying to alter content of TOPIC	*/
				/*	Checks if user is chanop	*/
				if(!it->second.isUserOper(user.getNickname()))
					reply = ERR_CHANOPRIVSNEEDED(servername, user.getNickname(), it->second.getName());
				else
					it->second.setTopic(msg.params[1]);
			}
		}
	}
	if (!reply.empty())
			user.queueSend(reply.c_str(), reply.size());

}
