#ifndef SERVER_HEADER
#define SERVER_HEADER

#include "server.h"
#include "packet_types.h"
#include "packet_parsing.h"
#include "connection.h"
#include <arpa/inet.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>



// init the server
// if 0 is returned the passed in server pointer will
// point to a valid server
// Since tftp is udp we only need the address and the port
int init_server(struct server *server, char* address, int port, char* root_dir, size_t max_connections) {
    printf("Hello from init server!!\n");

    int server_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in server_addr;

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(address);
    server_addr.sin_port = htons(port);

    int bind_res = bind(server_socket_fd, (const struct sockaddr*) &server_addr, sizeof(server_addr));
    if (bind_res != 0) {
        perror("Server failed to bind ");
        return -1;
    }

    server->socket_fd = server_socket_fd;

    char curr_dir_buf[100];
    printf("Current directory is %s\n", getcwd(curr_dir_buf, sizeof(curr_dir_buf)));
    printf("Changing to %s\n", root_dir);
    chdir(root_dir);
    printf("Current directory is now %s\n", getcwd(curr_dir_buf, sizeof(curr_dir_buf)));
    server->root_dir = root_dir;

    printf("Connections has size %lu\n", sizeof(struct connection));

    // TODO : Error handling
    struct connection* p_connections = malloc(max_connections * sizeof(struct connection));

    server->open_conns = p_connections;

    for(int i = 0; i < max_connections; i++) {
        memcpy(server->open_conns[i].service, EMPTY_SERVICE, strlen(EMPTY_SERVICE));
    }
    server->max_connections = max_connections;

    memset(server->error_msg_buf, 0, sizeof(server->error_msg_buf));

    // Set timeout for how long server will wait for recvfrom
    // before checking other items
    struct timeval wait_for_data;
    wait_for_data.tv_sec = 1;
    wait_for_data.tv_usec = 0;
    setsockopt(server->socket_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&wait_for_data, sizeof(wait_for_data));

    return 0;
}

