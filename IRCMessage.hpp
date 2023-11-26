#ifndef FT_IRC_IRCMESSAGE_HPP
# define FT_IRC_IRCMESSAGE_HPP

# include <string>
# include <vector>

class IRCMessage
{
	public:
		IRCMessage();
		IRCMessage(const std::string &msg);
		IRCMessage(const IRCMessage &other);
		IRCMessage &operator=(const IRCMessage &other);
		~IRCMessage();

		size_t	getTokenLen(const std::string &token, size_t pos,
					const std::string &terminator_set) const throw();
		bool	isValid() const throw();

		std::string					prefix;
		std::string 				command;
		std::vector<std::string>	params;

	private:
		bool	is_valid;
};

#endif