CC       = gcc
CFLAGS   = -Wall -Wextra -Werror -std=gnu11
NAME     = db
SRC_DIR  = src
SRCS     = $(wildcard $(SRC_DIR)/*.c)
OBJS     = $(SRCS:.c=.o)
HEADERS  = $(wildcard $(SRC_DIR)/*.h)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS)

$(OBJS): $(HEADERS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
