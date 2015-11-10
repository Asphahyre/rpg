/*
** main.c for Liblapin in /home/boulag_l/rendu/LibLapin
**
** Made by Luka Boulagnon
** Login   <boulag_l@epitech.net>
**
** Started on  Mon Nov 02 12:58:49 2015 Luka Boulagnon
** Last update Tue Nov 10 22:06:55 2015 Asphähyre
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

void		delete_plyr(t_players *list, int player)
{
  t_players	*before;

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

void			player_moved(t_players *players, t_bunny_position pos, int player)
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

void		push_plyr(t_players *list, t_players *next)
{
  while (list->next)
    list = list->next;
  list->next = next;
}

void			*get_pos(void *param)
{
  t_params		*params;
  t_packet		pck;
  t_players		*new_player;
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
	  new_player = malloc(sizeof(t_players));
	  new_player->next = NULL;
	  new_player->everyone = params->players;
	  new_player->elem = pos;
	  new_player->id = pck.player;
	  new_player->orientation = FACE;
	  new_player->skin = bunny_load_picture("pichu.png");
	  push_plyr(params->players, new_player);
	}
	break;
      case DISCONNECT:
	printf("CLIENT %d DISCONNECTED...\n", pck.player);
	delete_plyr(params->players, pck.player);
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
  t_bounds	bounds;
  int		i;

  i = 0;
  obst = params->obstacles;
  while (obst = obst->next)
  {
    bounds = obst->bounds;
    i++;
    if (bounds.type == 0)
    {
      if (x >= (bounds.x1 * 50 + obst->pos->x) && x < (bounds.x2 * 50 + obst->pos->x) &&
	  y >= (bounds.y1 * 50 + obst->pos->y) && y < (bounds.y2 * 50 + obst->pos->y))
	return (1);
    }
    else
    {
      printf("Player in pos :\n\t\tx = %d\n\t\ty = %d\n\n", x, y);
      printf("\tbounds.gate_x1 = %d\n\tbounds.gate_x2 = %d\n\tbounds.gate_y1 = %d\n\tbounds.gate_y2 = %d\n", bounds.gate_x1 * 50 + obst->pos->x, bounds.gate_x2 * 50 + obst->pos->x, bounds.gate_y1 * 50 + obst->pos->x, bounds.gate_y2 * 50 + obst->pos->x);
      if ((x < (bounds.gate_x1 * 50 + obst->pos->x) || x >= (bounds.gate_x2 * 50 + obst->pos->x) ||
	    y < (bounds.gate_y1 * 50 + obst->pos->x) || y >= (bounds.gate_y2 * 50 + obst->pos->y)))
	if (x >= (bounds.x1 * 50 + obst->pos->x) && x < (bounds.x2 * 50 + obst->pos->x) &&
	    y >= (bounds.y1 * 50 + obst->pos->y) && y < (bounds.y2 * 50 + obst->pos->y))
	  return (1);
    }
  }
  return (0);
}

void		move_camera(t_bunny_position *cam, t_bunny_position player)
{
  if (player.y > (-cam->y - 210 + 600))
    cam->y -= 1;
  if (player.x > (-cam->x - 210 + 800))
    cam->x -= 1;
  if (player.y < (-cam->y + 210))
    cam->y += 1;
  if (player.x < (-cam->x + 210))
    cam->x += 1;
}

void		*move_player(void *arg)
{
  t_params	*params;
  int		i;
  pthread_t	send;
  int		movex;
  int		movey;

  params = arg;
  i = 0;
  while (params->move)
  {
    movex = movey = 0;
    if (params->move_player[0] && !params->move_player[1] &&
	!collision(params, params->pos.x + 50 * params->move_player[0], params->pos.y))
      movex = (params->move_player[0] > 0) ? 1 : -1;
    else
      movex = 0;
    if (params->move_player[1] &&
	!collision(params, params->pos.x + 50 * movex, params->pos.y + 50 * params->move_player[1]))
      movey = (params->move_player[1] > 0) ? 1 : -1;
    else
      movey = 0;
    if (!(movex + movey))
      params->step = 0;
    i = 0;
    while ((i != 50) && (movex || movey))
      {
	if (i == 0)
	{
	  if (params->move_player[0] > 0)
	    params->orientation = RIGH;
	  else if (params->move_player[0] < 0)
	    params->orientation = LEFT;
	  if (params->move_player[1] > 0)
	    params->orientation = FACE;
	  else if (params->move_player[1] < 0)
	    params->orientation = BACK;
	}
	i++;
	params->pos.x = params->pos.x + movex;
	params->pos.y = params->pos.y + movey;
	usleep(1000000/(150 * (params->run + 1)));
	if ((i % 5) == 0)
	  pthread_create(&send, NULL, send_pos, params);
	if ((i % 25) == 0)
	  params->step = (1 + params->step) % 3;
	move_camera(&params->camera, params->pos);
      }
    usleep(100000/30);
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
      params->move_player[1] = -(event == GO_DOWN);
      break;
    case 16:
    case 71:
      params->move_player[0] = -(event == GO_DOWN);
      break;
    case 18:
    case 74:
      params->move_player[1] = (event == GO_DOWN);
      break;
    case 3:
    case 72:
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
    case 01:
      params->run = (event == GO_DOWN);
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

void			add_obstacle(t_obstacle *obstacles, t_bunny_picture *image, t_bunny_position *pos, t_bounds bounds)
{
  t_obstacle		*obst;

  obst = malloc(sizeof(t_obstacle));
  obst->pic = image;
  obst->pos = pos;
  obst->bounds = bounds;
  obst->next = NULL;
  push_obs(obstacles, obst);
}

t_bunny_position	*camera(t_bunny_position camera, t_bunny_position *pos)
{
  t_bunny_position	*result;

  result = malloc(sizeof(t_bunny_position));
  result->x = camera.x + pos->x;
  result->y = camera.y + pos->y;
  return (result);
}

void			disp_players(t_bunny_buffer *buffer, t_bunny_clipable *p, t_players *players, t_bunny_position cam)
{
  t_bunny_position	*pos;

  while (players = players->next)
  {
    pos = players->elem;
    bunny_blit(buffer, players->skin, camera(cam, pos));
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

  x = -20;
  while (x < 40)
  {
    y = -20;
    while (y < 40)
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
  bunny_blit(&win->buffer, params->skin[3 * params->orientation + params->step], camera(params->camera, &params->pos));
  render_obst(&win->buffer, params->obstacles, params->tree, params->camera);
  disp_players(&win->buffer, &params->black->clipable, params->players, params->camera);
  bunny_set_key_response(&key_pressed);
  bunny_display(win);
  usleep(1000000/60);
  return (GO_ON);
}

char	*plyr_pic_get_path(int sex, int orientation, int step)
{
  char	*prefix;
  char	*separator = "_";
  char	orient;

  switch (orientation)
  {
    case FACE:
      orient = 'f';
      break;
    case LEFT:
      orient = 'l';
      break;
    case RIGH:
      orient = 'r';
      break;
    case BACK:
      orient = 'b';
  }
  prefix = strdup("textures/players/player_X_X_X.png");
  prefix[24] = 'f';
  prefix[26] = orient;
  prefix[28] = step + '0';

  return (prefix);
}

int     main(int argc, char **argv)
{
  t_params		params;
  t_bunny_window	*w;
  t_bunny_position	position;
  t_bunny_position	origin;
  t_bunny_music		*music;
  t_bunny_pixelarray	*p;
  t_bunny_pixelarray	*black;
  int			*speed;
  pthread_t		moving;
  pthread_t		get_data;
  t_players		*players;
  char			*server;
  t_bounds		bounds;

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
  players = malloc(sizeof(t_players));
  players->next = NULL;
  players->elem = NULL;
  players->everyone = players;
  speed = (malloc(2 * sizeof(int)));
  speed[0] = 0;
  speed[1] = 0;
  params.win = w;
  params.run = 0;
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
  params.skin[0] = bunny_load_picture(plyr_pic_get_path(FEMALE, FACE, 0));
  params.skin[1] = bunny_load_picture(plyr_pic_get_path(FEMALE, FACE, 1));
  params.skin[2] = bunny_load_picture(plyr_pic_get_path(FEMALE, FACE, 2));
  params.skin[3] = bunny_load_picture(plyr_pic_get_path(FEMALE, LEFT, 0));
  params.skin[4] = bunny_load_picture(plyr_pic_get_path(FEMALE, LEFT, 1));
  params.skin[5] = bunny_load_picture(plyr_pic_get_path(FEMALE, LEFT, 2));
  params.skin[6] = bunny_load_picture(plyr_pic_get_path(FEMALE, RIGH, 0));
  params.skin[7] = bunny_load_picture(plyr_pic_get_path(FEMALE, RIGH, 1));
  params.skin[8] = bunny_load_picture(plyr_pic_get_path(FEMALE, RIGH, 2));
  params.skin[9] = bunny_load_picture(plyr_pic_get_path(FEMALE, BACK, 0));
  params.skin[10] = bunny_load_picture(plyr_pic_get_path(FEMALE, BACK, 1));
  params.skin[11] = bunny_load_picture(plyr_pic_get_path(FEMALE, BACK, 2));
  params.orientation = FACE;
  params.tree = bunny_load_picture("textures/tree.png");
  params.wheed = bunny_load_picture("textures/wheed.png");
  params.obstacles = malloc(sizeof(t_obstacle));
  params.obstacles->next = NULL;

  bounds.type = 0;
  bounds.x1 = 0;
  bounds.y1 = 1;
  bounds.x2 = 2;
  bounds.y2 = 3;
  add_obstacle(params.obstacles, params.tree, get_pos_by_coords(8, -1), bounds);
  add_obstacle(params.obstacles, params.tree, get_pos_by_coords(14, -1), bounds);
  add_obstacle(params.obstacles, params.tree, get_pos_by_coords(10, -1), bounds);
  add_obstacle(params.obstacles, params.tree, get_pos_by_coords(14, -1), bounds);
  add_obstacle(params.obstacles, params.tree, get_pos_by_coords(12, -1), bounds);
  add_obstacle(params.obstacles, params.tree, get_pos_by_coords(14, 1), bounds);
  add_obstacle(params.obstacles, params.tree, get_pos_by_coords(14, 3), bounds);
  add_obstacle(params.obstacles, params.tree, get_pos_by_coords(14, 5), bounds);
  add_obstacle(params.obstacles, params.tree, get_pos_by_coords(14, 7), bounds);
  add_obstacle(params.obstacles, params.tree, get_pos_by_coords(14, 9), bounds);
  add_obstacle(params.obstacles, params.tree, get_pos_by_coords(14, 11), bounds);
  add_obstacle(params.obstacles, params.tree, get_pos_by_coords(12, 11), bounds);
  add_obstacle(params.obstacles, params.tree, get_pos_by_coords(10, 11), bounds);
  add_obstacle(params.obstacles, params.tree, get_pos_by_coords(8, 11), bounds);
  add_obstacle(params.obstacles, params.tree, get_pos_by_coords(6, 11), bounds);
  add_obstacle(params.obstacles, params.tree, get_pos_by_coords(4, 11), bounds);
  add_obstacle(params.obstacles, params.tree, get_pos_by_coords(2, 11), bounds);
  add_obstacle(params.obstacles, params.tree, get_pos_by_coords(0, 1), bounds);
  add_obstacle(params.obstacles, params.tree, get_pos_by_coords(0, 3), bounds);
  add_obstacle(params.obstacles, params.tree, get_pos_by_coords(0, 5), bounds);
  add_obstacle(params.obstacles, params.tree, get_pos_by_coords(0, 7), bounds);
  add_obstacle(params.obstacles, params.tree, get_pos_by_coords(0, 9), bounds);
  add_obstacle(params.obstacles, params.tree, get_pos_by_coords(2, 1), bounds);
  add_obstacle(params.obstacles, params.tree, get_pos_by_coords(8, 3), bounds);
  add_obstacle(params.obstacles, params.tree, get_pos_by_coords(8, 5), bounds);

  bounds.type = 1;
  bounds.x1 = 0;
  bounds.y1 = 1;
  bounds.x2 = 5;
  bounds.y2 = 5;
  bounds.gate_x1 = 2;
  bounds.gate_y1 = 4;
  bounds.gate_x2 = 3;
  bounds.gate_y2 = 5;
  add_obstacle(params.obstacles, bunny_load_picture("textures/pokecentre.png"), get_pos_by_coords(3, 3), bounds);

  send_pos(&params);
  pthread_create(&get_data, NULL, get_pos, &params);
  pthread_create(&moving, NULL, move_player, &params);
  if (!((music = bunny_load_music("sounds/main_theme.flac")) == NULL))
    bunny_sound_play(music);
  bunny_loop(w, 255, &params);

  bunny_sound_stop(music);
  bunny_delete_sound(music);
  params.move = 0;
  close(params.sockfd);
  params.move = 0;
  bunny_stop(w);
}
