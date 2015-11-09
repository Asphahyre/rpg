##
## Makefile for j 10 in /home/boulag_l/rendu/Piscine_C_J10/do-op
## 
## Made by Luka Boulagnon
## Login   <boulag_l@epitech.net>
## 
## Started on  Mon Oct 12 17:35:10 2015 Luka Boulagnon
## Last update Mon Nov 09 17:09:14 2015 Asphähyre
##

NAME =	rpg

P_SRC =	./

SRCS =	main.c

SRV	= server.c

OBJS =	$(SRCS:.c=.o)

CC =	gcc

FLAGS =	-lsfml-audio -lsfml-graphics -lsfml-window -lsfml-system -lstdc++ -ldl -lm -L/home/hazard_v/.froot/lib -lpthread

RM =	rm -f

$(NAME):
	cd $(P_SRC) ; $(CC) $(FLAGS) $(SRCS) -llapin -o ./$(NAME)
	cd $(P_SRC) ; $(CC) $(FLAGS) $(SRV) -llapin -o ./$(NAME).srv

all:	$(NAME)

clean:
	cd $(P_SRC) ; $(RM) $(OBJS)
	cd $(P_DBG) ; $(RM) $(OBJS)

fclean:	clean
	$(RM) $(NAME)

re:	fclean all

gyver:
	sl
