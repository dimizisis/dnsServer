/* servDNS.c */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cnaiapi.h"

#define BUFFSIZE		256
#define CACHESIZE		5
#define DEFAULT_PORT		20000
#define FOUND_LOCALLY		"LOCAL"
#define NOT_FOUND_LOCALLY		"non-LOCAL"
#define NOT_FOUND		"-1 UKNOWN"

/* cache data type for keeping symbolic names and its IP adresses */

typedef struct {
	char sname[200];
	computer IP;
} cache; 

/*-----------------------------------------------------------------------
 *
 * Program: servDNS
 * Purpose: wait for a connection from clDNS (client) and return converted symbolic name
 * Usage:   servDNS <appnum>
 * Note:    Appnum is optional. If not specified, appnum
 *          20000 is used.
 *-----------------------------------------------------------------------
 */

void 
print_IP_to_buffer(computer IP, char buff[BUFFSIZE], int flag);
 
int 
exists_in_cache(char name[], cache c[], int size);

int
add_record_to_cache(char name[], computer IP, cache c[], int *size);
 
int
main(int argc, char *argv[])
{
	connection	conn;
	computer 	comp;
	int 	len;
	int 	pos;
	int		size=0;
	char	buff[BUFFSIZE];
	cache	cache[CACHESIZE];

	if (argc < 1 || argc > 2) {
		(void) fprintf(stderr, "usage: %s [appnum]\n", argv[0]);
		exit(1);
	}

	/* wait for a connection from client */
	
	/* if user enter specific appnum, use user's */
	if (argc == 2)
		conn = await_contact((appnum) atoi(argv[1]));
	
	/* if not specified, use 20000 */
	
	else
		conn = await_contact(DEFAULT_PORT);
	
	if (conn < 0)
		exit(1);
	
	/* initializing buffer with zeros */		
	memset(buff, 0, sizeof buff);

	/* iterate, until end of file */

	while(len = recv(conn, buff, BUFFSIZE, 0) > 0){
		
		if (len < 1) 
			break;
		
		/* if symbolic name has been found in cache */
		pos = exists_in_cache(buff, cache, size);
		if (pos != -1)
			/* print IP with the correct form (dot-decimal notation) */
			print_IP_to_buffer(cache[pos].IP, buff, 0);
			
		else{
			
			/* comp has the converted IP (or -1 if not found) */
			comp = cname_to_comp(buff);
			
			/* if not found */
			if (comp == -1)
				print_IP_to_buffer(comp, buff, -1);
			
			/* else, add record to cache and print */
			else{
				
					(void) add_record_to_cache(buff, comp, cache, &size);
					
					/* print IP with the correct form (dot-decimal notation) */
					print_IP_to_buffer(comp, buff, 1);
			}
		}
		
		/* send results to client */
		(void) send(conn, buff, strlen(buff), 0); 
		
		/* clear the buffer after sending to client */
		memset(buff, 0, sizeof buff);
		
	}
	
	send_eof(conn);
	return 0;
}

void 
print_IP_to_buffer(computer IP, char buff[BUFFSIZE], int flag){
	
	/* This function prints the IP address with the correct form. Parameter flag is 0 when host name found locally, 1 if found from cname_to_comp function and -1 if host name does not exist. */
	
	/* if host name does not exist */
	if (flag == -1)
		/* print to buffer */
		sprintf(buff, NOT_FOUND);
	else{
	
		unsigned char bytes[4];
	
		/* (number & 0xFF) leaves only the least significant byte */
		bytes[0] = IP & 0xFF;
	
		/* move the 2nd, 3rd and 4th bytes into the lower order byte */
		bytes[1] = (IP >> 8) & 0xFF;
		bytes[2] = (IP >> 16) & 0xFF;
		bytes[3] = (IP >> 24) & 0xFF;   
		
		/* if found locally */
		if(flag == 0)
			/* print to buffer */
			sprintf(buff, "%d.%d.%d.%d " FOUND_LOCALLY, bytes[0], bytes[1], bytes[2], bytes[3]);
		/* if found, but not locally */
		else
			/* print to buffer */
			sprintf(buff, "%d.%d.%d.%d " NOT_FOUND_LOCALLY, bytes[0], bytes[1], bytes[2], bytes[3]);
	
	}
}

int 
exists_in_cache(char name[], cache c[], int size)

/* This function returns the position, where a char array (symbolic name) exists in a cache (data type). If not, returns -1. */

{
	/* for every name in cache, check if it is equal */
	for(int i=0;i<CACHESIZE;++i){
		
		/* if found, return the position in cache */
		if (strcmp(name, c[i].sname) == 0)
			return i;
	}
	
	/* else, return -1 (Error report) */
	return -1;
}

int
add_record_to_cache(char name[], computer IP, cache c[], int *size)

/* This function adds a record to a cache (data type). When cache is full, overwrites cache's first element. */

{
	/* if cache is full, overwrite cache's first element */	
	if((*size) == CACHESIZE)
		(*size)=0;

	/* getting symbolic name to cache */
	sprintf(c[*size].sname, name);

	/* writing IP (data type: computer) to cache */
	c[*size].IP = IP;

	/* increasing cache size */
	++(*size);
	
	/* record successfully added to cache */
	return 0;
}
	