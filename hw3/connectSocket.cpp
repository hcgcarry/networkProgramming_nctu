#include "common.h"
#include <iostream>
#include <vector>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h> //struct stat
#include <unistd.h>

using namespace std;
int connectsock(string ip, int port, string protocol);
int connectTCP(string ip, int port)
{
    return connectsock(ip, port, "tcp");
}

int connectsock(string ip, int port, string protocol)
{

    struct hostent *phe;  /* pointer to host information entry */
    struct servent *pse;  /* pointer to service information entry */
    struct protoent *ppe; /* pointer to protocol information entry*/

    struct sockaddr_in sin; /* an Internet endpoint address */
    int s, type;            /* socket descriptor and socket type */

    bzero((char *)&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    if ((sin.sin_addr.s_addr = inet_addr(ip.c_str())) == INADDR_NONE)
        perror("can't get host entry\n");

    if (protocol == "udp")
        type = SOCK_DGRAM;
    else
        type = SOCK_STREAM;

    /* Map service name to port number */

    /* Use protocol to choose a socket type */
    /* Allocate a socket */
    s = socket(PF_INET, type, 0);

    if (s < 0)
        perror("can't create socket");
    /* Connect the socket */
    if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
        perror("can't connect ");
    return s;
}
/*
int main(void){
    connectTCP("127.0.0.1",7777);
    cout << "finish" << endl;
}
*/