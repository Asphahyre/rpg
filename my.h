struct s_params
{
  t_bunny_window	*win;
  t_bunny_position	pos;
  t_bunny_position	origin;
  t_bunny_pixelarray	*pa;
  t_bunny_pixelarray	*black;
  int			*move_player;
  int			move;
  int			move_multiplier;
};

typedef struct s_params t_params;
