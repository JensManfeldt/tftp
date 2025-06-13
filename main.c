#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "server.h"


int main(int argc, char* argv[]) {
   printf("Hello from tftp server..\n");

   char* root_dir;
   char* address;
   int opt;
   int port;
   int num_connections = 1;
   // Parse 3 options
   // p : port
   // a : address
   // d : direcotry for the server to serve. This will the be root for the server.
   // c : number of "connection" the serer can have a one time
   char* opt_string = "p:a:d:c:";

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
            case 'd':
                root_dir = optarg;
                printf("Parsed root dir: %s\n", root_dir);
                break;
            case 'c':
                num_connections = atoi(optarg);
                printf("Parsed num connections : %d\n", num_connections);
                break;
       }
   }

   struct server server;

   int init_res = init_server(&server, address, port, root_dir, num_connections);

   if (init_res != 0) {
       printf("Failed to init server");
       return destroy_server(&server);
   }

   run_server(&server);

   return destroy_server(&server);

}
