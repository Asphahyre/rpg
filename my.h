#define MOVE 1
#define CONNECT 2
#define DISCONNECT 3

/*
** Defininf players data
*/

#define PLAYER 1
#  define MALE 0
#  define FEMALE 0
#  define FACE 0
#  define LEFT 1
#  define RIGH 2
#  define BACK 3


struct s_packet
{
  char			type;
  int			player;
  t_bunny_position	pos;
  char			name[10];
};

struct s_list
{
  void			*everyone;
  void			*elem;
  struct s_list		*next;
  int			loop;
  int			id;
};

typedef struct s_list	chain_list;

struct s_players
{
  void			*everyone;
  void			*elem;
  struct s_players	*next;
  int			loop;
  int			id;
  t_bunny_picture	*skin;
  int			orientation;
};

typedef struct s_players	t_players;

struct s_server
{
  int			player;
  int			sockfd;
  t_bunny_position	pos;
  char			name[10];
};

struct s_bounds
{
  int	type;
  int	x1;
  int	y1;
  int	x2;
  int	y2;

  int	gate_x1;
  int	gate_x2;
  int	gate_y1;
  int	gate_y2;
};

typedef struct s_bounds	t_bounds;

struct s_obstacle
{
  struct s_obstacle	*next;
  t_bunny_picture	*pic;
  t_bunny_position	*pos;
  t_bounds		bounds;
};

typedef struct s_obstacle	t_obstacle;

struct s_params
{
  char			*server;
  int			*move_player;
  int			move;
  t_bunny_position	camera;
  int			move_multiplier;
  int			run;
  int			server_port;
  int			sockfd;
  int			user;
  t_players		*players;
  t_bunny_pixelarray	*black;
  t_bunny_pixelarray	*pa;
  t_bunny_position	origin;
  t_bunny_position	pos;
  t_bunny_window	*win;
  t_bunny_picture	*pichu;
  t_bunny_picture	*skin[12];
  t_bunny_picture	*tree;
  t_bunny_picture	*wheed;
  t_obstacle		*obstacles;
  int			orientation;
  int			step;
};

typedef struct s_params		t_params;
typedef struct s_packet		t_packet;
typedef struct s_server		t_server;

void	set_black(t_bunny_pixelarray *);
