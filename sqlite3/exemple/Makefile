##
##
##
##

CC		=	gcc
NAME		=	poc

SRCS		=	main.c

OBJS		=	$(SRCS:.c=.o)

CFLAGS		+=	-W -Wall -Wextra
CFLAGS		+=	-g

LDFLAGS		=	-lsqlite3

RM		=	rm -rf

all		:	$(NAME)

$(NAME)		:	$(OBJS)
			$(CC) -o $(NAME) $(OBJS) $(LDFLAGS)

clean		:
			$(RM) $(NAME)

fclean		:	clean
			$(RM) $(OBJS)

re		:	fclean all

.PHONY		:	all clean fclean re
