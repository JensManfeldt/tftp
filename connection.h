#ifndef CONNECTION_HEADER
#define CONNECTION_HEADER

#include "packet_types.h"
#include <netdb.h>
#include <stdio.h>

static const char* empty_service = "xxxxxxxx"; // Not the nicest but will do for now

// Connection struct
// Holds the information about the current file transfer
struct connection {
    char service[NI_MAXSERV]; // source of connection
    short block_number; // The current block number on the connection
    enum TFTP_PACKET_TYPE connection_type; // The type of operation the connection is RRQ or WRQ
    FILE* connection_fd; // The file descriptor for the current transfer
    int current_message_size; // size of the messeage currently storage in the buffer
    char message_buf[516]; // Max message size
};


// conn : The connection struct to create the connection inside
// service_buf : The service that made the connection
// filename : filename of the file the connection is reading
// return 0 when the valid data message could be countrusted
// otherwise -1 and fills the correct error message to reply with this means that the connection
// should not be presisted
int init_rrq_connection(struct connection* conn, char* service_buf, char* filename);

// conn : The connection struct to create the connection inside
// service_buf : The service that made the connection
// filename : filename of the file the connection is reading
// return 0 when the valid data message could be countrusted
// otherwise -1 and fills the correct error message to reply with this means that the connection
// should not be presisted
int init_wrq_connection(struct connection* conn, char* service_buf, char* filename);


// Increments the rrq connection place the next block of data into it
int increment_rrq_connection(struct connection* conn);

// Increments the wrq connection write the new data to disk and place
// ack message into the connection
int increment_wrq_connection(struct connection* conn, char* in_message, size_t in_message_size);

// Cleans the connection struct
void close_connection(struct connection *conn);

#endif