void run_server(struct server *server) {

    char in_message_buf[TFTP_MAX_MESSAGE_SIZE]; // define this as a constant tftp max message size is a data packet 2 (opcode) + 2 (block nr.) + max 512 (data)
    char host_buf[NI_MAXHOST];
    char service_buf[NI_MAXSERV];
    char filename[256]; // max filename length on ext4 is 255 so leave space for 0 terminator
    char mode[256]; // buffer for storing mode from package in


    while(1) {

        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(struct sockaddr);

        retransmit_on_required_connections(server);

        int recv_res = recvfrom(server->socket_fd, in_message_buf, sizeof(in_message_buf), 0, (struct sockaddr*)&client_addr, &client_addr_len);
        if (recv_res == -1) {
            perror("Error in recvfrom");
            continue;
        }

        int get_name_res = getnameinfo((struct sockaddr*)&client_addr, client_addr_len, host_buf, sizeof(host_buf), service_buf, sizeof(service_buf), NI_NUMERICHOST | NI_NUMERICSERV);
        if (get_name_res != 0) {
            // This if is here to make sure we always the host_buf and service_buf filled
            // might not be the best id's for the connections but used for now
            printf("Could not getnameinfo : %s\n", gai_strerror(get_name_res));
            continue;
        }

        uint16_t net_order_opcode;
        memcpy(&net_order_opcode, in_message_buf, sizeof(net_order_opcode));
        switch (ntohs(net_order_opcode)) {
            case RRQ:
                printf("Got read (RRQ) request from host=%s service=%s\n", host_buf, service_buf);

                parse_rrq_packet(&in_message_buf[TFTP_OPCODE_SIZE], sizeof(in_message_buf) - TFTP_OPCODE_SIZE, filename, sizeof(filename), mode, sizeof(mode));

                struct connection* new_read_connection = find_free_connection_slot(server);

                if (!new_read_connection) {
                    // Server can not handle anymore connection right now
                    // figure out something nice to do about it
                    // break for now
                    printf("Server could not handle anymore connections!!\n");
                    break;
                }

                int init_rrq_res = init_rrq_connection(new_read_connection, service_buf, client_addr, client_addr_len, filename);

                // The max message size is 516 but only send the amount of bytes we read from the file + 4 (for the "header")
                int sendto_res = sendto(server->socket_fd, new_read_connection->message_buf, new_read_connection->current_message_size, MSG_CONFIRM, (struct sockaddr*)&new_read_connection->client_addr, new_read_connection->client_addr_len);

                // The message just got send so the timestamp for next re-transmission should be moved forward
                int set_retransmit_rrq = set_next_retransmision_time(new_read_connection);
                if (set_retransmit_rrq != 0) {
                    printf("Failed to set next retransmit time");
                }

                if (sendto_res == -1) {
                    perror("Failed to send ");
                }

                // Wait till here with check the connection if it got init correctly since we always need to send the packet
                // bcause the init function will put the correct packet to response with into the message buffer always
                if (init_rrq_res != 0) {
                    printf("(RRQ) Error during setting up the connection error should have been send cleaning up connection now");
                    close_connection(new_read_connection);
                }

                break;
            case WRQ:
                printf("Got write (WRQ) request from host=%s service=%s\n", host_buf, service_buf);

                parse_wrq_packet(&in_message_buf[TFTP_OPCODE_SIZE], recv_res - TFTP_OPCODE_SIZE, filename, sizeof(filename), mode, sizeof(mode));

                struct connection* new_write_connection = find_free_connection_slot(server);
                if (!new_write_connection) {
                    // Server can not handle anymore connection right now
                    // figure out something nice to do about it
                    // break for now
                    printf("Server could not handle anymore connections!!");
                    break;
                }

                int init_wrq_res = init_wrq_connection(new_write_connection, service_buf, client_addr, client_addr_len, filename);

                int send_to_res = sendto(server->socket_fd, new_write_connection->message_buf, new_write_connection->current_message_size, MSG_CONFIRM, (struct sockaddr*)&new_write_connection->client_addr, new_write_connection->client_addr_len);

                // TODO: The message just got send so the timestamp for next re-transmission should be moved forward
                int set_retransmit_wrq = set_next_retransmision_time(new_write_connection);
                if (set_retransmit_wrq != 0) {
                    printf("Failed to set next retransmit time");
                }

                if (send_to_res != 0) {
                    // TODO : if the send fail the connection it should be marked for re-transmission
                    // If there is an error message in the message buffer we should also decide to do something
                    // In here sendto will have set errno use the value to do something smart about what to do next
                    printf("(WRQ) Sending messaged failed...");
                }

                // Wait till here with check the connection if it got init correctly since we always need to send the packet
                //bcause the init function will put the correct packet to response with into the message buffer always
                if (init_wrq_res != 0) {
                    printf("(WRQ) Error during setting up the connection error should have been send cleaning up connection now");
                    close_connection(new_write_connection);
                }
                break;

            case DATA:
                printf("Got data (DATA) package\n from host=%s service=%s\n", host_buf, service_buf);

                struct connection* data_conn = find_connection(server, service_buf);
                if (!data_conn) {
                    printf("Data packet came from an unkown service ignoring it...\n");
                    // This still needs a test that it is correct
                    create_error_packet(server->error_msg_buf, sizeof(server->error_msg_buf), UNKNOWN_TRANSFER_ID);
                    break;
                }

                int inc_wrq_res = increment_wrq_connection(data_conn, in_message_buf, recv_res);
                if(inc_wrq_res != 0) {
                    printf("Error during (WRQ) increment...");
                }

                printf("Sending ack\n");
                (void)sendto(server->socket_fd, data_conn->message_buf, data_conn->current_message_size, MSG_CONFIRM, (struct sockaddr*)&client_addr, client_addr_len);

                // TODO : The message just got send so the timestamp for next re-transmission should be moved forward
                int set_retransmit_data = set_next_retransmision_time(data_conn);
                if (set_retransmit_data != 0) {
                    printf("Failed to set next retransmit time");
                }

                break;
            case ACK:
                printf("Got ack (ACK) package from host=%s service=%s\n", host_buf, service_buf);

                printf("Ack block %d%d\n", in_message_buf[2], in_message_buf[3]);

                struct connection* acked_connection = find_connection(server, service_buf);
                if (!acked_connection) {
                    printf("Could not find connection for ack packet reviced ignoreing it...");
                    break;
                }

                int inc_rrq_res = increment_rrq_connection(acked_connection);
                if (inc_rrq_res != 0) {
                    printf("Error during (RRQ) increment...");
                }

                (void)sendto(server->socket_fd, (const void*)acked_connection->message_buf, acked_connection->current_message_size, MSG_CONFIRM, (struct sockaddr*)&client_addr, client_addr_len);
                // TODO: The message just got send so the timestamp for next re-transmission should be moved forward
                int set_retransmit_ack = set_next_retransmision_time(acked_connection);
                if (set_retransmit_ack != 0) {
                    printf("Failed to set next retransmit time");
                }

                break;
            case ERROR:
                printf("Got ERROR (ERROR) package from host=%s service=%s\n", host_buf, service_buf);

                uint16_t error_code_network_order;
                memcpy(&error_code_network_order, &in_message_buf[2], sizeof(error_code_network_order));
                uint16_t error_code_host_order = ntohs(error_code_network_order);
                printf("Error code is %d\n", error_code_host_order);

                char print_err_buf[TFTP_MAX_MESSAGE_SIZE - ERROR_HEADER_SIZE];
                memset(print_err_buf, 0, sizeof(print_err_buf));
                memcpy(print_err_buf, &in_message_buf[4], TFTP_MAX_MESSAGE_SIZE - ERROR_HEADER_SIZE);
                printf("Provided message was : %s\n", print_err_buf);

                break;
       }
    }
}

