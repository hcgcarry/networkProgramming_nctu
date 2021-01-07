
#include "np_simple.h"
#include"npshell_simple.h"
//#include"npshell.h"
using namespace std;

int main(int argc, char* argv[],char* envp[]) {

    //char *service = "echo";  /* service name or port number */
    int port = 7001;
    struct sockaddr_in fsin; /* the address of a client */
    int alen;                /* length of client's address */
    int msock;               /* master server socket */
    int ssock;               /* slave server socket */
    switch (argc)
    {
    case 1:
        break;
    case 2:
        port = atoi(argv[1]);
        break;
    default:
        fprintf(stderr,"usage: TCPechod [port]\n");
    }
    msock = passiveTCP(port, QLEN);
    //signal(SIGCHLD,(sighandler_t) reaper);
    signal(SIGCHLD, reaper);
    //npshell(argc,argv,envp);
    cout << "listen on port " << port << endl;
    while (1)
    {
        
        cout << " new iteration====" << endl;
        alen = sizeof(fsin);
        ssock = accept(msock, (struct sockaddr *)&fsin,(socklen_t*) &alen);
        cout << " accept" << endl;
        if (ssock < 0)
        {
            if (errno == EINTR)
                continue;
            perror("accept error");
        }

        switch (fork())
        {
        case 0: /* child */
            close(msock);
            close(0);
            close(1);
            close(2);
            dup(ssock);
            dup(ssock);
            dup(ssock);
            exit(npshell());
        default: /* parent */
            close(ssock);
            break;
        case -1:
            perror("fork error");
        }
    }
}

int TCPechod(int fd) {

    char buf[bufsize];
    int cc;
    while (cc = read(fd, buf, sizeof(buf)))
    {
        if (cc < 0)
            perror("read error");
        if (write(fd, buf, cc) < 0)
            perror("write error");
    }
    return 0;
}

void reaper(int status)
{
    while (waitpid(-1,0, WNOHANG)>0){
    }
}