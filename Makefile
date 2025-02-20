NAME = webserv

CC = c++

CFLAGS = -Wall -Werror -Wextra -Iincl -g3 -std=c++98 -fsanitize=address

RM = rm -f


SRC = src/ConfigParser.cpp \
	src/ServerManager.cpp \
	src/utils.cpp \
	src/main.cpp

OBJTS = $(SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJTS)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJTS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJTS)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re