struct connection* find_free_connection_slot(struct server* server) {
    printf("running find free conn\n");
    for(int i = 0; i < server->max_connections; i++) {
        printf("service %s\n", server->open_conns[i].service);
        if(strcmp(server->open_conns[i].service, EMPTY_SERVICE) == 0) {
            printf("Found empty slot for a new connection\n");
            return &server->open_conns[i];
        }
        else {
            printf("Server has active connection with service : %s\n", server->open_conns[i].service);
        }

    }
    printf("No slot open for new connection\n");
    return NULL;

}

struct connection* find_connection(struct server* server, char* service) {

    for(int i = 0; i < server->max_connections; i++) {
        if (strcmp(server->open_conns[i].service, service) == 0) {
            printf("Found matching connection\n");
            return &server->open_conns[i];
        }
    }

    printf("Could not find a current active connection matching that servic\n");
    return NULL;
}

void retransmit_on_required_connections(struct server* server) {
    struct timespec current_time;
    clock_gettime(CLOCK_MONOTONIC, &current_time);

    for (int i = 0; i < server->max_connections; i++) {
        struct connection conn = server->open_conns[i];

        if (strcmp(conn.service, EMPTY_SERVICE) == 0){
            // Not an open connection so continue
            continue;
        }

        // TODO : Doing it like this will make it so the retransmission can be up to a
        // second off from when it should have been but good enough for now
        if (conn.re_transmit_time.tv_sec > current_time.tv_sec) {
            continue;
        }

        (void)sendto(server->socket_fd, conn.message_buf, conn.current_message_size, MSG_CONFIRM, (struct sockaddr*)&conn.client_addr, conn.client_addr_len);
    }
}

int destroy_server(struct server *server) {
    close(server->socket_fd);
    free(server->open_conns);
    return 0;
}

#endif
