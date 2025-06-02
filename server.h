#include "connection.h"
#include <stddef.h>
#include <linux/io_uring.h>

struct server {
   int socket_fd;
   char* root_dir;
   int max_connections;
   struct connection* open_conns;
};


int init_server(struct server *server, char* address, int port, char* root_dir, size_t max_connections);

void run_server(struct server *server);

int destroy_server(struct server *server);

// Finds a free connection slot on the server
// returns NULL if non is avaliable
struct connection* find_free_connection_slot(struct server* server);

// find the connection that matches the service
// return NULL if it does not exist
struct connection* find_connection(struct server* server, char* service);
