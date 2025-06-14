#ifndef CONNECTION_HEADER
#define CONNECTION_HEADER

#include "packet_types.h"
#include <netdb.h>
#include <stdio.h>
#include <sys/socket.h>
#include <time.h>

#define RETRANSMIT_TIMEOUT_IN_SEC 2
static const char EMPTY_SERVICE[32] = "xxxxxxxx"; // Not the nicest but will do for now

// Connection struct
// Holds the information about the current file transfer
struct connection {
    char service[NI_MAXSERV]; // source of connection
    short block_number; // The current block number on the connection
    enum TFTP_PACKET_TYPE connection_type; // The type of operation the connection is RRQ or WRQ
    FILE* connection_fd; // The file descriptor for the current transfer
    int current_message_size; // size of the messeage currently storage in the buffer
    char message_buf[516]; // Max message size
    struct timespec re_transmit_time; // Time stamp when the message should be re-transmitted
    struct sockaddr_in client_addr; // Client address
    socklen_t client_addr_len; // Length of client address
};


// conn : The connection struct to create the connection inside
// service_buf : The service that made the connection
// filename : filename of the file the connection is reading
// return 0 when the valid data message could be countrusted
// otherwise -1 and fills the correct error message to reply with this means that the connection
// should not be presisted
int init_rrq_connection(struct connection* conn, char* service_buf, struct sockaddr_in client_addr, socklen_t client_addr_len, char* filename);

// conn : The connection struct to create the connection inside
// service_buf : The service that made the connection
// filename : filename of the file the connection is reading
// return 0 when the valid data message could be countrusted
// otherwise -1 and fills the correct error message to reply with this means that the connection
// should not be presisted
int init_wrq_connection(struct connection* conn, char* service_buf, struct sockaddr_in client_addr, socklen_t client_addr_len, char* filename);


// Increments the rrq connection place the next block of data into it
int increment_rrq_connection(struct connection* conn);

// Increments the wrq connection write the new data to disk and place
// ack message into the connection
int increment_wrq_connection(struct connection* conn, char* in_message, size_t in_message_size);

// Update the timestamp for when the next retransmit of the messge should happen
// return 0 on success and -1 on failure
int set_next_retransmision_time(struct connection* conn);

// Cleans the connection struct
void close_connection(struct connection *conn);

#endif
