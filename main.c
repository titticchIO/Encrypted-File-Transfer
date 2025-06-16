#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

int main()
{
    char hostname[256];
    gethostname(hostname, sizeof(hostname));

    struct hostent *host_entry = gethostbyname(hostname);

    char *ip = inet_ntoa(*((struct in_addr *)host_entry->h_addr_list[0]));
    printf("IP locale: %s\n", ip);
    return 0;
}
