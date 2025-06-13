#include "packet_parsing.h"
#include "packet_types.h"
#include <string.h>
#include <stdio.h>

static const char* TFTP_ERROR_CODE_STR[] = {
    "Not defined",
    "File not found",
    "Access violation",
    "Disk full or allocation exceeded",
    "Unknown transfer id",
    "File already exists",
    "No such user",
};

// This function contains some copies that is likey not neccesary
// pointer could just be set to the point to the correct positions in the message_buf
void parse_rrq_or_wrq_packet(char* message_buf, size_t message_size, char* filename, size_t filename_size, char* mode, size_t mode_size) {
    size_t filename_index = 0; // index in the massage for the filename start here
    while (message_buf[filename_index] != '\0') {
       filename_index++;
    }
    // copy filename from message to the filename buffer including the 0 terminator
    // This could be done without copy, but lets just copy for now
    strncpy(filename, message_buf, filename_size);
    printf("filename : %s\n", filename);

    size_t mode_index = filename_index + 1; // Index where the mode start +1 to skip over the 0 byte from the filename ending
    while (message_buf[mode_index] != '\0') {
        mode_index++;
    }

    strncpy(mode, &message_buf[filename_index + 1], mode_size);
    printf("mode : %s\n", mode);

}

void parse_rrq_packet(char* message_buf, size_t message_size, char* filename, size_t filename_size, char* mode, size_t mode_size){
   parse_rrq_or_wrq_packet(message_buf, message_size, filename, filename_size, mode, mode_size);
   return;
}

void parse_wrq_packet(char* message_buf, size_t message_size, char* filename, size_t filename_size, char* mode, size_t mode_size){
    parse_rrq_or_wrq_packet(message_buf, message_size, filename, filename_size, mode, mode_size);
    return;
}

void create_error_packet(char* message_buf, size_t message_buf_size, enum TFTP_ERROR_CODE error_type) {

    // TODO : Check size of the message buf (maybe...)
    // TODO : use the the function that puts this into network order
    message_buf[0] = 0;
    message_buf[1] = ERROR;
    message_buf[2] = 0;
    message_buf[3] = error_type;
    //TODO: Check the the message fits into the buffer
    (void)strncpy(&message_buf[4], TFTP_ERROR_CODE_STR[error_type], strlen(TFTP_ERROR_CODE_STR[error_type]));
    return;
}
