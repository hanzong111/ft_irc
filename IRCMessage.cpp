#include "IRCMessage.hpp"
#include <string>
#include <vector>
#include <cctype>
# include <iostream>

IRCMessage::IRCMessage() :
	is_valid(true),
	for_channel(false)
{}

IRCMessage::IRCMessage(const std::string &msg) :
	is_valid(true)
{
	size_t	ind = 0;
	size_t	len = 0;
	size_t	msg_len = msg.find("\r\n", 0);

	if (msg_len == 0 || msg_len == msg.npos)
		return ;
	if (msg[0] == ':')
	{
		len = getTokenLen(msg, 1, " ");
		prefix = msg.substr(1, len);
		ind = len + 2;
	}
	len = msg.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", ind) - ind;
	command = msg.substr(ind, len);
	// Not checking if command is valid here.
	ind += len;
	// Skip to next space.
	ind = msg.find(' ', ind);
	// If no space is found, set `ind` to end of string.
	if (ind == std::string::npos)
		ind = msg_len;
	while(ind < msg_len)
	{
		// If there are already 14 parameters, extend the param to end of message
		if (params.size() == 14)
			len = msg_len - ind;
		// If the sequence " :" is found, include spaces in the param
		else if (msg[ind] == ':')
			len = getTokenLen(msg, ++ind, "\r\n");
		else
			len = getTokenLen(msg, ind, " \r\n");
		if (len != 0)
			params.push_back(msg.substr(ind, len));
		ind += len + 1;
	}
	if (ind != msg_len)
		is_valid = false;
	if (params[0][0] == '#' || params[0][0] == '&' || params[0][0] == '+' || params[0][0] == '!')
		for_channel = true;
}

IRCMessage::IRCMessage(const IRCMessage &other)
{
	*this = other;
}

IRCMessage &IRCMessage::operator=(const IRCMessage &other)
{
	if (this == &other)
		return (*this);
	prefix = other.prefix;
	command = other.command;
	params = other.params;
	is_valid = other.is_valid;
	for_channel = other.for_channel;
	return (*this);
}

IRCMessage::~IRCMessage()
{}

/**
 * @brief Get the length of a token from a given position until the first 
 * occurrence of a character in the terminator set.
 *
 * This function calculates the length of a token in the provided string 'token' 
 * starting from the given position 'pos' and ending at the first occurrence of 
 * any character in the 'terminator_set'. It excludes the last two characters 
 * if they form a CRLF (Carriage Return + Line Feed) sequence.
 *
 * @param token The input string containing the token.
 * @param pos The starting position in the 'token' string to begin the search 
 * for the terminator characters.
 * @param terminator_set A string containing characters considered as 
 * terminators for the token.
 * @return The length of the token, excluding the CRLF sequence if present at 
 * the end. If the token is not found or if 'pos' is out of bounds, 0 is 
 * returned.
 * @throw This function does not throw any exceptions.
 */
size_t	IRCMessage::getTokenLen(const std::string &token, size_t pos,
	const std::string &terminator_set) const throw()
{
	size_t len = token.find_first_of(terminator_set, pos) - pos;

	// If last two characters are CRLF, exclude from result
	if (token.find("\r\n", len - 2) == 0)
		return (len - 2);
	// If not found, return 0.
	else if (len == std::string::npos - pos)
		return (0);
	return (len);
}

bool	IRCMessage::isValid() const throw()
{
	return (is_valid);
}

bool	IRCMessage::for_Channel() const throw()
{
	return (for_channel);
}
