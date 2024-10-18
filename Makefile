NAME 			=	ircserv
SRC_DIR 		=	./srcs/
INC_DIR 		=	./includes/
SRCS 			=	main.cpp\
					server.cpp\
					client.cpp\
					utils.cpp\
					channel.cpp\
				#	commands.cpp\


SRC_BASENAME    =   $(addprefix $(SRC_DIR), $(SRCS))
OBJS			=	${SRC_BASENAME:.cpp=.o}


FLAGS 			= 	-Wall -Wextra -Werror -std=c++98 -g3 -I $(INC_DIR)

%.o	:			%.cpp
				c++ $(FLAGS) -c $< -o $@

all	:			$(NAME)

$(NAME)	:		$(OBJS)
				c++ $(FLAGS) -o $(NAME) $(OBJS)

clean:
				rm -rf ${OBJS}

fclean:			clean
				rm -rf ${NAME}

re:				fclean all

.PHONY:			all clean fclean re
