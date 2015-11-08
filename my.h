#define MOVE 1
#define CONNECT 2
#define DISCONNECT 3


struct s_packet
{
  char			type;
  int			player;
  t_bunny_position	pos;
};

struct s_list
{
  void			*everyone;
  void			*elem;
  struct s_list		*next;
  int			loop;
};

typedef struct s_list	chain_list;

struct s_server
{
  int			player;
  int			sockfd;
};

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
};

typedef struct s_params	t_params;
typedef struct s_packet	t_packet;
typedef struct s_server	t_server;

void	set_black(t_bunny_pixelarray *);
