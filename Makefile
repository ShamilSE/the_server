
VPATH	=	srcs srcs/utils includes

SRCS	=	main.cpp \
			WebServ.cpp \
			Server.cpp \
			Client.cpp \
			Request.cpp \
			Response.cpp \
			ft_strtrim.cpp \
			ft_split.cpp \
			mapToCharArray.cpp \
			ft_strdup.cpp \
			ft_strjoin.cpp

HEADERS	=	WebServ.hpp \
			Server.hpp \
			Client.hpp \
			Request.hpp \
			Response.hpp \
			utils.hpp \

NAME	=	webserv

CC		=	clang++

CFLAGS	=	-Wall -Wextra -Werror -std=c++98 -g

# CFLAGS   +=   -g -Wshadow -Wno-shadow -Wshadow-uncaptured-local \
#          -Waddress -Wchar-subscripts -Wuninitialized -Wpedantic \
#          -Wnon-virtual-dtor -Wsign-promo

#  Компилятор clang имеет опцию -Wpadded, которая будет генерировать сообщения о заполнении.
# -Waddress : Предупреждает о странной работе с адресами. Например, сравнение строкового литерала с адресом
# -Wchar-subscripts : Предупреждает об обращении к массиву по индексу, тип которого char. А ведь char является знаковым на многих машинах
# -Wmisleading-indentation : Предупреждает о том, что отступы в коде не отражают структуру этого кода (Не работает на школьном маке)
# -Wuninitialized : Предупреждает об использовании переменных и членов класса, которые не были проинициализированы
# -Wpedantic : проверяет соответствие кода стандарту ISO C++, сообщает об использовании запрещённых расширений, о наличии лишних точек с запятой, нехватке переноса строки в конце файла и прочих полезных штуках.
# -Wnon-virtual-dtor : Предупреждает о том, что у класса есть виртуальные функции-члены, но деструктор при этом не виртуальный
# -Wsign-promo : Предупреждает о неочевидном выборе перегруженной функции

OBJS_DIR	=	.obj

OBJS	=	$(addprefix $(OBJS_DIR)/, $(patsubst %.cpp, %.o, $(SRCS)))

all:	$(NAME)

run:	all
		./${NAME}

$(NAME):	$(OBJS)
			@printf "$(GREEN)$(BOLD)Compiling $(NO_COLOR)$(UNDER_LINE)$(BOLD)$(NAME)$(NO_COLOR)$(BOLD)  ––  "
			@$(CC) $(CFLAGS) $(OBJS) -o $(NAME) && printf "$(LIGHT_GREEN)$(BOLD)[Success]" || \
			printf "$(RED)$(BOLD)[Failure]"
			@printf "$(NO_COLOR)\n"

$(OBJS_DIR)/%.o:	%.cpp ${HEADERS}
					@test -d $(OBJS_DIR) || mkdir $(OBJS_DIR)
					@printf "$(GREEN)$(BOLD)Compiling $(UNDER_LINE)$(YELLOW)$<$(NO_COLOR)  $(BOLD)–– "
					@printf "$(RED)[KO]$(NO_COLOR)"
					@$(CC) -g $(CFLAGS) -c $< -o $@ && printf "\b\b\b\b$(GREEN)[OK]$(NO_COLOR)\n" || printf "\n"


clean:
			@/bin/rm -rf $(OBJS_DIR)

fclean:		clean
			@rm -rf $(NAME)
			@printf "$(UNDER_LINE)$(BOLD)$(NAME)$(NO_COLOR) $(LIGHT_RED)deleted$(NO_COLOR)\n"

re:			fclean all

################
##   COLORS   ##
################

BOLD       = \033[1m
NO_COLOR    = \033[0m
UNDER_LINE   = \033[4m

BLACK       = \033[0;30m
RED        = \033[0;31m
GREEN        = \033[0;32m
BROWN        = \033[0;33m
BLUE       = \033[0;34m
PURPLE        = \033[0;35m
CYAN        = \033[0;36m
LIGHT_GRAY   = \033[0;37m

DARK_GRAY   = \033[1;30m
LIGHT_RED   = \033[1;31m
LIGHT_GREEN   = \033[1;32m
YELLOW       = \033[1;33m
LIGHT_BLUE   = \033[1;34m
LIGHT_PURPLE= \033[1;35m
LIGHT_CYAN   = \033[1;36m
WHITE       = \033[1;37m

.PHONY:	all clean fclean re
.PHONY:	SRCS NAME HEADER CC CFLAGS OBJS_DIR OBJS
.PHONY:	UNDER_LINE BOLD NO_COLOR BLACK RED GREEN BROWN BLUE PURPLE CYAN LIGHT_GRAY DARK_GRAY LIGHT_RED LIGHT_GREEN YELLOW LIGTH_BLUE LIGHT_PURPLE LIGHT_CYAN WHITE
