#include "network_layer.h"
 
int main(int argc, char **argv) {
    int sockfd;
	int aliaslen;
	char* alias_copy;
	int gui_on = 0;
	
	if(argc > 1) {
		printf("GUI option enabled.\n");
		gui_on = 1;
	}
    
	memset(&server_info, 0, sizeof(struct user_info));
    memset(&me, 0, sizeof(struct user_info));
    printf("Client ready: \n");
	
    while(gets(option)) {
		// check if the user would like to exit
        if(!strncmp(option, "exit", 4)) {
            logout(&me);
            break;
        }
		
		// display a list of supported options if the user needs help
		// TODO: clean this up because its ugly
        if(!strncmp(option, "help", 4)) {
			show_help();
        }
        else if(!strncmp(option, "login", 5)) {  // log the user into the server
            char *ptr = strtok(option, " ");
            ptr = strtok(0, " ");
            memset(me.alias, 0, sizeof(char) * MAX_ALIAS_SIZE);
			memset(server_info.alias, 0, sizeof(char) * MAX_ALIAS_SIZE);
			strcpy(server_info.alias, "server");
			
            if(ptr != NULL) {
                aliaslen =  strlen(ptr);
                if(aliaslen > MAX_ALIAS_SIZE) ptr[MAX_ALIAS_SIZE] = 0;
                strcpy(me.alias, ptr);
            }
            else {
				// if the user does not specify a username, assign "Anonymous"
                strcpy(me.alias, "Anonymous");
            }
			
			// log the new user in
            login(&me);
			
			// make a copy of the users name so we can announce the arrival to the chat
			alias_copy = (char*) malloc(sizeof(char)*aliaslen);
			strcpy(alias_copy, me.alias);
			
			// send a message to all users that a new user has joined
			sendtoall(&server_info, strcat(alias_copy , " has joined the chat room.\n"));
        }
        else if(!strncmp(option, "change", 5)) {    // if the user would like to change their username
            char *ptr = strtok(option, " ");
            ptr = strtok(0, " ");
            memset(me.alias, 0, sizeof(char) * MAX_ALIAS_SIZE);
            if(ptr != NULL) {
                aliaslen =  strlen(ptr);
                if(aliaslen > MAX_ALIAS_SIZE) ptr[MAX_ALIAS_SIZE] = 0;
                strcpy(me.alias, ptr);
                setalias(&me);
            }
        }
        else if(!strncmp(option, "whisper", 5)) {    // if the user would like to send a private message
            char *ptr = strtok(option, " ");
            char temp[MAX_ALIAS_SIZE];
            ptr = strtok(0, " ");
            memset(temp, 0, sizeof(char) * MAX_ALIAS_SIZE);
            if(ptr != NULL) {
                aliaslen =  strlen(ptr);
                if(aliaslen > MAX_ALIAS_SIZE) ptr[MAX_ALIAS_SIZE] = 0;
                strcpy(temp, ptr);
                while(*ptr) ptr++; ptr++;
                while(*ptr <= ' ') ptr++;
                whisper(&me, temp, ptr);
            }
        }
        else if(!strncmp(option, "send", 4)) {    // if the user would like to talk to the whole chat room
            sendtoall(&me, &option[5]);
        }
		else if(!strncmp(option, "list", 4)) {    // display all the currently connected users
			show_list(&me);
        }
        else if(!strncmp(option, "logout", 6)) {
			// make a copy of the users name so we can announce the arrival to the chat
			alias_copy = (char*) malloc(sizeof(char)*aliaslen);
			strcpy(alias_copy, me.alias);
			
			// send a message to all users that a user has left
			sendtoall(&server_info, strcat(alias_copy , " has left the chat room.\n"));
			
			// exit the chat room
            logout(&me);
        }
        else {
			// catch all
			printf("Command not supported: %s...\n", option);
			show_help();
		}
    }
    return 0;
}


