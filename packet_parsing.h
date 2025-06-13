#ifndef PACKET_PARSING_HEADER
#define PACKET_PARSING_HEADER

#include "packet_types.h"
#include <stdlib.h>

void parse_rrq_packet(char* message_buf, size_t message_size, char* filename, size_t filename_size, char* mode, size_t mode_size);

void parse_wrq_packet(char* message_buf, size_t message_size, char* filename, size_t filename_size, char* mode, size_t mode_size);

void create_error_packet(char* message_buf, size_t message_buf_size, enum TFTP_ERROR_CODE error_type);

#endif
