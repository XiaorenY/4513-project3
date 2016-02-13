/*
Xiaoren Yang

Reference code from Mark Claypool 

*/
#include <stdio.h>
#include <stdlib.h>
#include "msock.h"

#define QUERY_PORT 7000
#define RESPONSE_PORT 7001
#define QUERY_ADDR "239.0.0.1"
#define RESPONSE_ADDR "239.0.0.2"

#define MESSAGE_LEN 128
#define SLEEP_TIME 3

int main(int argc, char *argv[])
{
	char message[MESSAGE_LEN];
	int len, sock, cnt;

	/* client */
	if(argc > 1){
		printf("Client started.\n");

		/* set up socket */
		if ((sock=msockcreate(SEND, QUERY_ADDR, QUERY_PORT)) < 0) {
			perror("msockcreate");
			exit(1);
		}


		printf("Enter movie name: %s\n", argv[1]);
		printf("Sending search request\n");

		cnt = msend(sock, argv[1], strlen(argv[1])+1);
		if (cnt < 0) {
			perror("msend");
			exit(1);
		}

		printf("Waiting for response ...\n");



	}
	/* server */
	else {
		printf("Server started\n");

		/* open the config.nutella file to cache all movies here */
		FILE *fp;
		char *line = NULL;
		size_t len = 0;
		ssize_t read = 0;

		fp = fopen("config.nutella", "r");
		if(fp == NULL){
			fprintf(stderr, "Server can't find config.nutella file.\n");
			exit(EXIT_FAILURE);
		}
		/* find the config file */
		while((read = getline(&line, &len, fp)) != -1) {
			printf("Retrieved line of length %zu :\n", read);
			printf("%s", line);			
		}
		fclose(fp);
		if(line)
			free(line);

		/* set up socket */
		if ((sock=msockcreate(RECV, QUERY_ADDR, QUERY_PORT)) < 0) {
			perror("msockcreate");
			exit(1);
		}

		printf("Listening...\n");

		/* receiver plays out messages */
		cnt = mrecv(sock, message, MESSAGE_LEN);
		if (cnt < 0) {
			perror("mrecv");
			exit(1);
		}

		printf("Received search request %s\n", message);
	}

}
