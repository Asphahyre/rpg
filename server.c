/* A simple server in the internet domain using TCP
  The port number is passed as an argument */
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
#include <unistd.h>

void	*error(const char *msg)
{
  perror(msg);
  return (NULL);
}

void		delete(chain_list *list, chain_list *elem)
{

  while (list->next != elem)
    list = list->next;
  list->next = elem->next;
  free(elem->elem);
  free(elem);
}

void		push(chain_list *list, chain_list *elem)
{
  while (list->next)
    list = list->next;
  list->next = elem;
}

void		send_to_all(int type, int who, void *what, chain_list *clients)
{
  t_server	*srv;
  chain_list	*elm;
  t_packet	pck;

  switch(type)
  {
    case MOVE:
      pck.type = MOVE;
      pck.player = who;
      pck.pos = *((t_bunny_position *)what);
      clients = clients->next;
      while (clients = clients->next)
      {
	srv = clients->elem;
	write(srv->sockfd, &pck, sizeof(t_packet));
      }
      break;
    case CONNECT:
      pck.type = CONNECT;
      pck.player = who;
      pck.pos = *((t_bunny_position *)what);
      while (clients = clients->next)
      {
	srv = clients->elem;
	write(srv->sockfd, &pck, sizeof(t_packet));
      }
      break;
    case DISCONNECT:
      pck.type = DISCONNECT;
      pck.player = who;
      while (clients = clients->next)
      {
	srv = clients->elem;
	if (srv->player != who)
	  write(srv->sockfd, &pck, sizeof(t_packet));
      }
      break;
  }
}

void	*listen_sock(void *strct)
{
  t_packet		pck;
  chain_list		*elm;
  t_server		*srv;

  elm = strct;
  srv = elm->elem;
  while (elm->loop > 0)
  {
    elm->loop = read(srv->sockfd, &pck, sizeof(pck));
    if (elm->loop < 0) return error("ERROR while reading from socket");
    printf("Received positions [%d ; %d] from %d\n", pck.pos.x, pck.pos.y, srv->player, srv->sockfd);
    send_to_all(MOVE, srv->player, &(pck.pos), elm->everyone);
  }
  send_to_all(DISCONNECT, srv->player, &(pck.pos), elm->everyone);
  delete(elm->everyone, elm);
  close(srv->sockfd);
  return (NULL);
}

int create_servers()
{
  int sockfd, newsockfd, portno;
  socklen_t clilen;
  int buffer;
  struct sockaddr_in serv_addr, cli_addr;
  int n;
  t_server		*srv;
  t_params		*params;
  chain_list		*list;
  chain_list		*nlist;
  pthread_t		server;
  t_bunny_position	pos;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
   error("ERROR opening socket");
  bzero((char *) &serv_addr, sizeof(serv_addr));
  portno = 8080;
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);
  if (bind(sockfd, (struct sockaddr *) &serv_addr,
      sizeof(serv_addr)) < 0)
    error("ERROR on binding");
  list = malloc(sizeof(chain_list));

  while (1)
    {
      listen(sockfd,5);
      newsockfd = accept(sockfd,
		        (struct sockaddr *) &cli_addr,
		        &clilen);
      if (newsockfd < 0)
        error("ERROR on accept");
      clilen = sizeof(cli_addr);
      buffer = 0;
      n = read(newsockfd, &buffer, sizeof(int));
      printf("NEW CONNECTION FROM %d\n", buffer);
      srv = malloc(sizeof(t_server));
      srv->player = buffer;
      srv->sockfd = newsockfd;
      nlist = malloc(sizeof(chain_list));
      nlist->elem = srv;
      nlist->loop = 1;
      nlist->everyone = list;
      nlist->next = NULL;
      push(list, nlist);
      pos.x = 200;
      pos.y = 100;
      send_to_all(CONNECT, srv->player, &pos, list);
      pthread_create(&server, NULL, listen_sock, nlist);
    }
  return 0;
}

void		main()
{
  create_servers();
}
