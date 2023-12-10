#include "IRCChannel.hpp"
#include <string>
#include <utility>

IRCChannel::IRCChannel(const std::string &channel_name) :
	modes(0),
	name(channel_name),
	topic(NULL),
	key(NULL)
{}

IRCChannel::IRCChannel(const std::string &channel_name, const std::string &channel_key) :
	modes(0),
	name(channel_name),
	topic(NULL),
	key(channel_key)
{
}
IRCChannel::IRCChannel(const IRCChannel &other) :
	modes(other.modes),
	name(other.name),
	key(other.key),
	users(other.users),
	muted_users(other.muted_users),
	banned_users(other.banned_users)
{
	if (other.topic != NULL)
		topic = new std::string(*(other.topic));
	else
		topic = NULL;
}

IRCChannel::~IRCChannel()
{
	delete (topic);
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
