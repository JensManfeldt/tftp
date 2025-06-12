#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "connection.h"
#include "packet_types.h"

int init_rrq_connection(struct connection *conn, char* service_buf, char* filename){

    FILE* read_fptr = fopen(filename, "rb");

    if (!read_fptr) {
        // In here the creathe the error message in the message_buf
        // and return -1 or something like that
        int err = errno;
        printf("Could not open file (%s) : %s\n", filename, strerror(err));

        // TODO : Fill in the correct error message
        return -1;
    }

    strncpy(conn->service, service_buf, NI_MAXSERV);
    conn->block_number = 1;
    conn->connection_type = RRQ;
    conn->connection_fd = read_fptr;
    memset(conn->message_buf, 0, sizeof(conn->message_buf));

    conn->message_buf[0] = 0;
    conn->message_buf[1] = DATA;
    conn->message_buf[2] = 0;
    conn->message_buf[3] = 1; // first block number is 1

    int bytes_read = fread(&conn->message_buf[DATA_HEADER_SIZE], 1, sizeof(conn->message_buf) - DATA_HEADER_SIZE, read_fptr);
    conn->current_message_size = bytes_read + DATA_HEADER_SIZE; // The amount of bytes read from the file + 4 for the "header"

    return 0;
}

int init_wrq_connection(struct connection* conn, char* service_buf, char* filename) {

    FILE* write_fptr = fopen(filename, "wb");
    if(!write_fptr) {
        // In here the
        // create the error message in the message_buf
        // and return -1 or something like that
        int err = errno;
        printf("Could not open file (%s) : %s\n", filename, strerror(err));

        // TODO : Fill in the correct error message
        return -1;
    }

    strncpy(conn->service, service_buf, NI_MAXSERV);
    conn->block_number = 0;
    conn->connection_type = WRQ;
    conn->connection_fd = write_fptr;
    memset(conn->message_buf, 0, sizeof(conn->message_buf));

    conn->message_buf[0] = 0;
    conn->message_buf[1] = ACK;
    conn->message_buf[2] = 0;
    conn->message_buf[3] = 0;
    conn->current_message_size = ACK_MESSAGE_SIZE;

    return 0;
}

int increment_rrq_connection(struct connection* conn) {


    conn->block_number++;
    conn->message_buf[0] = 0;
    conn->message_buf[1] = DATA;
    // TODO : Fix the setting of the next block so it also works when the
    // block number takes more than 1 byte
    conn->message_buf[2] = 0;
    conn->message_buf[3] = (char)conn->block_number; // first block number is 1

    int bytes_read = fread(&conn->message_buf[DATA_HEADER_SIZE], 1, sizeof(conn->message_buf) - DATA_HEADER_SIZE, conn->connection_fd);
    conn->current_message_size = bytes_read + DATA_HEADER_SIZE; // The amount of bytes read from the file + 4 for the "header"

   return 0;
}


int increment_wrq_connection(struct connection* conn, char* in_message_buf, size_t in_message_buf_size) {

    // TODO : This can error handle it
    (void)fwrite(&in_message_buf[DATA_HEADER_SIZE], 1, in_message_buf_size - DATA_HEADER_SIZE , conn->connection_fd);

    fflush(conn->connection_fd); // This only needs to be here till termination of connection is implemented

    conn->message_buf[0] = 0;
    conn->message_buf[1] = ACK;

    // TODO : This likey works just fells werid to use the message to ack the
    // message itself. Fells like there could be some packet ording problems...
    memcpy(&conn->message_buf[2], &in_message_buf[2], sizeof(char));
    memcpy(&conn->message_buf[3], &in_message_buf[3], sizeof(char));
    conn->block_number++;

    conn->current_message_size = ACK_MESSAGE_SIZE;

    return 0;
}

void close_connection(struct connection *conn) {
    (void)fclose(conn->connection_fd);
    memset(conn->service, 0, sizeof(conn->service));
    memcpy(conn->service, EMPTY_SERVICE, strlen(EMPTY_SERVICE)); // Make sure the empty service name is set
    conn->block_number = 0;
    conn->connection_type = 0;
    return;
}
