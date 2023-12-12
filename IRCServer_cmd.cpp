# include "IRCServer.hpp"
# include "IRCUser.hpp"
# include "IRCNumericReplies.hpp"
# include <iostream>
#include <cstdlib>

void	IRCServer::sendWelcomeMessages(IRCUser &user)
{
	std::string	str;

	// Send welcome messages
	str = RPL_WELCOME(servername, user.getNickname(),
		user.getUsername(), user.getRealName());
	str += RPL_YOURHOST(servername, user.getNickname(), IRCSERVER_VER);
	str += RPL_CREATED(servername, user.getNickname(), time_created);
	str += RPL_MYINFO(servername, user.getNickname(),
				IRCSERVER_VER,
				IRCSERVER_SUPPORTED_USER_MODES,
				IRCSERVER_SUPPORTED_CHANNEL_MODES);
	user.queueSend(str.c_str(), str.size());
}


void	IRCServer::S_handlePASS(IRCUser &user, const IRCMessage &msg)
{
	std::string	reply;

	if (msg.params.size() < 1)
		reply = ERR_NEEDMOREPARAMS(servername, user.getNickname(), msg.command);
	else if (user.isAuthenticated() && user.isRegistered())
		reply = ERR_ALREADYREGISTRED(servername, user.getNickname());
	else if (!(msg.params[0] == conn_pass))
		reply = ERR_WRONGPASS(servername, user.getNickname(), msg.command);
	if (!reply.empty())
		user.queueSend(reply.c_str(), reply.size());
	else if (msg.params[0] == conn_pass)
		user.makeAuthenticated();
}

void	IRCServer::S_handleNICK(IRCUser &user, const IRCMessage &msg)
{
	std::string reply;

	if (msg.params.size() < 1)
		reply = ERR_NONICKNAMEGIVEN(servername, "*");
	else if (!IRCUser::isValidNickname(msg.params[0]))
		reply = ERR_ERRONEUSNICKNAME(servername, user.getNickname(), msg.params[0]);
	else if (users_map.find(msg.params[0]) != users_map.end())
		reply = ERR_NICKNAMEINUSE(servername, user.getNickname(), msg.params[0]);
	else if (user.getModeFlags() & RESTRICTED)
		reply = ERR_RESTRICTED(servername, user.getNickname());
	if (!reply.empty())
	{
		user.queueSend(reply.c_str(), reply.size());
		return ;
	}

	// Update IRCUser object
	std::string old_nickname = user.getNickname();
	user.changeNickname(msg.params[0]);
	// Update `users_map`
	size_t ind = users_map[old_nickname];
	users_map.erase(old_nickname);
	users_map[msg.params[0]] = ind;
	// Check if registration is completed
	if (!user.getUsername().empty())
	{
		user.makeRegistered();
		sendWelcomeMessages(user);
	}
}

void	IRCServer::S_handleUSER(IRCUser &user, const IRCMessage &msg)
{
	std::string reply;

	if (msg.params.size() < 4)
		reply = ERR_NEEDMOREPARAMS(servername, user.getNickname(), msg.command);
	else if (user.isAuthenticated() && user.getUsername() != "")
		reply = ERR_ALREADYREGISTRED(servername, user.getNickname());
	if (!reply.empty())
	{
		user.queueSend(reply.c_str(), reply.size());
		return ;
	}
	user.changeUsername(msg.params[0]);
	user.changeRealName(msg.params[3]);
	// It would be a good idea to check if the mode specified in the message
	// is a single digit, but since there is no reply specified for invalid
	// parameter in RFC2812, the check isn't performed here.
	int	mode = std::atoi(msg.params[1].c_str()) & (WALLOPS | INVISIBLE);
	user.setModeFlag(mode);
	// Check if registration is completed
	if (user.getNickname().find(IRCUSER_DEFAULT_NICK_PREFIX) != 0)
	{
		user.makeRegistered();
		sendWelcomeMessages(user);
	}
}

