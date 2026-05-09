GREEN = \033[1;32m
RED = \033[1;31m
RESET = \033[0m

CXX = c++
CXXFLAGS = -g -Wall -Wextra -Werror -std=c++98

NAME = ircserv

DIR_MAIN = main
DIR_SRC  = src
FILES = \
	$(DIR_MAIN)/main.cpp \
	$(DIR_SRC)//server/server.cpp \
	$(DIR_SRC)/server/clean.cpp \
	$(DIR_SRC)/server/accept.cpp \
	$(DIR_SRC)/server/bye.cpp \
	$(DIR_SRC)/server/desconnect.cpp \
	$(DIR_SRC)/server/extract.cpp \
	$(DIR_SRC)/server/HandleEvent.cpp \
	$(DIR_SRC)/server/HandleSignals.cpp \
	$(DIR_SRC)/server/receve.cpp \
	$(DIR_SRC)/utils/utls.cpp \
	$(DIR_SRC)/authentication/authentication.cpp \
	$(DIR_SRC)/channels/channel_manager.cpp \
	$(DIR_SRC)/channels/channel_join_part.cpp \
	$(DIR_SRC)/channels/channel_message.cpp \
	$(DIR_SRC)/channels/channel_modes.cpp \
	$(DIR_SRC)/channels/channel_operators.cpp \

OBJS = $(FILES:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	@$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)
	@echo "$(GREEN)Build successful! $(NAME) is ready.$(RESET)"
# 	./$(NAME) 4444 0000
%.o: %.cpp
	@$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@rm -f $(OBJS)
	@echo "$(RED)Cleaned object files.$(RESET)"

fclean: clean
	@rm -f $(NAME)
	@echo "$(RED)Cleaned everything.$(RESET)"

re: fclean all

.PHONY: all clean fclean re