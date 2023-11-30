NAME := ft_irc

DIR_SRC := .
DIR_INCL := .
DIR_OBJ := obj

CXX := c++
CXX_FLAGS := -Wall -Wextra -Werror -std=c++98 -I$(DIR_INCL) -g -fsanitize=address

SRC_FILES := 	ft_irc.cpp			\
				IRCChannel.cpp		\
				IRCMessage.cpp		\
				IRCServer.cpp		\
				IRCUser.cpp			\
				TCPConn.cpp			\
				TCPHost.cpp

SRC := $(addprefix $(DIR_SRC)/, SRC_FILES)
OBJ := $(addprefix $(DIR_OBJ)/, $(patsubst %.cpp, %.o, $(SRC_FILES)))

all: $(NAME) $(OBJ)

clean:
	@rm -rf $(DIR_OBJ)

fclean:clean
	@rm -f $(NAME)

re:fclean all

$(NAME): $(DIR_OBJ) $(OBJ)
	$(CXX) -o $@ $(OBJ) $(CXX_FLAGS)

$(OBJ): | $(DIR_OBJ)

$(DIR_OBJ)/%.o: $(DIR_SRC)/%.cpp
	$(CXX) $(CXX_FLAGS) -c -o $@ $<

$(DIR_OBJ):
	@mkdir $(DIR_OBJ)

.PHONY:all clean fclean re
