/* clDNS.c */

#include <stdlib.h>
#include <stdio.h>
#include "cnaiapi.h"

#define BUFFSIZE		256
#define INPUT_PROMPT		"Enter Domain Name > "
#define RECEIVED_PROMPT		"IP Address > "

int readln(char *, int);

/*-----------------------------------------------------------------------
 *
 * Program: clDNS
 * Purpose: contact servDNS, send user input and print server response
 * Usage:   clDNS <compname> [appnum]
 * Note:    Appnum is optional. If not specified, appnum
 *          20000 is used.
 *
 *-----------------------------------------------------------------------
 */
int
main(int argc, char *argv[])
{
	computer	comp;
	appnum		app;
	connection	conn;
	char		buff[BUFFSIZE];
	int		expect, received, len;

	if (argc < 2 || argc > 3) {
		(void) fprintf(stderr, "usage: %s <compname> [appnum]\n",
			       argv[0]);
		exit(1);
	}

	/* convert the arguments to binary format comp and appnum */

	comp = cname_to_comp(argv[1]);
	if (comp == -1)
		exit(1);

	if (argc == 3)
		app = (appnum) atoi(argv[2]);
	else
		app=20000;

	/* form a connection with servDNS */

	conn = make_contact(comp, app);
	if (conn < 0)
		exit(1);
	
	/* if connection successful */
	
	printf("\nConnection with server established.\n\n");

	(void) printf(INPUT_PROMPT);
	(void) fflush(stdout);

	/* iterate: read input from the user, send to the server,	*/
	/*	    receive reply from the server, and display for user */
	
	memset(buff, 0, sizeof buff);

	while((len = readln(buff, BUFFSIZE)) > 0) {
		
		/* If user pressed enter without entering anything else, break */
		if (buff[0] == '\n') 
			break;

		/* send the input to DNS server */
		buff[len - 1] = '\0';
		(void) send(conn, buff, len, 0);
		(void) printf(RECEIVED_PROMPT);
		(void) fflush(stdout);
		memset(buff, 0, sizeof buff);
		
		/* receive and print from the DNS server */
		if ((len = recv(conn, buff, BUFFSIZE, 0)) < 1)
			break;
		(void) write(STDOUT_FILENO, buff, len);
		(void) printf("\n");
		(void) printf(INPUT_PROMPT);
		(void) fflush(stdout);
	}

	/* iteration ends when EOF found on stdin */

	(void) send_eof(conn);
	(void) printf("\n");
	return 0;
}
