/*
** main.c for Liblapin in /home/boulag_l/rendu/LibLapin
**
** Made by Luka Boulagnon
** Login   <boulag_l@epitech.net>
**
** Started on  Mon Nov 02 12:58:49 2015 Luka Boulagnon
** Last update Mon Nov 09 23:31:04 2015 Asphähyre
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
	  printf("RECEIVED FROM SERVER: player %d connected with skin %s\n", pck.player, pck.name);
	  pos = malloc(sizeof(t_bunny_position));
	  pos->x = pck.pos.x;
	  pos->y = pck.pos.y;
	  new_player = malloc(sizeof(chain_list));
	  new_player->next = NULL;
	  new_player->everyone = params->players;
	  new_player->elem = pos;
	  new_player->id = pck.player;
	  new_player->skin = bunny_load_picture("pichu.png");
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

char		collision(t_params *params, int x, int y)
{
  t_obstacle	*obst;
  t_bounds_s	bounds;
  int		i;

  i = 0;
  obst = params->obstacles;
  while (obst = obst->next)
  {
    bounds = obst->bounds;
    printf("Cheking bounds on %dst obstacle\n", i);
    printf("\tbounds on x1=%d\n\t\t  x2=%d\n\t\t  y1=%d\n\t\t  y2=%d\n\n", (bounds.x1 * 50 + obst->pos->x), (bounds.x2 * 50 + obst->pos->x), (bounds.y1 * 50 + obst->pos->y), (bounds.y2 * 50 + obst->pos->y));
    printf("Player on [%d, %d]\n\n", x, y);
    i++;
    if (x >= (bounds.x1 * 50 + obst->pos->x) && x < (bounds.x2 * 50 + obst->pos->x) &&
	y >= (bounds.y1 * 50 + obst->pos->y) && y < (bounds.y2 * 50 + obst->pos->y))
      return (1);
  }
  return (0);
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
    if (params->move_player[0] && !collision(params, params->pos.x + 50 * params->move_player[0], params->pos.y))
      movex = (params->move_player[0] > 0) ? 1 : -1;
    else
      movex = 0;
    if (params->move_player[1] && !collision(params, params->pos.x + 50 * movex, params->pos.y + 50 * params->move_player[1]))
      movey = (params->move_player[1] > 0) ? 1 : -1;
    else
      movey = 0;
    i = 0;
    while ((i < 50) && (movex || movey))
      {
	i++;
	params->pos.x = params->pos.x + movex;
	params->pos.y = params->pos.y + movey;
	usleep(1000000/300);
	if ((i % 5) == 0)
	  pthread_create(&send, NULL, send_pos, params);
      }
    usleep(1000000/30);
  }
}

t_bunny_response	key_pressed(t_bunny_event_state event, t_bunny_keysym key,  void *param)
{
  t_params	*params;

  params = param;
  switch (key)
  {
    case 25:
    case 73:
      if (!params->move_player[0])
        params->move_player[1] = -(event == GO_DOWN);
      break;
    case 16:
    case 71:
      if (!params->move_player[1])
	params->move_player[0] = -(event == GO_DOWN);
      break;
    case 18:
    case 74:
      if (!params->move_player[0])
	params->move_player[1] = (event == GO_DOWN);
      break;
    case 3:
    case 72:
      if (!params->move_player[1])
	params->move_player[0] = (event == GO_DOWN);
      break;
    case 8:
      params->camera.y += 9 * (event == GO_DOWN);
      break;
    case 9:
      params->camera.x += 9 * (event == GO_DOWN);
      break;
    case 10:
      params->camera.y -= 9 * (event == GO_DOWN);
      break;
    case 11:
      params->camera.x -= 9 * (event == GO_DOWN);
      break;
    default:
      printf("Key %d pressed\n");
  }
  return (GO_ON);
}

void		push_obs(t_obstacle *list, t_obstacle *next)
{
  while (list->next)
    list = list->next;
  list->next = next;
}

void			add_obstacle(t_obstacle *obstacles, t_bunny_picture *image, t_bunny_position *pos, t_bounds_s bounds)
{
  t_obstacle		*obst;

  obst = malloc(sizeof(t_obstacle));
  obst->pic = image;
  obst->pos = pos;
  obst->bounds = bounds;
  obst->next = NULL;
  printf("push_obs in %p of %p...\n", obstacles, obst);
  push_obs(obstacles, obst);
}

void			disp_players(t_bunny_buffer *buffer, t_bunny_clipable *p, chain_list *players)
{
  t_bunny_position	*pos;

  while (players = players->next)
  {
    pos = players->elem;
    bunny_blit(buffer, players->skin, pos);
  }
}

t_bunny_position	*get_pos_by_coords(int x, int y)
{
  t_bunny_position	*to_return;

  to_return = malloc(sizeof(t_bunny_position));
  to_return->x = 50 * x;
  to_return->y = 50 * y;
  return (to_return);
}

t_bunny_position	*camera(t_bunny_position camera, t_bunny_position *pos)
{
  t_bunny_position	*result;

  result = malloc(sizeof(t_bunny_position));
  result->x = camera.x + pos->x;
  result->y = camera.y + pos->y;
  return (result);
}

void			render_obst(t_bunny_buffer *buff, t_obstacle *obst, t_bunny_picture *pic, t_bunny_position cam)
{
  while (obst && (obst = obst->next))
    bunny_blit(buff, obst->pic, camera(cam, obst->pos));
}

void			render_wheed(t_bunny_buffer *buff, t_bunny_picture *wheed, t_bunny_position cam)
{
  int			x;
  int			y;
  t_bunny_position	*pos;

  x = 0;
  while (x < 20)
  {
    y = 0;
    while (y < 20)
    {
      pos = camera(cam, get_pos_by_coords(x, y));
      bunny_blit(buff, wheed, pos);
      free(pos);
      y++;
    }
    x++;
  }
}

t_bunny_response	loop(void *param)
{
  t_bunny_position	position;
  t_bunny_pixelarray	*p;
  t_bunny_window	*win;
  t_params		*params;
  t_bunny_position	*pos;

  params = param;
  win = params->win;
  p = params->pa;
  bunny_blit(&win->buffer, &params->black->clipable, &params->origin);
  render_wheed(&win->buffer, params->wheed, params->camera);
  bunny_blit(&win->buffer, params->skin, camera(params->camera, &params->pos));
  disp_players(&win->buffer, &params->black->clipable, params->players);
  render_obst(&win->buffer, params->obstacles, params->tree, params->camera);
  bunny_set_key_response(&key_pressed);
  bunny_display(win);
  usleep(1000000/60);
  return (GO_ON);
}

int     main(int argc, char **argv)
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
  char			*server;
  t_bounds_s		bounds;

  if (argc < 2)
    server = "localhost";
  else
    server = argv[1];
  position.x = 600;
  position.y = 100;
  origin.x = 0;
  origin.y = 0;
  w = bunny_start(800, 600, 0, "bonjour");
  p = bunny_new_pixelarray(50, 50);
  black = bunny_new_pixelarray(800, 600);
  bunny_set_loop_main_function(&loop);
  bunny_set_key_response(&key_pressed);

  srand(time(NULL));
  players = malloc(sizeof(chain_list));
  players->next = NULL;
  players->elem = NULL;
  players->everyone = players;
  speed = (malloc(2 * sizeof(int)));
  speed[0] = 0;
  speed[1] = 0;
  params.win = w;
  params.pos = position;
  params.camera = origin;
  params.pa = p;
  params.black = black;
  params.origin = origin;
  params.move = 1;
  params.move_player = speed;
  params.move_multiplier = 3;
  params.server = server;
  params.server_port = 8080;
  params.sockfd = 0;
  params.user = rand();
  params.players = players;
  params.skin = bunny_load_picture("textures/players/player_f_f_0.png");
  params.tree = bunny_load_picture("textures/tree.png");
  params.wheed = bunny_load_picture("textures/wheed.png");
  params.obstacles = malloc(sizeof(t_obstacle));
  params.obstacles->next = NULL;

  bounds.x1 = 0;
  bounds.y1 = 2;
  bounds.x2 = 2;
  bounds.y2 = 3;
  add_obstacle(params.obstacles, params.tree, get_pos_by_coords(14, -1), bounds);
  add_obstacle(params.obstacles, params.tree, get_pos_by_coords(14, 0), bounds);
  add_obstacle(params.obstacles, params.tree, get_pos_by_coords(14, 1), bounds);
  add_obstacle(params.obstacles, params.tree, get_pos_by_coords(14, 2), bounds);
  add_obstacle(params.obstacles, params.tree, get_pos_by_coords(14, 3), bounds);
  add_obstacle(params.obstacles, params.tree, get_pos_by_coords(14, 4), bounds);
  add_obstacle(params.obstacles, params.tree, get_pos_by_coords(14, 5), bounds);
  add_obstacle(params.obstacles, params.tree, get_pos_by_coords(14, 6), bounds);
  add_obstacle(params.obstacles, params.tree, get_pos_by_coords(14, 7), bounds);
  add_obstacle(params.obstacles, params.tree, get_pos_by_coords(14, 8), bounds);
  add_obstacle(params.obstacles, params.tree, get_pos_by_coords(14, 9), bounds);
  add_obstacle(params.obstacles, params.tree, get_pos_by_coords(0, -1), bounds);
  add_obstacle(params.obstacles, params.tree, get_pos_by_coords(0, 0), bounds);
  add_obstacle(params.obstacles, params.tree, get_pos_by_coords(0, 1), bounds);
  add_obstacle(params.obstacles, params.tree, get_pos_by_coords(0, 2), bounds);
  add_obstacle(params.obstacles, params.tree, get_pos_by_coords(0, 3), bounds);
  add_obstacle(params.obstacles, params.tree, get_pos_by_coords(0, 4), bounds);
  add_obstacle(params.obstacles, params.tree, get_pos_by_coords(0, 5), bounds);
  add_obstacle(params.obstacles, params.tree, get_pos_by_coords(0, 6), bounds);
  add_obstacle(params.obstacles, params.tree, get_pos_by_coords(0, 7), bounds);
  add_obstacle(params.obstacles, params.tree, get_pos_by_coords(0, 8), bounds);
  add_obstacle(params.obstacles, params.tree, get_pos_by_coords(0, 9), bounds);

  send_pos(&params);
  pthread_create(&get_data, NULL, get_pos, &params);
  pthread_create(&moving, NULL, move_player, &params);
  bunny_loop(w, 255, &params);

  params.move = 0;
  close(params.sockfd);
  params.move = 0;
  bunny_stop(w);
}
