#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
 
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
 
#include "list.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
 
#define IP "127.0.0.1"
#define PORT 8080
#define BACKLOG 10
#define LINEBUFF 2048
 
// global variables
int sockfd;
int newfd;
thread_info_struct thread_info[CLIENTS];
 
// function prototypes
void *io_handler(void *param);
void *client_handler(void *fd);
 
int main(int argc, char **argv) {
    int err_ret;
	int sin_size;
    struct sockaddr_in serv_addr, client_addr;
    pthread_t interrupt;
 
    // create linked list of clients
    list_init(&client_list);
 
    // create mutex for managing the client list
    pthread_mutex_init(&clientlist_mutex, NULL);
 
    // open socket
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        err_ret = errno;
        printf("socket() failed...\n");
        return err_ret;
    }
 
    // set initial values
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = inet_addr(IP);
    memset(&(serv_addr.sin_zero), 0, 8);
 
    // bind address with socket
    if(bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr)) == -1) {
        err_ret = errno;
        printf("bind() failed...\n");
        return err_ret;
    }
 
    // listen for connection
    if(listen(sockfd, BACKLOG) == -1) {
        err_ret = errno;
        printf("listen() failed...\n");
        return err_ret;
    }
 
    // initiate interrupt handler for IO
    printf("Starting server...\n");
    if(pthread_create(&interrupt, NULL, io_handler, NULL) != 0) {
        err_ret = errno;
        printf("pthread_create() failed...\n");
        return err_ret;
    }
 
    // accept connections until the client list reaches its MAX
    printf("Starting socket listener...\n");
    while(1) {
        sin_size = sizeof(struct sockaddr_in);
		printf("Server ready\n");
        if((newfd = accept(sockfd, (struct sockaddr *)&client_addr, (socklen_t*)&sin_size)) == -1) {
            err_ret = errno;
            printf("accept() failed...\n");
            return err_ret;
        }
        else {
            if(client_list.size == CLIENTS) {
                printf("Connection full, request rejected...\n");
                continue;
            }
            printf("Connection requested received...\n");
            thread_info_struct threadinfo;
            threadinfo.sockfd = newfd;
            strcpy(threadinfo.alias, "Anonymous");
            pthread_mutex_lock(&clientlist_mutex);
            list_insert(&client_list, &threadinfo);
            pthread_mutex_unlock(&clientlist_mutex);
            pthread_create(&threadinfo.thread_ID, NULL, client_handler, (void *)&threadinfo);
        }
    }
 
    return 0;
}
 
void *io_handler(void *param) {
    char option[OPTLEN];
    while(scanf("%s", option) == 1) {
        if(!strcmp(option, "exit")) {
            // kill server on exit
            printf("Terminating server...\n");
            pthread_mutex_destroy(&clientlist_mutex);
            close(sockfd);
            exit(0);
        }
        else if(!strcmp(option, "list")) {
            // display the list on the server side
            pthread_mutex_lock(&clientlist_mutex);
            list_dump(&client_list);
            pthread_mutex_unlock(&clientlist_mutex);
        }
        else {
            printf("Command not supported: %s...\n", option);
        }
    }
    return NULL;
}
 
