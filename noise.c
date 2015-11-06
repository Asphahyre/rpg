/*
** noise.c for noise_2020 in /home/boulag_l/rendu/noise_2020
** 
** Made by Luka Boulagnon
** Login   <boulag_l@epitech.net>
** 
** Started on  Mon Nov 02 15:53:00 2015 Luka Boulagnon
** Last update Fri Nov 06 19:48:56 2015 Asphähyre
*/

#include "lapin.h"
#include <stdlib.h>

void		noise_color_full(t_bunny_pixelarray *pix)
{
  t_color	*pixels;
  unsigned int	i;

  pixels = pix->pixels;
  i = pix->clipable.clip_width * pix->clipable.clip_height;
  while ((i = i - 1) > 0)
  {
    pixels[i].argb[0] = rand() % 256;
    pixels[i].argb[1] = rand() % 256;
    pixels[i].argb[2] = rand() % 256;
    pixels[i].argb[3] = rand() % 256;
  }
}

void		set_black(t_bunny_pixelarray *pix)
{
  t_color	*pixels;
  unsigned int	i;

  pixels = pix->pixels;
  i = (pix->clipable.clip_width - pix->clipable.clip_x_position) * (pix->clipable.clip_height - pix->clipable.clip_y_position);
  while ((i = i - 1) > 0)
  {
    pixels[i].argb[0] = 0;
    pixels[i].argb[1] = 0;
    pixels[i].argb[2] = 0;
    pixels[i].argb[3] = 0;
  }
}

void		noise_color(t_bunny_pixelarray *pix)
{
  t_color	*pixels;
  unsigned int	i;

  pixels = pix->pixels;
  i = (pix->clipable.clip_width - pix->clipable.clip_x_position) * (pix->clipable.clip_height - pix->clipable.clip_y_position);
  while ((i = i - 1) > 0)
  {
    pixels[i].argb[0] = rand() % 256;
    pixels[i].argb[1] = rand() % 256;
    pixels[i].argb[2] = rand() % 256;
    pixels[i].argb[3] = rand() % 256;
  }
}

void		noise_grey(t_bunny_pixelarray *pix)
{
  t_color	*pixels;
  unsigned int	i;
  char		color;

  pixels = pix->pixels;
  i = (pix->clipable.clip_width - pix->clipable.clip_x_position) * (pix->clipable.clip_height - pix->clipable.clip_y_position);
  while ((i = i - 1) > 0)
  {
    color = rand() % 256;
    pixels[i].argb[0] = color;
    pixels[i].argb[1] = color;
    pixels[i].argb[2] = color;
    pixels[i].argb[3] = rand() % 256;
  }
}
