# include "IRCServer.hpp"
# include "IRCUser.hpp"
# include "IRCNumericReplies.hpp"
#include "utils.hpp"
#include <iostream>
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

void	IRCServer::C_handleMODE(IRCUser &user, const IRCMessage &msg)
{
	std::map<std::string, IRCChannel>::iterator				channel_it;
	std::string												reply;
	int														flag_requested;

	channel_it = channels.find(msg.params[0]);
	if (channel_it == channels.end())
		reply = ERR_NOSUCHCHANNEL(servername, user.getNickname(), msg.params[0]);
	else if (msg.params.size() == 1)
	{
		std::string	mode_str;

		mode_str = channel_it->second.getModestr();
		reply = RPL_CHANNELMODEIS(servername, user.getNickname(), msg.params[0], mode_str);
	}
	else if (msg.params[1].size() != 2 || !(msg.params[1][0] == '+' || msg.params[1][0] == '-'))
		reply = ERR_UNKNOWNMODE(servername, user.getNickname(), channel_it->second.getName(), msg.params[1]);
	else
	{
		try
		{
			flag_requested = channel_it->second.getFlag_map().at(msg.params[1][1]);
		}
		catch (const std::out_of_range &e)
		{
			reply = ERR_UNKNOWNMODE(servername, user.getNickname(), channel_it->second.getName(), msg.params[1]);
			user.queueSend(reply.c_str(), reply.size());
			return ;
		}
		if (msg.params[1][0] == '+')
		{
			if(!channel_it->second.isUserOper(user.getNickname()))
			{
				reply = ERR_CHANOPRIVSNEEDED(servername, user.getNickname(), channel_it->second.getName());
				std::cout << RED << channel_it->second.getName() + ": User is not an Operator!" << DEF_COLOR << std::endl;
			}
			else
			{
				if(flag_requested & C_KEY)
					c_key(user, msg, channel_it->second);
				else if(flag_requested & C_LIMIT)
					c_limit(user, msg, channel_it->second);
				else if(flag_requested & C_OPER)
					c_oper(user, msg, channel_it->second);
				else if(flag_requested & C_BANNED)
					c_banned(user, msg, channel_it->second);
				else if(flag_requested & C_MUTED)
					c_mute(user, msg, channel_it->second);
			}
		}
		else if(msg.params[1][0] == '-')
		{
			if(flag_requested & C_KEY)
				channel_it->second.removeKey();
			else if(flag_requested & C_LIMIT)
				channel_it->second.clearLimit();
			else if(flag_requested & C_OPER)
				channel_it->second.removeOper(user.getNickname());
			else if(flag_requested & C_BANNED)
				c_unban(user, msg, channel_it->second);
			else if(flag_requested & C_MUTED)
				c_unmute(user, msg, channel_it->second);
		}
	}
	if(!reply.empty())
		user.queueSend(reply.c_str(), reply.size());
	
}

void	IRCServer::C_handleINVITE(IRCUser &user, const IRCMessage &msg)
{
	std::string		reply;

	if (msg.params.size() < 2)
		reply = ERR_NEEDMOREPARAMS(servername, user.getNickname(), "INVITE");
	else if (users_map.find(msg.params[0]) == users_map.end())
		reply = ERR_NOSUCHNICK(servername, user.getNickname(), msg.params[0]);
	else if (channels.find(msg.params[1]) != channels.end()
			&& channels.at(msg.params[1]).isUserInChannel(msg.params[0]))
		reply = ERR_USERONCHANNEL(servername, user.getNickname(), msg.params[0], msg.params[1]);
	else if (channels.find(msg.params[1]) == channels.end()
			|| channels.at(msg.params[1]).isUserInChannel(user.getNickname()) == false)
		reply = ERR_NOTONCHANNEL(servername, user.getNickname(), msg.params[1]);
	if (!reply.empty())
	{
		user.queueSend(reply.c_str(), reply.size());
		return ;
	}
	reply = ":" + user.getNickname() + " INVITE " + msg.params[0] + " :" + msg.params[1] + "\r\n";
	clients[users_map[msg.params[0]]].queueSend(reply.c_str(), reply.size());
	reply = RPL_INVITING(servername, user.getNickname(), msg.params[1], msg.params[0]);
	user.queueSend(reply.c_str(), reply.size());
}