void	IRCServer::S_handleOPER(IRCUser &user, const IRCMessage &msg)
{
	std::string reply;

	if (msg.params.size() < 2)
		reply = ERR_NEEDMOREPARAMS(servername, user.getNickname(), msg.command);
	else if (msg.params[1] != SERVER_OPERATOR_PASS)
		reply = ERR_PASSWDMISMATCH(servername, user.getNickname());
	else
	{
		reply = RPL_YOUREOPER(servername, user.getNickname());
		user.setModeFlag(OPER);
	}
	if (!reply.empty())
	{
		user.queueSend(reply.c_str(), reply.size());
		return ;
	}	
}

void	IRCServer::S_handleMODE(IRCUser &user, const IRCMessage &msg)
{
	std::string reply;
	IRCUserModesMap::iterator flag;
	int			flag_requested = -1;

	if (msg.params.size() < 1)
		reply = ERR_NEEDMOREPARAMS(servername, user.getNickname(), msg.command);
	else if (user.getNickname() != msg.params[0])
		reply = ERR_USERSDONTMATCH(servername, user.getNickname());
	else if (msg.params.size() == 1)
		reply = RPL_UMODEIS(servername, user.getNickname(), user.getModestr());
	else if (msg.params[1].size() != 2 || !(msg.params[1][0] == '+' || msg.params[1][0] == '-'))
		reply = ERR_UMODEUNKNOWNFLAG(servername, user.getNickname());
	else
	{
		try
		{
			flag_requested = user.getFlag_map().at(msg.params[1][1]);
		}
		catch (const std::out_of_range &e)
		{
			reply = ERR_UMODEUNKNOWNFLAG(servername, user.getNickname());
			user.queueSend(reply.c_str(), reply.size());
			return ;
		}
		if(msg.params[1][0] == '+')
		{
			if(flag_requested == OPER || flag_requested == LOCAL_OPER || flag_requested == AWAY)
				return;
			user.setModeFlag(flag_requested);
		}
		else if (msg.params[1][0] == '-')
		{
			if(flag_requested == RESTRICTED)
				return;
			user.clearModeFlag(flag_requested);
		}
		flag = user.getFlag_map().end();
	}
	if (!reply.empty())
	{
		user.queueSend(reply.c_str(), reply.size());
		return ;
	}	
}

void	IRCServer::S_handleQUIT(IRCUser &user, const IRCMessage &msg)
{
	size_t		ind;
	std::string	reply;

	if (msg.params.size() != 1)
		reply = ":" + user.getNickname() + "ERROR :" + msg.params[1] + "\r\n";
	else
		reply = ":" + user.getNickname() + "ERROR :\r\n";
	if (!reply.empty())
	{
		user.queueSend(reply.c_str(), reply.size());
		return ;
	}
	ind = users_map.at(user.getNickname());
	users_map.erase(clients[ind].getNickname());
	removeClient(clients[ind]);
	updateUsersMap();
}

