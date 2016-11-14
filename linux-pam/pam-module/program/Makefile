CC		=	gcc

NAME		=	dumb_test

_SRCS		=	main.c \
			print_usage.c \
			print_current_groups.c

SRCS		=	$(addprefix src/, $(_SRCS))

OBJS		=	$(SRCS:.c=.o)

CFLAGS		+=	-W -Wall -Wextra
CFLAGS		+=	-g
CFLAGS		+=	-I .

LDFLAGS		=	-lpam -lpam_misc

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
