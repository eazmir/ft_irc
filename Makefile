CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

NAME = ircserv

DIR_MAIN = main
DIR_SRC  = src

FILES = \
	$(DIR_MAIN)/main.cpp \
	$(DIR_SRC)/server.cpp\
	$(DIR_SRC)/utls.cpp\
	$(DIR_SRC)/authentication.cpp\
	$(DIR_SRC)/channels/channel_manager.cpp\
	$(DIR_SRC)/channels/channel_join_part.cpp\
	$(DIR_SRC)/channels/channel_message.cpp\
	$(DIR_SRC)/channels/channel_modes.cpp\
	$(DIR_SRC)/channels/channel_operators.cpp\

OBJS = $(FILES:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)
	./$(NAME) 4444 0000
clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re