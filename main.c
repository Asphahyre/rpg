/*
** main.c for Liblapin in /home/boulag_l/rendu/LibLapin
**
** Made by Luka Boulagnon
** Login   <boulag_l@epitech.net>
**
** Started on  Mon Nov 02 12:58:49 2015 Luka Boulagnon
** Last update Mon Nov 09 09:30:10 2015 Asphähyre
*/


#include "lapin.h"
#include "lapin_enum.h"
#include "my.h"
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

void error(char *msg)
{
    perror(msg);
    exit(0);
}

void		delete(chain_list *list, int player)
{
  chain_list	*before;

  while (list->id != player)
  {
    before = list;
    list = list->next;
  }
  before->next = list->next;
  free(list->elem);
  free(list);
}

void		push(chain_list *list, chain_list *next)
{
  while (list->next)
    list = list->next;
  list->next = next;
}

void			player_moved(chain_list *players, t_bunny_position pos, int player)
{
  t_bunny_position	*position;

  while (players && players->id != player)
    players = players->next;
  if (players)
  {
    position = players->elem;
    *position = pos;
  }
}

void			*get_pos(void *param)
{
  t_params		*params;
  t_packet		pck;
  chain_list		*new_player;
  t_bunny_position	*pos;
  int			readed;

  params = param;
  while (params->sockfd == 0)
    usleep(1000000/120);
  do
  {
    readed = read(params->sockfd, &pck, sizeof(t_packet));
    switch(pck.type)
    {
      case MOVE:
	if (pck.player == params->user)
	  break;
	printf("RECEIVED FROM SERVER: player %d moved\n", pck.player);
	player_moved(params->players, pck.pos, pck.player);
	break;
      case CONNECT:
	if (pck.player == params->user)
	  printf("RECEIVED FROM SERVER: I'm connected !\n");
	else
	{
	  printf("RECEIVED FROM SERVER: player %d connected\n", pck.player);
	  pos = malloc(sizeof(t_bunny_position));
	  pos->x = pck.pos.x;
	  pos->y = pck.pos.y;
	  new_player = malloc(sizeof(chain_list));
	  new_player->next = NULL;
	  new_player->everyone = params->players;
	  new_player->elem = pos;
	  new_player->id = pck.player;
	  push(params->players, new_player);
	}
	break;
      case DISCONNECT:
	printf("CLIENT %d DISCONNECTED...\n", pck.player);
	delete(params->players, pck.player);
	break;
    }
  } while (readed);
}

void		*send_pos(void *param)
{
  int			sockfd;
  int			n;
  struct sockaddr_in	serv_addr;
  struct hostent	*server;
  t_params		*params;
  t_packet		pck;
  char			buffer[256];

  params = param;
  if (params->sockfd == 0)
    {
      sockfd = socket(AF_INET, SOCK_STREAM, 0);
      if (sockfd < 0)
	error("ERROR opening socket");
      server = gethostbyname(params->server);
      if (server == NULL)
	error("ERROR, no such host\n");
      bzero((char *) &serv_addr, sizeof(serv_addr));
      serv_addr.sin_family = AF_INET;
      bcopy((char *)server->h_addr,
	   (char *)&serv_addr.sin_addr.s_addr,
	   server->h_length);
      serv_addr.sin_port = htons(params->server_port);
      if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	  error("ERROR connecting");
      params->sockfd = sockfd;
      n = write(sockfd, &(params->user), sizeof(int));
      if (n < 0)
	error("ERROR writing to socket");
    }
  pck.player = params->user;
  pck.pos = params->pos;
  n = write(params->sockfd, &pck, sizeof(pck));
  if (n < 0)
       error("ERROR writing to socket");
  if (n < 0)
       error("ERROR reading from socket");
  return (NULL);
}

void		*move_player(void *arg)
{
  t_params	*params;
  int		i;
  pthread_t	send;
  int		movex;
  int		movey;

  params = arg;
  while (params->move)
  {
    if (params->move_player[0])
      movex = (params->move_player[0] > 0) ? 1 : -1;
    else
      movex = 0;
    if (params->move_player[1])
      movey = (params->move_player[1] > 0) ? 1 : -1;
    else
      movey = 0;
    i = 0;
    while ((i < 50) && (movex || movey))
      {
	i++;
	params->pos.x = params->pos.x + movex;// * params->move_multiplier;
	params->pos.y = params->pos.y + movey;// * params->move_multiplier;
	pthread_create(&send, NULL, send_pos, params);
	usleep(1000000/600);
      }
    if (movex || movey)
    movex = movey = 0;
  }
}

t_bunny_response	truc_pressed(t_bunny_event_state event, t_bunny_keysym key,  void *param)
{
  t_params	*params;

  params = param;
  switch (key)
  {
    case 25:
      params->move_player[1] = -(event == GO_DOWN);
      break;
    case 16:
      params->move_player[0] = -(event == GO_DOWN);
      break;
    case 18:
      params->move_player[1] = (event == GO_DOWN);
      break;
    case 3:
      params->move_player[0] = (event == GO_DOWN);
      break;
  }
  return (GO_ON);
}

void			disp_players(t_bunny_buffer *buffer, t_bunny_clipable *p, chain_list *players)
{
  t_bunny_position	*pos;

  while (players = players->next)
  {
    pos = players->elem;
    bunny_blit(buffer, p, pos);
  }
}

t_bunny_response	loop(void *param)
{
  t_bunny_position	position;
  t_bunny_pixelarray	*p;
  t_bunny_window	*win;
  t_params		*params;

  params = param;
  win = params->win;
  p = params->pa;
  bunny_blit(&win->buffer, &params->black->clipable, &(params->origin));
  bunny_blit(&win->buffer, &p->clipable, &(params->pos));
  disp_players(&win->buffer, bunny_load_picture("pichu.png"), params->players);
  bunny_set_key_response(&truc_pressed);
  bunny_display(win);
  usleep(1000000/60);
  return (GO_ON);
}

int     main()
{
  t_params		params;
  t_bunny_window	*w;
  t_bunny_position	position;
  t_bunny_position	origin;
  t_bunny_pixelarray	*p;
  t_bunny_pixelarray	*black;
  int			*speed;
  pthread_t		moving;
  pthread_t		get_data;
  chain_list		*players;

  position.x = 200;
  position.y = 100;
  origin.x = 0;
  origin.y = 0;
  w = bunny_start(800, 600, 0, "bonjour");
  p = bunny_new_pixelarray(50, 50);
  black = bunny_new_pixelarray(800, 600);
  set_black(black);
  bunny_set_loop_main_function(&loop);
  bunny_set_key_response(&truc_pressed);

  srand(time(NULL));
  players = malloc(sizeof(chain_list));
  players->next = NULL;
  players->elem = NULL;
  players->loop = 0;
  players->everyone = players;
  speed = (malloc(2 * sizeof(int)));
  speed[0] = 0;
  speed[1] = 0;
  params.win = w;
  params.pos = position;
  params.pa = p;
  params.black = black;
  params.origin = origin;
  params.move = 1;
  params.move_player = speed;
  params.move_multiplier = 3;
  params.server = "localhost";
  params.server_port = 8080;
  params.sockfd = 0;
  params.user = rand();
  params.players = players;

  send_pos(&params);
  pthread_create(&get_data, NULL, get_pos, &params);
  pthread_create(&moving, NULL, move_player, &params);
  bunny_loop(w, 255, &params);

  params.move = 0;
  close(params.sockfd);
  params.move = 0;
  bunny_stop(w);
}
