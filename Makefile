CC = c++
CPPFLAGS = -std=c++98 -Wall -Wextra -Werror -pedantic

SRCS = main.cpp 

OBJS = $(SRCS:.cpp=.o)
RM = rm -f
NAME = webserv

all: $(NAME)

%.o : %.cpp
	$(CC) $(CPPFLAGS) -c $<

$(NAME): $(OBJS)
	$(CC) $(CPPFLAGS) $(OBJS) -o $(NAME)

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)

re: fclean $(NAME)

.PHONY: all clean fclean re