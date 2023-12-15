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

bool isNumeric(const std::string& str) {
    for (std::string::const_iterator it = str.begin(); it != str.end(); ++it) 
	{
        if (!std::isdigit(*it)) 
            return(false);
    }
    return(true);
}

void	IRCServer::C_handleMODE(IRCUser &user, const IRCMessage &msg)
{
	std::map<std::string, IRCChannel>::iterator				channel_it;
	std::string												reply;
	int														flag_requested;
	std::cout << RED << "Inside Chnnel Modes" << DEF_COLOR << std::endl;

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
			std::cout << GREEN << "We are in channel :" + channel_it->second.getName() << DEF_COLOR << std::endl;
			channel_it->second.print_opers();
			std::cout << GREEN << "user.getnickname is  :" + user.getNickname() << DEF_COLOR << std::endl;
			if(channel_it->second.isUserOper(user.getNickname()))
			{
				std::cout << "User is not an operator in channel" << std::endl;
				reply = ERR_CHANOPRIVSNEEDED(servername, user.getNickname(), channel_it->second.getName());
			}
			else
			{
				if(flag_requested & C_KEY)
				{
					if (msg.params.size() < 3)
						reply = ERR_NEEDMOREPARAMS(servername, user.getNickname(), msg.command);
					else if(channel_it->second.isKeyset())
						reply = ERR_KEYSET(servername, user.getNickname(), channel_it->second.getName());
					else
						channel_it->second.setKey(msg.params[2]);
				}
				else if(flag_requested & C_LIMIT)
				{
					if (msg.params.size() < 3)
						reply = ERR_NEEDMOREPARAMS(servername, user.getNickname(), msg.command);
					else if(isNumeric(msg.params[2]))
						channel_it->second.setLimit(std::atoi(msg.params[2].c_str()));
					else
						std::cout << "Lmit can only be numbers" << std::endl;
				}
				else if(flag_requested & C_OPER)
				{
					std::cout << "inside OPER command" << std::endl;
					if (msg.params.size() < 3)
					{
						std::cout << "need more params" << std::endl;
						reply = ERR_NEEDMOREPARAMS(servername, user.getNickname(), msg.command);
					}
					else if (!channel_it->second.isUserInChannel(msg.params[3]))
					{
						std::cout << "inside user not in channel" << std::endl;
						reply = ERR_USERNOTINCHANNEL(servername, user.getNickname(), channel_it->second.getName());
					}
					else
					{
						std::cout << "Inside isuseroper" << std::endl;
						if(!channel_it->second.isUserOper(msg.params[3]))
							channel_it->second.addOper(msg.params[3]);
					}
				}

			}
		}
		else if(msg.params[1][0] == '-')
		{
			std::cout << "Inside - flag" << std::endl;
		}
	}
	if(!reply.empty())
		user.queueSend(reply.c_str(), reply.size());
	
}