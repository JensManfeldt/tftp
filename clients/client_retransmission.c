#include <arpa/inet.h>
#include <bits/getopt_core.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

// Client code that will not response and thus force a retransmission to from the server

int main(int argc, char* argv[]) {

    char test_filename[256];
    char* address;
    int opt;
    int port;
    int num_connections = 1;
    unsigned char opcode = 0;
    // Parse 3 options
    // p : port
    // a : address
    // d : direcotry for the server to serve. This will the be root for the server.
    // c : number of "connection" the serer can have a one time
    // r : will make the client send a rrq
    // w : will make the client send a wrq
    char* opt_string = "p:a:f:c:r:w:";

    while((opt = getopt(argc, argv, opt_string)) != -1) {
        switch (opt) {
             case 'p':
                 port = atoi(optarg);
                 printf("Parsed port : %d\n", port);
                 break;
             case 'a':
                 address = optarg;
                 printf("Parsed address : %s\n", address);
                 break;
             case 'f':
                 //test_filename = optarg;
                 strncpy(test_filename, optarg, sizeof(test_filename));
                 printf("Parsed filename: %s\n", test_filename);
                 break;
             case 'c':
                 num_connections = atoi(optarg);
                 printf("Parsed num connections : %d\n", num_connections);
                 break;
            case 'r':
                opcode = 1;
                printf("Parsed opcode to be %d", opcode);
                break;
            case 'w':
                opcode = 2;
                printf("Parsed opcode to be %d", opcode);
                break;
        }
    }

    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in server_addr;
    socklen_t server_addr_len = sizeof(struct sockaddr);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(address);
    server_addr.sin_port = htons(port);

    int conn_res = connect(sock_fd,(struct sockaddr*)&server_addr, server_addr_len);
    if(conn_res != 0) {
        perror("Failed to connect to server :");
        exit(-1);
    }

    char test_file_mode[] = "octat";
    char msg[516];

    msg[0] = 0;
    msg[1] = opcode; // This is a RRQ message
    memcpy(&msg[2], test_filename , sizeof(test_filename));
    memcpy(&msg[2 + sizeof(test_filename)], test_file_mode, sizeof(test_file_mode));

    int send_to_res = sendto(sock_fd, msg, sizeof(msg), MSG_WAITALL, (struct sockaddr*)&server_addr, server_addr_len);
    if(send_to_res == -1) {
        perror("Failed to sendto :");
        exit(-1);
    }

    struct sockaddr_in recv_addr;
    socklen_t recv_addr_len = sizeof(struct sockaddr);
    char in_msg[516];
    memset(in_msg, 0, sizeof(in_msg));
    int recv_res = recvfrom(sock_fd, &in_msg, sizeof(in_msg), MSG_WAITALL, (struct sockaddr*)&recv_addr, &recv_addr_len);
    if(recv_res == -1) {
        perror("Error during recvfrom :");
        exit(-1);
    }

    printf("Got message from server opcode : %d, block_nr : %d\n", in_msg[1], in_msg[3]);

    printf("Will wait for block to be retransmitted...\n");

    struct sockaddr_in recv_addr_2;
    socklen_t recv_addr_len_2 = sizeof(struct sockaddr);
    char in_msg_2[516];
    memset(in_msg_2, 0, sizeof(in_msg_2));
    int recv_res_2 = recvfrom(sock_fd, &in_msg_2, sizeof(in_msg_2), MSG_WAITALL, (struct sockaddr*)&recv_addr_2, &recv_addr_len_2);
    if(recv_res_2 == -1) {
        int err = errno;
        printf("Error during recvfrom 2 : %s\n", strerror(err));
        exit(-1);
    }

    if (recv_res != recv_res_2) {
        printf("The 2 packet recived did not have the same length pakcet 1 : %d \t packet 2 : %d\n", recv_res, recv_res_2);
        exit(1);
    }

    printf("First packet and retransmitted packet had same length\n");

    for (int i = 0; i < recv_res; i++) {
        if (in_msg[i] != in_msg_2[i]){
            printf("The re-transmitted packet did not match the packet first transmitted byte nr %d did not match\n", i);
            exit(1);
        }
    }

    printf("Got the same packet retransmitted !!!\n");
    exit(0);
}
