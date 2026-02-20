CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

NAME = ircserv

DIR_MAIN = main
DIR_SRC  = src

FILES = \
	$(DIR_MAIN)/main.cpp \
	$(DIR_SRC)/server.cpp\
	$(DIR_SRC)/client.cpp \

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
