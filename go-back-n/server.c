#include "socket.h"
#include "packet_interface.h"
#include "server.h"
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
#include <unistd.h>

char * buffer_read;
char* END = "exit";
uint8_t SEQNUM = 0;

void receive_data(char* hostname, int port, char* file){
    /*
    - Get the real_address of our hostname
    */
    struct sockaddr_in6 real_addr;
    memset(&real_addr, 0, sizeof(real_addr));
    const char* ret_addr = real_address(hostname, &real_addr);
    if(ret_addr){
        fprintf(stderr, "Address %s not recognized. Error code : %s\n", hostname, ret_addr);
        return;
    }

    /*
    - Create Socket
    */
    int socket = create_socket(&real_addr, port, NULL, 0); // Socket
    if(socket < 0){
        fprintf(stderr, "Error creating receiving socket\n");
        return;
    }

    int wait = wait_for_client(socket);
    if(wait < 0){
        fprintf(stderr, "Error waiting for client\n");
        return;
    }

    int fd; // File Descriptor
    if(file)
        fd = open((const char*) file, O_WRONLY | O_CREAT | O_TRUNC, 0755);
    else
        fd = STDOUT_FILENO;

    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;

    buffer_read = (char*) malloc(sizeof(char) * 528);

    pkt_t* pkt_receiver = pkt_new();
    if(!pkt_receiver){
        fprintf(stderr, "Error creating receiving packet\n");
        free(buffer_read);
        return;
    }

    int endOfFile = 0;

    fd_set read_set;
    int len_set;
    pkt_status_code return_status;

    while(!endOfFile){

        FD_ZERO(&read_set);
        FD_SET(socket, &read_set);
        len_set = (socket < fd) ? fd + 1 : socket + 1;

        struct timeval new_tv = tv;

        select(len_set, &read_set, NULL, NULL, &new_tv);
        /*
            Lecture sur le socket
        */
        if(FD_ISSET(socket, &read_set)){
            int length = read(socket, (void*) buffer_read, 528);

            if(length < 0){
                fprintf(stderr, "Error reading from socket\n");
                //TODO Send Nack
                continue;
            }
            else if(length == 0){
                endOfFile = 1;
            }
            else{ // length > 0
                return_status = pkt_decode((const char*) buffer_read, length, pkt_receiver);
                if(return_status != PKT_OK){
                    /*
                        Not a good packet, send nack
                    */
                    continue;
                }
                else{
                    if(pkt_get_type(pkt_receiver) != PTYPE_DATA ||  pkt_get_tr(pkt_receiver) == 1){
                        /*
                            Not good data type, send nack
                        */
                        continue;
                    }
                    else{
                        int len_received = pkt_get_length(pkt_receiver);
                        // End of transmission
                        if(len_received == 0){
                            //TODO Send ack
                            continue;
                        }
                        if(strcmp(pkt_get_payload(pkt_receiver), END) == 0){
                            break;
                        }
                        length = write(fd, (void*) pkt_get_payload(pkt_receiver), len_received);
                        if(length < 0){
                            //TODO Send nack
                            continue;
                        }


                    }
                }
            }

        }

    }

    free(buffer_read);
    pkt_del(pkt_receiver);
    close(fd);

}

int send_ack(pkt_t *pkt_ack, int seqnum, int sfd, int ack, uint32_t time_data, int window){
    return 1;
}
