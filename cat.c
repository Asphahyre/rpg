/*
** cat.c for j 12 in /home/boulag_l/rendu/Piscine_C_J12/cat
** 
** Made by Luka Boulagnon
** Login   <boulag_l@epitech.net>
** 
** Started on  Wed Oct 14 09:18:04 2015 Luka Boulagnon
** Last update Fri Nov 06 15:03:51 2015 Asphähyre
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

int	my_putsterr(char *str)
{
  int	i;
  char	lie;

  i = 0;
  lie = 1;
  while (lie != '\0')
    {
      lie = *(str + i);
      write (2, &lie, 1);
      i = i + 1;
    }
}

void	nyan(char *n, int color)
{
  if (my_strcmp(n, "--nyan") == 0)
    {
      my_putchar(0);
      my_putchar('\033');
      my_putchar('[');
      my_putchar('0');
      my_putchar(';');
      my_putchar('3');
      my_putchar((color % 10) + 48);
      my_putstr("m");
      usleep((rand() % 100 * 1000));
    }
}

void	prompt(int mode)
{
  char	buff[32000];
  char	*me;
  int	i;

  if (mode == 1)
    {
      if (read(0, buff, 1) == 1 && my_putchar(*buff) == 0)
	while (read(0, buff, 1))
	  my_putchar(*buff);
      else
	{
	  i = -1;
	  while ((i = i + 1) < 32000)
	    buff[i] = '\0';
	  read(0, buff, 32000);
	  my_putstr(buff);
	}
      my_putchar('\n');
    }
  else
    {
      i = 0;
      while (read(0, buff, 1))
	  nyan("--nyan", (i = i + my_putchar(*buff) + 1));
    }
}

int	main(int argc, char **argv)
{
  int	fd;
  int	i;
  char	buff;
  int	color;

  if (argc == (i = (color = 0)) + 1
      || (argc == 2 && !my_strcmp(argv[1], "--nyan")))
    prompt(argc);
  while ((i = i + 1) < argc)
    {
      if ((fd = open(argv[i], O_RDONLY)) == -1)
	{
	  my_putsterr(argv[0]);
	  my_putsterr(": ");
	  my_putsterr(argv[i]);
	  my_putsterr(": No such file or directory\n");
	}
      else
	{
	  while (read(fd, &buff, 1) == 1)
	    {
	      nyan(argv[1], (color = color + 1));
	      my_putchar(buff);
	    }
	}
    }
}
