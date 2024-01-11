NAME	:= webserv

CC		:=	c++
CPPFLAGS:=	-Wall -Wextra -Werror
CPPFLAG	:=	-std=c++98

SRCS	:=	webserv.cpp srcs/conf_reader.cpp srcs/conf_parser.cpp srcs/location_storage.cpp srcs/server_storage.cpp srcs/help.cpp multiserver/server.cpp multiserver/Get_handling.cpp multiserver/Request.cpp multiserver/StatusCodes.cpp multiserver/files.cpp  multiserver/Post.cpp srcs/cgi_srcs/cgi_conf.cpp
INCS	:=	incs/parse_config/
OBJS	:=	$(SRCS:.cpp=.o)

%.o:%.cpp
	$(CC) -I $(INCS) $(CPPFLAGS) $(CPPFLAG) -c $< -o $@

all:		$(NAME)

$(NAME): $(OBJS) $(INCS)
	$(CC) $(OBJS) -I $(INCS) $(CPPFLAGS) $(CPPFLAG) -o $@

clean:
	rm -rf $(OBJS)

fclean:		clean
	rm -rf $(NAME)

re:			fclean all

.PHONY:		all clean fclean re
