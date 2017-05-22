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

#define BUFFSIZE 1024
#define MAX_ALIAS_SIZE 32
#define OPTLEN 16
#define CLIENTS 10

typedef struct packet_struct {
    char option[OPTLEN];            // instruction
    char alias[MAX_ALIAS_SIZE];     // clients alias
    char buff[BUFFSIZE];            // payload
} packet_struct;

typedef struct thread_info_struct {
    pthread_t thread_ID;           // threads pointer
    int sockfd;                    // socket file descriptor
    char alias[MAX_ALIAS_SIZE];    // client alias
} thread_info_struct;

typedef struct node {
    thread_info_struct threadinfo;
    struct node *next;
} node;
 
typedef struct linked_list {
    node *head;
	node *tail;
    int size;
} linked_list;

linked_list client_list;
pthread_mutex_t clientlist_mutex;