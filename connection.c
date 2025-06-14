#include <errno.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "connection.h"
#include "packet_parsing.h"
#include "packet_types.h"

int init_rrq_connection(struct connection *conn, char* service_buf, struct sockaddr_in client_addr, socklen_t client_addr_len, char* filename){

    FILE* read_fptr = fopen(filename, "r");

    if (!read_fptr) {
        conn->connection_type = ERROR;
        int err = errno;
        switch (err) {
           case  ENOENT:
                printf("Requested file (%s) does not exists\n", filename);
                create_error_packet(conn->message_buf, sizeof(conn->message_buf), FILE_NOT_FOUND);
                return -1;
            case EACCES:
                printf("Requested file (%s) is not allowed to be accessed by server\n", filename);
                create_error_packet(conn->message_buf, sizeof(conn->message_buf), ACCESS_VIOLATION);
                return -1;
            default:
                printf("Got to default case when file (%s) got requested errno was : %s\n", filename, strerror(err));
                create_error_packet(conn->message_buf, sizeof(conn->message_buf), NOT_DEFINED);
                return -1;
        }
    }

    strncpy(conn->service, service_buf, NI_MAXSERV);
    conn->block_number = 1;
    conn->connection_type = RRQ;
    conn->connection_fd = read_fptr;
    memset(conn->message_buf, 0, sizeof(conn->message_buf));


    memcpy(&conn->client_addr, &client_addr, client_addr_len);
    conn->client_addr_len = client_addr_len;

    conn->message_buf[0] = 0;
    conn->message_buf[1] = DATA;
    conn->message_buf[2] = 0;
    conn->message_buf[3] = 1; // first block number is 1

    int bytes_read = fread(&conn->message_buf[DATA_HEADER_SIZE], 1, sizeof(conn->message_buf) - DATA_HEADER_SIZE, read_fptr);
    conn->current_message_size = bytes_read + DATA_HEADER_SIZE; // The amount of bytes read from the file + 4 for the "header"

    return 0;
}

int init_wrq_connection(struct connection* conn, char* service_buf, struct sockaddr_in client_addr, socklen_t client_addr_len, char* filename) {

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
    conn->block_number = 1;
    conn->connection_type = WRQ;
    conn->connection_fd = write_fptr;
    memset(conn->message_buf, 0, sizeof(conn->message_buf));


    memcpy(&conn->client_addr, &client_addr, client_addr_len);
    conn->client_addr_len = client_addr_len;

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

    uint16_t blk_num_in_network_order = htons(conn->block_number);
    memcpy(&conn->message_buf[2], &blk_num_in_network_order, sizeof(conn->block_number));

    int bytes_read = fread(&conn->message_buf[DATA_HEADER_SIZE], 1, sizeof(conn->message_buf) - DATA_HEADER_SIZE, conn->connection_fd);
    conn->current_message_size = bytes_read + DATA_HEADER_SIZE; // The amount of bytes read from the file + 4 for the "header"

   return 0;
}


int increment_wrq_connection(struct connection* conn, char* in_message_buf, size_t in_message_buf_size) {
    uint16_t in_msg_block_net;
    memcpy(&in_msg_block_net, &in_message_buf[2], sizeof(in_msg_block_net));

    if (conn->block_number != ntohs(in_msg_block_net)) {
        // TODO : We should do something smarter here
        // since we just need to wait for the the correct packet that we need to come
        printf("Was waiting for block num %d but got %d", conn->block_number, ntohs(in_msg_block_net) );
    }


    // TODO : This can error handle it
    (void)fwrite(&in_message_buf[DATA_HEADER_SIZE], 1, in_message_buf_size - DATA_HEADER_SIZE , conn->connection_fd);

    fflush(conn->connection_fd); // This only needs to be here till termination of connection is implemented

    conn->message_buf[0] = 0;
    conn->message_buf[1] = ACK;

    uint16_t blk_num_in_network_order= htons(conn->block_number);
    memcpy(&conn->message_buf[2], &blk_num_in_network_order, sizeof(conn->block_number));
    conn->block_number++;

    conn->current_message_size = ACK_MESSAGE_SIZE;

    return 0;
}

int set_next_retransmision_time(struct connection* conn) {
    int get_time_res = clock_gettime(CLOCK_MONOTONIC, &conn->re_transmit_time);
    if (get_time_res != 0) {
        int err = errno;
        printf("Failed to get MONOTONIC clock time : %s", strerror(err));

        conn->re_transmit_time.tv_nsec = 0;
        conn->re_transmit_time.tv_sec = 0;
        return -1;
    }

    conn->re_transmit_time.tv_sec += RETRANSMIT_TIMEOUT_IN_SEC;

    printf("Moved forward the timer for restranmission");
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
