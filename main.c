/*
** main.c for Liblapin in /home/boulag_l/rendu/LibLapin
** 
** Made by Luka Boulagnon
** Login   <boulag_l@epitech.net>
** 
** Started on  Mon Nov 02 12:58:49 2015 Luka Boulagnon
** Last update Fri Nov 06 21:14:36 2015 Asphähyre
*/

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include "lapin.h"
#include "lapin_enum.h"
#include "my.h"

void error(char *msg)
{
    write(2, msg, strlen(msg));
    exit(0);
}

void	*send_pos(t_params params)
{

  return (NULL);
}

void		*move_player(void *arg)
{
  t_params	*params;
  int		i;
  pthread_t	send;

  params = arg;
  i = 0;
  while (params->move)
  {
    i = i + 1;
    params->pos.x = params->pos.x + params->move_player[0] * params->move_multiplier;
    params->pos.y = params->pos.y + params->move_player[1] * params->move_multiplier;
    if (!(i % 6))
      pthread_create(&send, NULL, send_pos, &params);
    usleep(1000000/60);
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
  bunny_set_key_response(&truc_pressed);
  bunny_display(win);
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

  pthread_create(&moving, NULL, move_player, &params);
  bunny_loop(w, 60, &params);

  params.move = 0;
  bunny_stop(w);
}
