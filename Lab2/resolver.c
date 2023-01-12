// C program to display hostname
// and IP address
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

void checkHostEntry(struct hostent * hostentry)
{
	if (hostentry == NULL)
	{
		printf("Not found information.");
		exit(1);
	}
}

int main(int agrv, char** argc)
{
	struct hostent *host_entry;

// Check if input is a domain name or an IP
    if(agrv <=1) {
        printf("No input value!!!");
    }
    else {
        int isIP = 1;
        for( int i=0; i<strlen(argc[1]); i++)
        {
            if(argc[1][i] >= 'a' && argc[1][i] <= 'z') {
                isIP = 0;
                break;
            }
        }
// If input is a domain name
        if(isIP == 0) {
        	host_entry = gethostbyname(argc[1]);
	        checkHostEntry(host_entry);

	        printf("Official IP: %s\n", inet_ntoa(*((struct in_addr*)host_entry->h_addr)));
            printf("Alias IP:\n");
            char **iAlias;
            for(iAlias = host_entry->h_addr_list; *iAlias != 0; iAlias++) {
                printf("%s\n", inet_ntoa(*((struct in_addr*)iAlias)));
            }

        }
// If input is an IP address
        else {
            host_entry = gethostbyaddr(argc[1],4,AF_INET);
            checkHostEntry(host_entry);
            printf("Official name: %s\n", host_entry->h_name);
            printf("Alias name:\n");
            char **pAlias; 
            for (pAlias = host_entry->h_aliases; *pAlias != 0; pAlias++) {
                printf("%s\n", *pAlias);
            }

        }
    }

	return 0;
}
