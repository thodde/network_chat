/*
 * This file is purely for testing purposes. It can be used to 
 * verify that the code works. It starts by connecting to the server
 * and creating a client. Then it proceeds to run each command that
 * is supported by the chat program.
 */
#include "client.h"

int main(int argc, char *argv[]) {
	user_info test_app_alpha;
	user_info server_info;
	char* alias_copy;
	char option[LINEBUFF];
	int done = 1;
	
	// used for storing the names of clients and server
	memset(&server_info, 0, sizeof(struct user_info));
    memset(&test_app_alpha, 0, sizeof(struct user_info));
	
	int port = 8080;
	printf("Beginning test...\n");
	
	if (argc > 1)
        port = atoi(argv[1]);
	
	while(done != 0) {
		// assigns names to server and clients
		memset(test_app_alpha.alias, 0, sizeof(char) * MAX_ALIAS_SIZE);
		memset(server_info.alias, 0, sizeof(char) * MAX_ALIAS_SIZE);
		strcpy(server_info.alias, "server");
        strcpy(test_app_alpha.alias, "test_app_alpha");
		
		// log the user in to the server and send the join message
		printf("[TestApp]: Logging into the server.\n");
		login(&test_app_alpha);
		alias_copy = (char*) malloc(sizeof(char)*strlen(test_app_alpha.alias));
		strcpy(alias_copy, test_app_alpha.alias);
		printf("[TestApp]: Login Successful.\n");
		printf("[TestApp]: Testing send.\n");
		sendtoall(&test_app_alpha, "This is a message.\n");
		printf("[TestApp]: Send completed.\n");
		
		//printf("[TestApp]: Testing list functionality.\n");
		//show_list(&test_app_alpha);
		//printf("[TestApp]: List completed.\n");
		printf("[TestApp]: Logging out.\n");
		logout(&test_app_alpha);
		
		// remove later
		done = 0;
	}
	
	return 0;
}