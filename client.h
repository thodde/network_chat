#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
 
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "common.h"

#define SERVERIP "127.0.0.1"
#define SERVERPORT 8080
 
#define LINEBUFF 2048

// client related structs
typedef struct user_info {
        int sockfd;                 // users socket descriptor
        char alias[MAX_ALIAS_SIZE]; // users name
} user_info;
 
typedef struct client_thread_info_struct {
    pthread_t thread_ID;             // thread pointer
    int sockfd;                      // socket file descriptor
} client_thread_info_struct;

// global variables
int is_connected;
int sockfd;
char option[LINEBUFF];
user_info me;
user_info server_info;
 
// function prototypes
void show_help();
int connect_with_server();
void setalias(user_info *me);
void logout(user_info *me);
void login(user_info *me);
void *receiver(void *param);
void show_list(user_info *me);
void sendtoall(user_info *me, char *msg);
void sendtoalias(user_info *me, char * target, char *msg);