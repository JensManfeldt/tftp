#include "connection.h"
#include <stddef.h>

struct server {
    int socket_fd; // Socket file descriptor the server is running on
    char* root_dir; // Root dir the server is serving from
    int max_connections; // Max number of connections the server can handle
    struct connection* open_conns; // list of open conns the server has
    char error_msg_buf[516]; // buffer to send error messages from
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
