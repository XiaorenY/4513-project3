/* 

multicast.c

- Mark Claypool, 2002

The following program sends or receives multicast packets. If invoked
with one argument, it sends a packet containing the current time to an
arbitrarily chosen multicast group and UDP port. If invoked with no
arguments, it receives and prints these packets. Start it as a sender on
just one host and as a receiver on all the other hosts.

(Note, this is similar to the multicast.c example but it uses
the msock wrappers.)

*/

#include <stdio.h>
#include <time.h>
#include <string.h>
#include "msock.h"

#define EXAMPLE_PORT 16000
#define EXAMPLE_ADDR "239.0.0.1"
#define MESSAGE_LEN 75
#define SLEEP_TIME 3

int main(int argc) {
   char message[MESSAGE_LEN];
   int len, sock, cnt;

   if (argc > 1) {
     
     printf("I am a sender. Sending time...\n");
     
     /* set up socket */
     if ((sock=msockcreate(SEND, EXAMPLE_ADDR, EXAMPLE_PORT)) < 0) {
       perror("msockcreate");
       exit(1);
     }

     /* sender sends time every 3 seconds */
     while (1) {
       time_t t = time(0);	/* get current time */
       sprintf(message, "time at %s is %-24.24s", 
	       getenv("HOST"), ctime(&t)); /* make readable */
       printf("sending: %s\n", message);
       cnt = msend(sock, message, strlen(message)+1);
       if (cnt < 0) {
	 perror("msend");
	 exit(1);
       }
       sleep(SLEEP_TIME);
     }
   } else {
     
     printf("I am a receiver.  Waiting for messages...\n");
     
     /* set up socket */
     if ((sock=msockcreate(RECV, EXAMPLE_ADDR, EXAMPLE_PORT)) < 0) {
       perror("msockcreate");
       exit(1);
     }

     /* receiver plays out messages */
     while (1) {
       cnt = mrecv(sock, message, MESSAGE_LEN);
       if (cnt < 0) {
	 perror("mrecv");
	 exit(1);
       } else if (cnt==0) {
	 break;
       }
       printf("message = \"%s\"\n", message);
     }
   }
}