void	IRCServer::C_handleLIST(IRCUser &user, const IRCMessage &msg)
{
	std::string					reply;
	std::vector<std::string>	channel_names;

	if (msg.params.size() >= 2 && msg.params[1] != servername)
	{
		reply = ERR_NOSUCHSERVER(servername, user.getNickname(), msg.params[1]);
		user.queueSend(reply.c_str(), reply.size());
		return ;
	}
	if (msg.params.size() == 0)
	{
		std::map<std::string, IRCChannel>::iterator it;

		for (it = channels.begin(); it != channels.end(); it++)
		{
			reply = RPL_LIST(servername, user.getNickname(), it->second.getName(),
							to_string(it->second.getNumUsers()), it->second.getTopic());
			user.queueSend(reply.c_str(), reply.size());
		}
		reply = RPL_LISTEND(servername, user.getNickname());
		user.queueSend(reply.c_str(), reply.size());
	}
	else
	{
		std::vector<std::string>::iterator it;

		channel_names = split(msg.params[0], ',');
		for (it = channel_names.begin(); it < channel_names.end(); it++)
		{
			try
			{
				IRCChannel &channel = channels.at(*it);
				reply = RPL_LIST(servername, user.getNickname(), *it,
							to_string(channel.getNumUsers()), channel.getTopic());
				user.queueSend(reply.c_str(), reply.size());
			}
			catch (const std::out_of_range &e)
			{
				// Should we send ERR_NOSUCHCHANNEL?
				continue ;
			}
		}
		reply = RPL_LISTEND(servername, user.getNickname());
		user.queueSend(reply.c_str(), reply.size());
	}
}

void	IRCServer::C_handleKICK(IRCUser &user, const IRCMessage &msg)
{
	std::string										reply;
	std::map<std::string, IRCChannel>::iterator 	it;
	std::map<std::string, size_t>::iterator 		target_it;
	std::string										part_msg = user.getNickname();

	if(msg.params.size() < 3)
		reply = ERR_NEEDMOREPARAMS(servername, user.getNickname(), msg.command);
	else
	{
		it = channels.find(msg.params[1]);
		if(it == channels.end())
			reply = ERR_NOSUCHCHANNEL(servername, user.getNickname(), msg.params[1]);
		else
		{
			if(!it->second.isUserInChannel(user.getNickname()))
				reply = ERR_NOTONCHANNEL(servername, user.getNickname(), it->second.getName());
			else if(!it->second.isUserOper(user.getNickname()))
				reply = ERR_CHANOPRIVSNEEDED(servername, user.getNickname(), it->second.getName());
			else if(!it->second.isUserInChannel(msg.params[2]))
				reply = ERR_USERNOTINCHANNEL(servername, user.getNickname(), msg.params[2], it->second.getName());
			else
			{       		
				if(msg.params.size() > 4)
					part_msg = msg.params[3];
				broadcastToChannel(it->second.getName(), reply);
				reply = ":" + user.getNickname() + " KICK " + it->second.getName() + " " + msg.params[2] + " " + part_msg + "\r\n";
				user.queueSend(reply.c_str(), reply.size());
				reply = ":" + msg.params[2] + " PART " + it->second.getName() + " " + part_msg + "\r\n";
				it->second.removeUser(msg.params[2]);
				if(it->second.isUserOper(msg.params[2]))
					it->second.removeOper(msg.params[2]);
			}
		}
	}
	if(!reply.empty())
		user.queueSend(reply.c_str(), reply.size());
}