std::vector<std::string> split(const std::string& str, char delimiter)
{
    std::vector<std::string> tokens;
    std::istringstream tokenStream(str);
    std::string token;
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

void	make_channelandkeys(std::map<std::string , std::string> *channels_and_keys, const IRCMessage &msg)
{
	std::string	reply;
	std::vector<std::string> keys;
	std::vector<std::string> channels;
	size_t i = 0;

	channels = split(msg.params[0], ',');
	if (msg.params.size() > 1)
		keys = split(msg.params[1], ',');
	while(i < channels.size() && channels[i].empty() != true)
    {
		if(msg.params.size() > 1 &&  i < keys.size() && keys[i].empty() != true)
			channels_and_keys->insert(std::make_pair(channels[i], keys[i]));
		else
			channels_and_keys->insert(std::make_pair(channels[i], ""));
		i++;
	}
}

void	IRCServer::create_channel(IRCUser &user, std::map<std::string, std::string>::iterator 	it, std::string *reply)
{
	std::string 									channel_name = it->first;
	std::string 									channel_key = it->second;
	std::map<std::string, IRCChannel>::iterator		find;

	/*	Makes sure channelname format and channelname is not Taken	*/
	if(!(channel_name[0] == '+' || channel_name[0] == '!' || channel_name[0] == '#' || channel_name[0] == '&'))
	{
		*reply = ERR_NOSUCHCHANNEL(servername, user.getNickname(), channel_name);
		return ;
	}
	if(isChanneltaken(channel_name))
	{
		*reply = ERR_NOSUCHCHANNEL(servername, user.getNickname(), channel_name);
		return ;
	}
	/*	Makes a new IRCChannel object and adds it into our map	*/
	IRCChannel newchannel(channel_name, channel_key);
	channels.insert(std::make_pair(channel_name, newchannel));
	find = channels.find(channel_name);
	std::cout << GREEN << "Succesfully created Channel[" + find->first + "]" + DEF_COLOR << std::endl;
	/*	Joins channel */
	join_channel(user, channel_key, find->second, reply);
}

void	IRCServer::join_channel(IRCUser &user, std::string &user_key, IRCChannel &channel, std::string	*reply)
{
	/*	Checks if user is banned */
	if(channel.isUserBanned(user.getNickname()))
	{
		*reply = ERR_BANNEDFROMCHAN(servername, user.getNickname(), channel.getName());
		return;
	}
	/*	If channel needs a key to join , checks if key value is correct*/
	if(channel.getModeFlags() & C_KEY && (channel.getKey() != (const std::string)user_key))
	{
		*reply = ERR_BADCHANNELKEY(servername, user.getNickname(), channel.getName());
			return;
	}
	channel.addUser(user.getNickname());
	/*	user succesfully joined , sending JOIN command to client.	*/
	*reply = ":" + user.getNickname() + " JOIN " + channel.getName() + "\r\n";
}

void	IRCServer::S_handleJOIN(IRCUser &user, const IRCMessage &msg)
{
	(void)user;
	std::string										reply;
	std::map<std::string , std::string>				channels_and_keys;
	std::map<std::string, std::string>::iterator 	it;
	std::map<std::string, IRCChannel>::iterator 	find_channel;

	make_channelandkeys(&channels_and_keys, msg);
	for (it = channels_and_keys.begin(); it != channels_and_keys.end(); ++it) 
	{
		find_channel = channels.find(it->first);
        if(find_channel == channels.end())
		{
			if(user.getModeFlags() & OPER)
				create_channel(user, it, &reply);
			else
				reply = ERR_NOSUCHCHANNEL(servername, user.getNickname(), it->first);
		}
		else
			join_channel(user, it->second, find_channel->second, &reply);
		if (!reply.empty())
			user.queueSend(reply.c_str(), reply.size());
    }
	// for (find_channel = channels.begin(); find_channel != channels.end(); ++find_channel) 
	// {
	// 	std::cout << "Channel_Name:" + find_channel->second.getName() <<  std::endl;
	// 	if (find_channel->second.getModeFlags() & C_KEY)
	// 		std::cout << "Key :" << find_channel->second.getKey() << std::endl;
    // }
}

void	IRCServer::S_handlePRIVMSG(IRCUser &user, const IRCMessage &msg)
{
	std::string reply;
	std::map<std::string, size_t>::iterator it;
	
	std::cout << "inside PRIVMSG" << std::endl;
	if(msg.for_Channel() == true)
		std::cout << "For Channel" << std::endl;
	else
	{
		std::cout << RED <<  "For Users" << DEF_COLOR << std::endl;
		if(msg.params.size() < 1)
		{
			reply = ERR_NORECIPIENT(servername, user.getNickname(), "PRIVMSG");
			user.queueSend(reply.c_str(), reply.size());
			return;
		}
		else if(msg.params.size() < 2)
		{
			reply = ERR_NOTEXTTOSEND(servername, user.getNickname());
			user.queueSend(reply.c_str(), reply.size());
			return;
		}
		std::cout << "target : " + msg.params[0] << std::endl;
		it = users_map.find(msg.params[0]);
		if(it != users_map.end())
		{
			std::cout << "sending msg" << std::endl;
       		IRCUser		&target = clients[it->second];
			reply =  ":" + user.getNickname() + "!" + user.getUsername() + "@" + user.getIPAddrStr();
			reply += " PRIVMSG " + target.getNickname() + " " + msg.params[1] + "\r\n";
			target.queueSend(reply.c_str(), reply.size());
		}
		else
		{
			reply = ERR_NOSUCHNICK(servername, user.getNickname(), msg.params[0]);
		 	user.queueSend(reply.c_str(), reply.size());
		}
	}
}
