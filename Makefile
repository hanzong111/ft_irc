NAME := ft_irc

DIR_SRC := .
DIR_INCL := .
DIR_OBJ := obj

CXX := c++
CXX_FLAGS := -Wall -Wextra -Werror -std=c++98 -I$(DIR_INCL) -g -fsanitize=address

SHARED_SRC_FILES := 	IRCChannel.cpp		\
						IRCMessage.cpp		\
						IRCServer.cpp		\
						IRCUser.cpp			\
						TCPConn.cpp			\
						TCPHost.cpp			\
						IRCServer_cmd.cpp	\
						IRCChannel_cmd.cpp	\
						Channel_flags.cpp

SERVER_SRC_FILES := ft_irc.cpp
SERVER_SRC_FILES += $(SHARED_SRC_FILES)

BOT_SRC_FILES := 	irc_bot.cpp \
					IRCBot.cpp
BOT_SRC_FILES += $(SHARED_SRC_FILES)

SERVER_SRC := $(addprefix $(DIR_SRC)/, SERVER_SRC_FILES)
SERVER_OBJ := $(addprefix $(DIR_OBJ)/, $(patsubst %.cpp, %.o, $(SERVER_SRC_FILES)))

BOT_SRC := $(addprefix $(DIR_SRC)/, BOT_SRC_FILES)
BOT_OBJ := $(addprefix $(DIR_OBJ)/, $(patsubst %.cpp, %.o, $(BOT_SRC_FILES)))

all: $(NAME) bot

clean:
	@rm -rf $(DIR_OBJ)

fclean:clean
	@rm -f $(NAME)
	@rm -f irc_bot

re:fclean all

bot: $(DIR_OBJ) $(BOT_OBJ)
	$(CXX) -o irc_bot $(BOT_OBJ) $(CXX_FLAGS)

$(NAME): $(DIR_OBJ) $(SERVER_OBJ)
	$(CXX) -o $@ $(SERVER_OBJ) $(CXX_FLAGS)

$(SERVER_OBJ): | $(DIR_OBJ)

$(BOT_OBJ): | $(DIR_OBJ)

$(DIR_OBJ)/%.o: $(DIR_SRC)/%.cpp
	$(CXX) $(CXX_FLAGS) -c -o $@ $<

$(DIR_OBJ):
	@mkdir $(DIR_OBJ)

.PHONY:all clean fclean re