void *client_handler(void *fd) {
    thread_info_struct threadinfo = *(thread_info_struct *)fd;
    packet_struct packet;
    node *curr;
    int bytes, sent;
	
    while(1) {
        bytes = recv(threadinfo.sockfd, (void *)&packet, sizeof(packet_struct), 0);
        if(!bytes) {
            printf("Connection lost from [%d] %s...\n", threadinfo.sockfd, threadinfo.alias);
            pthread_mutex_lock(&clientlist_mutex);
            list_delete(&client_list, &threadinfo);
            pthread_mutex_unlock(&clientlist_mutex);
            break;
        }
		
		// display the contents of the current packet
        printf("[%d] %s %s %s\n", threadinfo.sockfd, packet.option, packet.alias, packet.buff);
        if(!strcmp(packet.option, "change")) {
            printf("Set username to %s\n", packet.alias);
            pthread_mutex_lock(&clientlist_mutex);
            for(curr = client_list.head; curr != NULL; curr = curr->next) {
                if(compare(&curr->threadinfo, &threadinfo) == 0) {
                    strcpy(curr->threadinfo.alias, packet.alias);
                    strcpy(threadinfo.alias, packet.alias);
                    break;
                }
            }
            pthread_mutex_unlock(&clientlist_mutex);
        }
        else if(!strcmp(packet.option, "whisper")) {
            int i;
            char target[MAX_ALIAS_SIZE];
            for(i = 0; packet.buff[i] != ' '; i++); 
				packet.buff[i++] = 0;
				
            strcpy(target, packet.buff);
            pthread_mutex_lock(&clientlist_mutex);

            // iterate over the list until we find the correct client
            for(curr = client_list.head; curr != NULL; curr = curr->next) {
                if(strcmp(target, curr->threadinfo.alias) == 0) {
                    packet_struct spacket;
                    memset(&spacket, 0, sizeof(packet_struct));
                    if(!compare(&curr->threadinfo, &threadinfo)) continue;
                    strcpy(spacket.option, "msg");
                    strcpy(spacket.alias, packet.alias);
                    strcpy(spacket.buff, &packet.buff[i]);
                    sent = send(curr->threadinfo.sockfd, (void *)&spacket, sizeof(packet_struct), 0);
                }
            }
            pthread_mutex_unlock(&clientlist_mutex);
        }
        else if(!strcmp(packet.option, "send")) {
            pthread_mutex_lock(&clientlist_mutex);

            // iterate over the list until we have send the message to each user
            for(curr = client_list.head; curr != NULL; curr = curr->next) {
                packet_struct spacket;
                memset(&spacket, 0, sizeof(packet_struct));
				
				// don't send to yourself
                if(!compare(&curr->threadinfo, &threadinfo)) 
					continue;
				
                // build a packet to send to the current client
                strcpy(spacket.option, "msg");
                strcpy(spacket.alias, packet.alias);
                strcpy(spacket.buff, packet.buff);
				
                sent = send(curr->threadinfo.sockfd, (void *)&spacket, sizeof(packet_struct), 0);
            }
            pthread_mutex_unlock(&clientlist_mutex);
        }
        else if(!strcmp(packet.option, "exit")) {
            printf("[%d] %s has disconnected...\n", threadinfo.sockfd, threadinfo.alias);
            pthread_mutex_lock(&clientlist_mutex);
            // remove the user from the list
            list_delete(&client_list, &threadinfo);
            pthread_mutex_unlock(&clientlist_mutex);
            break;
        }
		else if(!strcmp(packet.option, "list")) {
            printf("List requested by %s.\n", packet.alias);
            // lock the list
            pthread_mutex_lock(&clientlist_mutex);
			char* name_list = malloc(sizeof(char)* BUFFSIZE);

            // iterate through the list and collect the name of each member of the chat
            for(curr = client_list.head; curr != NULL; curr = curr->next) {
                strcat(name_list, curr->threadinfo.alias);
                strcat(name_list, "\n");
            }

            // build a packet to display the list to the requester
            packet_struct spacket;
            memset(&spacket, 0, sizeof(packet_struct));
            strcpy(spacket.option, "msg");
            strcpy(spacket.alias, packet.alias);
            strcpy(spacket.buff, name_list);

            // need this loop to grab the threadinfo from the correct client
            // this makes it possible to send the packet to the correct user
            for(curr = client_list.head; curr != NULL; curr = curr->next) {
                if(strcmp(packet.alias, curr->threadinfo.alias) == 0) {
                    // send the packet
                    sent = send(curr->threadinfo.sockfd, (void *)&spacket, sizeof(packet_struct), 0);
                }
            }

            // unlock the list
            pthread_mutex_unlock(&clientlist_mutex);
        }
        else {
            printf("Garbage data from [%d] %s...\n", threadinfo.sockfd, threadinfo.alias);
        }
    }
 
    // clean up
    close(threadinfo.sockfd);
 
    return NULL;
}
