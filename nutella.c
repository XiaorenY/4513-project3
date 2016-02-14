/*
Xiaoren Yang

Reference code from Mark Claypool 

*/

#include "msock.h"
#include "pnode.h"
#include "tcptalk.h"
#include <time.h>

#define QUERY_PORT 7000
#define RESPONSE_PORT 7001
#define QUERY_ADDR "239.0.0.1"
#define RESPONSE_ADDR "239.0.0.2"

#define MESSAGE_LEN 128
#define SLEEP_TIME 3
#define TIMEOUT 3000

char* concat(char *s1, char *s2);

int main(int argc, char *argv[])
{
	if(argc != 2){
		fprintf(stderr, "Need and only need set port number for server.\n");
		exit(-1);
	}

	char message[MESSAGE_LEN];
	char buf[BUFFSIZE];
	int len, multisock, unisock, cnt, clilen, newsock;
	int pid; 	/* process id */

	/* fork child process here */
	pid = fork();
	if(pid == 0){
		/* client */
		printf("Client started.\n");

		/* set up socket */
		if ((multisock=msockcreate(SEND, QUERY_ADDR, QUERY_PORT)) < 0) {
			perror("msockcreate");
			exit(1);
		}

		/* set up timeout for recev multicast message */
		struct timeval tv;
		tv.tv_sec = 3;
		tv.tv_usec = 0;
		if(setsockopt(multisock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0){
			perror("setsockopt()");
			exit(-1);
		}

		char moviename[20];
		printf("Enter movie name: \n");
		scanf("%s", &moviename);

		printf("Sending search request\n");

		cnt = msend(multisock, moviename, strlen(moviename)+1);
		if (cnt < 0) {
			perror("msend");
			exit(1);
		}

		char *getmoviename;
		char *gethostname;
		int getport;
		struct sockaddr_in serv_addr;
		struct hostent *hp;

		while(1){
			printf("Waiting for response ...\n");
			/* receive a response from a server */
			cnt = mrecv(multisock, message, MESSAGE_LEN);
			if (cnt < 0) {
				perror("mrecv");
				if((errno == EAGAIN) || (errno == EWOULDBLOCK))
					fprintf(stderr, "Timeout! No server response to this moviename\n");
				exit(1);
			}
			printf("Client receive info message, %s\n", message);
			getmoviename = strtok(message, ",");
			if(strcmp(moviename, getmoviename) != 0){
				fprintf(stderr, "movie name doesn't match, %s - %s\n", moviename, getmoviename);
				continue;
			}
			else {
				break;
			}
		}

		gethostname = strtok(NULL, ",");
		getport = atoi(strtok(NULL, ","));
		printf("Hostname received from server is %s, port %d\n", gethostname, getport);

		/* lookup hostname.
		* (Note - gethostbyname() deprecated, could use getaddrinfo())
		*/
		bzero((void *) &serv_addr, sizeof(serv_addr));
		printf("Looking up %s...\n", gethostname);
		if ((hp = gethostbyname(gethostname)) == NULL) {
			perror("host name error");
			exit(1);
		}
		bcopy(hp->h_addr, (char *) &serv_addr.sin_addr, hp->h_length);

		printf("Found it.  Setting port connection to %d...\n", getport);
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_port = htons(getport);
		/* serv_addr.sin_addr.s_addr = inet_addr(serv_host_addr); */

		/* create a TCP socket (an Internet stream socket). */
		puts("Done. Creating socket...");
		if ((unisock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			perror("creating socket");
			exit(1);
		}

		/* socket created, so connect to the server */
		puts("Created. Trying connection to server...");
		if (connect(unisock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
			perror("can't connect");
			exit(1);
		}
			
		printf("Connection established!\n");

		writeSocket(unisock, "movie start", BUFFSIZE);

		readSocket(unisock, buf, BUFFSIZE);

		while(strcmp(buf, "movie end") != 0) {
			printf("%s", buf);
			readSocket(unisock, buf, BUFFSIZE);
		}


		/* read from stdin, sending to server, until quit */
		// while (fgets(buf, 80, stdin)) {
		// 	buf[strlen(buf)-1] = '\0'; /* remove last \n */
		// 	printf("sending: '%s'\n", buf);
		// 	if (write(unisock, buf, strlen(buf)) == -1) {
		// 		perror("write failed");
		// 		break;
		// 	}
		// }

		/* close socket */
		close(unisock);
		// readSocket(unisock, message, BUFFSIZE);
		// printf("%s\n", message);
		msockdestroy(multisock);
	}
	else {		/* parent process deals with server part */
		printf("Server started\n");

		pNode head=NULL, tail=NULL;
		initList(&head, &tail);

		/* open the config.nutella file to cache all movies here */
		FILE *fp, *movieptr;
		char *line = NULL;
		size_t len = 0;
		ssize_t readline = 0;

		fp = fopen("config.nutella", "r");
		if(fp == NULL){
			fprintf(stderr, "Server can't find config.nutella file.\n");
			exit(EXIT_FAILURE);
		}
		/* find the config file */
		while((readline = getline(&line, &len, fp)) != -1) {
			printf("server Retrieved line of length %zu :\n", readline);
			printf("%s", line);		
			line[readline-1]='\0';
			addNode(&head, &tail, line);
		}
		if(fp){
			fclose(fp);
			fp = NULL;
		}

		if(line)
			free(line);

		
		/* set up multicast socket */
		if ((multisock=msockcreate(RECV, QUERY_ADDR, QUERY_PORT)) < 0) {
			perror("msockcreate");
			exit(1);
		}
		
		/* set up unicast socket */
		int serv_host_port;
		struct sockaddr_in cli_addr, serv_addr;
		serv_host_port = atoi(argv[1]);
		/* create socket from which to read */
		if ((unisock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			perror("creating socket");
			exit(1);
		}
		/* bind our local address so client can connect to us */
		bzero((char *) &serv_addr, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		serv_addr.sin_port = htons(serv_host_port);
		if (bind(unisock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
			perror("can't bind to local address");
			exit(1);
		}

		/**/
		char hostname[50];
		gethostname(hostname, 50);
		// printf("sssss\n");
		printf("%s\n", hostname);
		/**/
		printf("Server listening...\n");

		char info[100];
		int bytes;
		char *frame=malloc(BUFFSIZE);
		// char *frame = "";
		struct timespec tim, tim2;
		tim.tv_sec = 1;
		tim.tv_nsec = 100000000;

		while(1){
			/* receiver plays out messages */
			cnt = mrecv(multisock, message, MESSAGE_LEN);
			if (cnt < 0) {
				perror("mrecv");
				exit(1);
			}
			printf("Server received search request %s\n", message);
			
			// travList(head);
			/* movie is here */
			if(findMovie(head, message)){
				printf("Server has the match movie %s\n", message);	

				/* multicase a message with moviename, 
					IP address and port number*/
				sprintf(info, "%s,%s,%s", message, hostname, argv[1]);
				printf("Sending an info message to client, %s\n", info);
				cnt = msend(multisock, info, strlen(info)+1);
				if (cnt < 0) {
					perror("msend");
					exit(1);
				}

				/* then ready to accept the tcp connection from client */
				if (listen(unisock, 5) == -1) {
					perror("listen");
					exit(1);
				}
				/* wait here (block) for connection */
				clilen = sizeof(cli_addr);
				printf("wait here for accepting connection from client\n");
				newsock = accept(unisock, (struct sockaddr *) &cli_addr, &clilen);
				printf("Socket ready to go! Accepting connections....\n\n");
				if (newsock < 0) {
					perror("accepting connection");
					exit(1);
				}
				
				// writeSocket(unisock, "abc", 4);
				/* read data until no more */
				// while ((bytes = read(newsock, message, 1024)) > 0) {
				// 	message[bytes] = '\0';  do this just so we can print as string 
				// 	printf("received: '%s'\n", message);
				// }

				// if (bytes == -1)
				// 	perror("error in read");
				// else
				// 	printf("server exiting\n");
	
				readSocket(newsock, buf, BUFFSIZE);
				printf("received: '%s'\n", buf);

				/* open the movie file and then send it to client */
				movieptr = fopen(message, "r");
				if(movieptr == NULL){
					fprintf(stderr, "Server can't find %s.\n", message);
					exit(EXIT_FAILURE);
				}
				/* find the config file */
				while((readline = getline(&line, &len, movieptr)) != -1) {
					// printf("server Retrieved line of length %zu :\n", readline);
					printf("%s", line);		
					if(strcmp(line, "end\n") == 0){
						/* sleep a little bit here */
						if(nanosleep(&tim, &tim2) < 0){
							fprintf(stderr, "Nano sleep failed\n");
							exit(-1);
						}

						writeSocket(newsock, frame, BUFFSIZE);
						// memset(&frame ,0, sizeof(frame));
						strcpy(frame, "");
						// free(frame);
						// frame=malloc(BUFFSIZE);
						// frame="";
					}
					else {
						// frame = concat(frame, line);
						strcat(frame, line);
					}
				}
				printf("sssss\n");
	
				if(movieptr){
					fclose(movieptr);
					movieptr = NULL;
				}				
				printf("tttttt\n");
				if(line)
					free(line);

				writeSocket(newsock, "movie end", BUFFSIZE);

				/* close connected socket and original socket */
				close(newsock);
			}
			else{ 	/* movie is not here */
				printf("Movie not here, so not response\n");
			}		
		}
		msockdestroy(multisock);

	}
}

char* concat(char *s1, char *s2)
{
	char *result = malloc(strlen(s1)+strlen(s2)+1);//+1 for the zero-terminator
	//in real code you would check for errors in malloc here
	strcpy(result, s1);
	strcat(result, s2);
	return result;
}
