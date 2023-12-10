#include "IRCChannel.hpp"
#include <string>
#include <utility>

IRCChannel::IRCChannel(const std::string &channel_name) :
	modes(0),
	name(channel_name),
	topic(NULL)
{}

IRCChannel::IRCChannel(const IRCChannel &other) :
	modes(other.modes),
	name(other.name),
	users(other.users),
	muted_users(other.muted_users),
	banned_users(other.banned_users)
{
	// topic = new std::string(*(other.topic));
}

IRCChannel::~IRCChannel()
{
	// delete (topic);
}

const std::string	&IRCChannel::getName() const throw()
{
	return (name);
}

const IRCChannel::UsersList	&IRCChannel::getUsers() const throw()
{
	return (users);
}

void	IRCChannel::addUser(const std::string nickname)
{
	//std::pair<UsersList::iterator, bool>	result;

	//result = users.insert(nickname);
	if (banned_users.find(nickname) == banned_users.end())
		users.insert(nickname);
}

void	IRCChannel::removeUser(const std::string nickname)
{
	users.erase(nickname);
}

bool	IRCChannel::isUserInChannel(const std::string nickname) const throw()
{
	return (users.find(nickname) != users.end());
}

void	IRCChannel::muteUser(const std::string nickname)
{
	muted_users.insert(nickname);
}

bool	IRCChannel::isUserMuted(const std::string nickname) const throw()
{
	return (muted_users.find(nickname) != muted_users.end());
}

void	IRCChannel::banUser(const std::string nickname)
{
	banned_users.insert(nickname);
}

bool	IRCChannel::isUserBanned(const std::string nickname) const throw()
{
	return (banned_users.find(nickname) != banned_users.end());
}

const std::string	IRCChannel::getTopic() const throw()
{
	if (topic == NULL)
		return ("");
	else
		return (*topic);
}

void	IRCChannel::setTopic(const std::string &topic_str)
{
	delete (topic);
	topic = new std::string(topic_str);
}

int	IRCChannel::getModeFlags()
{
	return (modes);
}

void	IRCChannel::setModeFlag(int flag)
{
	modes |= flag;
}

void	IRCChannel::clearModeFlag(int flag)
{
	modes &= ~flag;
}

void	IRCChannel::Channel_commands(IRCUser &user, const IRCMessage &msg)
{
	std::cout << RED << "inside Channel Commands" << DEF_COLOR << std::endl;
	(void)user;
	(void)msg;
	// MemFuncPtr	f;

	// try
	// {
	// 	f = chan_func_map.at(msg.command);
	// }
	// catch (std::out_of_range &e)
	// {
	// 	std::cerr << "Warning: Invalid command (" << msg.command << ")" << std::endl;
	// 	return ;
	// }
	// // Call function
	// (this->*f)(user, msg);
}
