#include "socket.h"
#include "client.h"
#include "packet_interface.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> /* * sockaddr_in6 */
#include <netdb.h>
#include <math.h>

uint8_t SEQNUM = 0;
uint16_t LAST_ACK;
char* END = "exit";

void send_data(char *hostname, int port, char* file){
    /*
        - Get the real_address of our hostname
    */
    struct sockaddr_in6 real_addr;
    memset(&real_addr, 0, sizeof(real_addr));
    const char * ret_addr = real_address(hostname, &real_addr);
    if(ret_addr){
        fprintf(stderr, "Address %s not recognized. Error code : %s\n", hostname, ret_addr);
        return;
    }

    /*
        - Create Socket
    */
    int socket = create_socket(NULL, 0, &real_addr, port); // Socket
    int fd;                                                // File Descriptor

    if(file){
        fd = open((const char*) file, O_RDONLY);
        if(!fd){
            fprintf(stderr, "Error opening the Input file %s \n", file);
            return;
        }
    }
    else{
        fd = STDIN_FILENO;
    }

    int endOfFile = 0;  // 1 if we have reached the EOF, 0 else
    int isEmpty = 1;   // 1 if the buffer is empty, 0 else


    /*
        Initialize the sender.

        TYPE => PTYPE_DATA
        TR => 0
        WINDOW => 1
        TIMESTAMP => default
        SEQNUM => 0
    */
    pkt_status_code return_status;
    pkt_t* pkt_sender = pkt_new();
    if(!pkt_sender){
        fprintf(stderr, "Error creating the sender packet. \n");
        return;
    }
    return_status = pkt_set_window(pkt_sender, 1);
    if(return_status != PKT_OK){
        fprintf(stderr, "Error, invalid window size\n");
        return;
    }

    fd_set read_set;
    int len_set;

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100000;

    int stop = 0; //DEBUG
    /*
        Create loop for reading Input
    */
    while(!endOfFile){
        FD_ZERO(&read_set);
        FD_SET(socket, &read_set);

        int notFull = 1;

        char* buffer_read = (char*) malloc(sizeof(char) * 512); // Payload
        char *buffer_temp = (char*) malloc(sizeof(char) * 528); // Payload de 512 max + header + crc = 528
        if(!endOfFile && (isEmpty || notFull)){
            FD_SET(fd, &read_set);
        }
        len_set = (socket > fd) ? socket + 1 : fd + 1;

        struct timeval new_tv = tv;

        select(len_set, &read_set, NULL, NULL, &new_tv);

        /*
            Lecture sur le fichier file ou sur le STDIN
        */
        if(FD_ISSET(fd, &read_set)){

            int length = read(fd, (void*) buffer_read, MAX_PAYLOAD_SIZE);
            if(!file && length > 0)
                buffer_read[strcspn(buffer_read, "\n")] = 0;
            if(length == 0)
                endOfFile = 1;
            else if(length < 0){
                fprintf(stderr, "Error reading data in client.c\n");
                /*
                    Free allocated memory
                */
                pkt_del(pkt_sender);
                free(buffer_temp);
                free(buffer_read);
                return;
            }
            else{ // length > 0, data to read
                if(strcmp(buffer_read, END) == 0){
                    stop = 1;
                }

                if(isEmpty == 1)
                    isEmpty = 0;

                return_status = pkt_set_payload(pkt_sender, (const char*) buffer_read, length);
                if(return_status != PKT_OK){
                    fprintf(stderr, "Error setting payload\n");
                    pkt_del(pkt_sender);
                    free(buffer_temp);
                    free(buffer_read);
                    return;
                }
                pkt_set_seqnum(pkt_sender, SEQNUM);
                SEQNUM = (SEQNUM + 1) % 256; // Increment the Seqnum, cannot be more than 255

                size_t length_temp = 528;

                return_status = pkt_encode(pkt_sender, buffer_temp, &length_temp);
                if(return_status != PKT_OK){
                    fprintf(stderr, "Error encoding pkt with seqnum n°%d\n", SEQNUM-1);
                    pkt_del(pkt_sender);
                    free(buffer_temp);
                    free(buffer_read);
                    return;
                }

                length = write(socket, buffer_temp, length_temp);
                if(length < 0){
                    fprintf(stderr, "Error writing pkt with seqnum %d onto socket\n", SEQNUM-1);
                    pkt_del(pkt_sender);
                    free(buffer_temp);
                    free(buffer_read);
                    return;
                }
                if(stop == 1){
                    free(buffer_read);
                    free(buffer_temp);
                    break;
                }

            } // end length > 0

        } // End of reading control block

        /*
            Lecture sur le socket => ACK ou NACK
        */
        if(FD_ISSET(socket, &read_set)){

        } // End of Ack/Nack control block
        free(buffer_read);
        free(buffer_temp);
    } // End while loop
    pkt_del(pkt_sender);


} // End send_data
