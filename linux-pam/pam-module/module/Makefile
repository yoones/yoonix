##
##
##
##

CC		=	gcc

LD		=	ld

NAME		=	libdumb_test.so

_SRCS		=	helpers/discreet_display.c

_SRCS		+=	auth/pam_sm_authenticate.c \
			auth/pam_sm_setcred.c

# _SRCS		+=	account/account.c

# _SRCS		+=	password/password.c

# _SRCS		+=	session/session.c

# _SRCS		+=	conv/conv.c

SRCS		=	$(addprefix src/, $(_SRCS))

OBJS		=	$(SRCS:.c=.o)

CFLAGS		+=	-W -Wall -Wextra
CFLAGS		+=	-fPIC
CFLAGS		+=	-g
CFLAGS		+=	-I . -I include

LDFLAGS		=	-x -shared # -lcrypt -lpam_misc

RM		=	rm -rf

all		:	$(NAME)

$(NAME)		:	$(OBJS)
			$(LD) -o $(NAME) $(OBJS) $(LDFLAGS)

clean		:
			$(RM) $(NAME)

fclean		:	clean
			$(RM) $(OBJS)

re		:	fclean all

.PHONY		:	all clean fclean re
