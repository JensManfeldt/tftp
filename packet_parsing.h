#ifndef PACKET_PARSING_HEADER
#define PACKET_PARSING_HEADER

#include <stdlib.h>

void parse_rrq_packet(char* message_buf, size_t message_size, char* filename, size_t filename_size, char* mode, size_t mode_size);

void parse_wrq_packet(char* message_buf, size_t message_size, char* filename, size_t filename_size, char* mode, size_t mode_size);

#endif
