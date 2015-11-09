#define MOVE 1
#define CONNECT 2
#define DISCONNECT 3


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
  t_bunny_picture	*skin;
};

typedef struct s_list	chain_list;

struct s_server
{
  int			player;
  int			sockfd;
  t_bunny_position	pos;
  char			name[10];
};

struct s_obstacle
{
  struct s_obstacle	*next;
  t_bunny_picture	*pic;
  t_bunny_position	*pos;
  int			size[2];
};

typedef struct s_obstacle	t_obstacle;

struct s_params
{
  char			*server;
  int			*move_player;
  int			move;
  int			move_multiplier;
  int			server_port;
  int			sockfd;
  int			user;
  chain_list		*players;
  t_bunny_pixelarray	*black;
  t_bunny_pixelarray	*pa;
  t_bunny_position	origin;
  t_bunny_position	pos;
  t_bunny_window	*win;
  t_bunny_picture	*pichu;
  t_bunny_picture	*skin;
  t_bunny_picture	*tree;
  t_bunny_picture	*wheed;
  t_obstacle		*obstacles;
};

typedef struct s_params		t_params;
typedef struct s_packet		t_packet;
typedef struct s_server		t_server;

void	set_black(t_bunny_pixelarray *);
