#include "network_layer.h"

void show_list(user_info *me) {
	int sent;
	packet_struct packet;
	
	// do not display the list unless the user is connected
	if(!is_connected) {
		printf("You are not connected...\n");
		return;
	}
	
	// build a packet to request the list from the server
	memset(&packet, 0, sizeof(packet_struct));
	strcpy(packet.option, "list");
	strcpy(packet.alias, me->alias);
	
	// send request to view client list
	sent = send(sockfd, (void *)&packet, sizeof(packet_struct), 0);
}

// this function displays usage information for the commands supported by the chat
void show_help() {
	printf("List of commands supported by the chat room:\n\n");
	printf("\t- Login - Allow a user to join the chat room. Should specify a user name.\n");
	printf("\t\tExample: \n");
	printf("\t\t\tlogin Tyler\n\n");

	printf("\t- Send - Send a message to all users in the chat room.\n");
	printf("\t\tExample: \n");
	printf("\t\t\tsend Hello there!\n\n");

	printf("\t- Whisper - Send a private message to a single user in the chat room by specifying their name.\n");
	printf("\t\tExample: \n");
	printf("\t\t\twhisper Tyler Hello there!\n\n");

	printf("\t- List - Display a list of all users in the chat room.\n");
	printf("\t\tExample: \n");
	printf("\t\t\tlist\n\n");

	printf("\t- Help - Display these instructions about how to use the chat room commands.\n");
	printf("\t\tExample: \n");
	printf("\t\t\thelp\n\n");

	printf("\t- Logout - Allow a user to exit the chat room on their own free will.\n");
	printf("\t\tExample: \n");
	printf("\t\t\tlogout\n\n");
}
 
void login(user_info *me) {
    int recvd;
	// check if the user has connected previously
    if(is_connected) {
        printf("You are already connected to server at %s:%d\n", SERVERIP, SERVERPORT);
        return;
    }
	
	// connect the user to the server
    sockfd = connect_with_server();
	// if we get a valid response, the user is now connected to the server
    if(sockfd >= 0) {
        is_connected = 1;
        me->sockfd = sockfd;
        if(strcmp(me->alias, "Anonymous")) 
			setalias(me);
		
        printf("Logged in as %s\n", me->alias);
        printf("Receiver started [%d]...\n", sockfd);
        // create a thread for the client to talk on
		client_thread_info_struct threadinfo;
        pthread_create(&threadinfo.thread_ID, NULL, receiver, (void *)&threadinfo);
    }
    else {
        printf("Connection refused...\n");
    }
}
 
int connect_with_server() {
    int newfd, err_ret;
    struct sockaddr_in serv_addr;
    struct hostent *to;
 
    // generate address
    if((to = gethostbyname(SERVERIP))==NULL) {
        err_ret = errno;
        printf("gethostbyname() error...\n");
        return err_ret;
    }
 
    // open socket
    if((newfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        err_ret = errno;
        printf("socket() error...\n");
        return err_ret;
    }
 
    // set initial values
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVERPORT);
    serv_addr.sin_addr = *((struct in_addr *)to->h_addr);
    memset(&(serv_addr.sin_zero), 0, 8);
 
    // try to connect with server
    if(connect(newfd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr)) == -1) {
        err_ret = errno;
        printf("connect() error...\n");
        return err_ret;
    }
    else {
        printf("Connected to server at %s:%d\n", SERVERIP, SERVERPORT);
        return newfd;
    }
}
 
void logout(user_info *me) {
    int sent;
    packet_struct packet;
    
	// make sure the user is connected
    if(!is_connected) {
        printf("You are not connected...\n");
        return;
    }
    
    memset(&packet, 0, sizeof(packet_struct));
    strcpy(packet.option, "exit");
    strcpy(packet.alias, me->alias);
    
    // send request to close connection
    sent = send(sockfd, (void *)&packet, sizeof(packet_struct), 0);
    is_connected = 0;
}
 
void setalias(user_info *me) {
    int sent;
    packet_struct packet;
    
    if(!is_connected) {
        printf("You are not connected...\n");
        return;
    }
    
	// build a packet to request a nickname change from the server
    memset(&packet, 0, sizeof(packet_struct));
    strcpy(packet.option, "change");
    strcpy(packet.alias, me->alias);
    
    // send request to close connection
    sent = send(sockfd, (void *)&packet, sizeof(packet_struct), 0);
}
 
void *receiver(void *param) {
    int recvd;
    packet_struct packet;
    
    printf("Waiting here [%d]...\n", sockfd);
    while(is_connected) {
        // listen for a received packet
        recvd = recv(sockfd, (void *)&packet, sizeof(packet_struct), 0);
        // if nothing comes in, the client has lost its connection
		if(!recvd) {
            printf("Connection lost from server...\n");
            is_connected = 0;
			// close the socket to free it up
            close(sockfd);
            break;
        }
		
		// if the value is valid, display the contents of the packet
        if(recvd > 0) {
            printf("[%s]: %s\n", packet.alias, packet.buff);
        }
        memset(&packet, 0, sizeof(packet_struct));
    }
    return NULL;
}
 
void sendtoall(user_info *me, char *msg) {
    int sent;
    packet_struct packet;
    
	// make sure the user is connected before sending messages
    if(!is_connected) {
        printf("You are not connected...\n");
        return;
    }
    
    msg[BUFFSIZE] = 0;
    
    memset(&packet, 0, sizeof(packet_struct));
	
	// build a packet to send info to all users
    strcpy(packet.option, "send");
    strcpy(packet.alias, me->alias);
    strcpy(packet.buff, msg);
    
    sent = send(sockfd, (void *)&packet, sizeof(packet_struct), 0);
}
 
void whisper(user_info *me, char *target, char *msg) {
    int sent, targetlen;
    packet_struct packet;
    
	// don't send to invalid usernames
    if(target == NULL) {
        return;
    }
    
	// don't send blank messages
    if(msg == NULL) {
        return;
    }
    
	// make sure the client is connected to the server 
    if(!is_connected) {
        printf("You are not connected...\n");
        return;
    }
    msg[BUFFSIZE] = 0;
    targetlen = strlen(target);
    
	// build a packet to send info to a user
    memset(&packet, 0, sizeof(packet_struct));
    strcpy(packet.option, "whisper");
    strcpy(packet.alias, me->alias);
    strcpy(packet.buff, target);
    strcpy(&packet.buff[targetlen], " ");
    strcpy(&packet.buff[targetlen+1], msg);
    
    // send request to close connection
    sent = send(sockfd, (void *)&packet, sizeof(packet_struct), 0);
}