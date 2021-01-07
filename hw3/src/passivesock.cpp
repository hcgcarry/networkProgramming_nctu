
#include"passivesock.h"
int passiveTCP(int port,int qlen) {
    return passivesock(port, "tcp", qlen);
}

int passivesock(int port ,string protocol, int qlen ) {

    struct servent *pse;    /* pointer to service information                      entry*/
    struct protoent *ppe;   /* pointer to protocol information entry*/
    struct sockaddr_in sin; /* an Internet endpoint address */
    int s, type;            /* socket descriptor and socket type */
    bzero((char *)&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    /* Map service name to port number */
    sin.sin_port = htons(port);

    /* Use protocol to choose a socket type */
    if (protocol ==  "udp")
        type = SOCK_DGRAM;
    else
        type = SOCK_STREAM;
    /* Allocate a socket */
    s = socket(PF_INET, type, 0);
    if (s < 0)
        perror("can't create socket");
    // set socket reuse
    int flag=1;
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int)) < 0){
        perror("can't set socket reuse");
    }
    /* Bind the socket */
    if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
        perror("can't bind on socket");

    if (type == SOCK_STREAM && listen(s, qlen) < 0)
        perror("can't listen on port");
    return s;
}
