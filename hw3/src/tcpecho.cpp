/* TCPtecho.c - main, TCPtecho, reader, writer, mstime */
#include"common.h"
#include"connectSocket.h"
#include"passivesock.h"
#include <sys/types.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <stdio.h>
#include<string.h>
#include<stdlib.h>
#define BUFSIZE 4096     /* write buffer size */
#define CCOUNT 64 * 1024 /* default character count */
#define USAGE "usage: TCPtecho [ -c count ] host1 host2...\n"
char *hname[NOFILE];        /* fd to host name mapping */
int rc[NOFILE], wc[NOFILE]; /* read/write character counts */
char buf[BUFSIZE];           /* read/write data buffer */


int main(int argc, char* argv[]) 
{
    int ccount = CCOUNT;
    int i, hcount, maxfd, fd;
    int one = 1;
    fd_set afds;
    hcount = 0;
    maxfd = -1;
    int port =  atoi(argv[1]);
    for (i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "-c") == 0)
        {
            if (++i < argc && (ccount = atoi(argv[i])))
                continue;
            perror(USAGE);
        }
        /* else, a host */
        fd = connectTCP(argv[i], port);
        if (ioctl(fd, FIONBIO, (char *)&one))
            perror("can't mark socket nonblocking");
        if (fd > maxfd)
            maxfd = fd;
        hname[fd] = argv[i];
        ++hcount;
        FD_SET(fd, &afds);
    }
    TCPtecho(&afds, maxfd + 1, ccount, hcount);
    exit(0);
}

int TCPtecho(fd_set* pafds,int  nfds,int  ccount,int  hcount)
{
    fd_set rfds, wfds;   /* read/write fd sets */
    fd_set rcfds, wcfds; /* read/write fd sets (copy) */
    int fd, i;
    for (i = 0; i < BUFSIZE; ++i) /* echo data */
        buf[i] = 'D';
    bcopy((char *)pafds, (char *)&rcfds, sizeof(rcfds));
    bcopy((char *)pafds, (char *)&wcfds, sizeof(wcfds));
    for (fd = 0; fd < nfds; ++fd)
        rc[fd] = wc[fd] = ccount;
    (void)mstime((long *)0); /* set the epoch */

    while (hcount)
    {
        bcopy((char *)&rcfds, (char *)&rfds, sizeof(rfds));
        bcopy((char *)&wcfds, (char *)&wfds, sizeof(wfds));
        if (select(nfds, &rfds, &wfds, (fd_set *)0, (struct timeval *)0) < 0)
            perror("select failed: %s\n");
        for (fd = 0; fd < nfds; ++fd)
        {
            if (FD_ISSET(fd, &rfds))
                if (reader(fd, &rcfds) == 0)
                    hcount--;
            if (FD_ISSET(fd, &wfds))
                writer(fd, &wcfds);
        }
    }
}

int reader(int fd,fd_set* pfdset) 
{
    long now;
    int cc;
    cc = read(fd, buf, sizeof(buf));
    if (cc < 0)
        perror("read: error");
    if (cc == 0)
        perror("read: premature end of file\n");
    rc[fd] -= cc;
    if (rc[fd])
        return 1;

    (void)mstime(&now);
    printf("%s: %d ms\n", hname[fd], now);
    (void)close(fd);
    FD_CLR(fd, pfdset);
    return 0;
}

int writer(int fd,fd_set* pfdset)
{
    int cc;
    cc = write(fd, buf, MIN(sizeof(buf), wc[fd]));
    if (cc < 0)
        perror("read: error");
    wc[fd] -= cc;
    if (wc[fd] == 0)
    {
        (void)shutdown(fd, 1);
        FD_CLR(fd, pfdset);
    }
}

long mstime(long* pms)
{
    static struct timeval epoch;
    struct timeval now;
    if (gettimeofday(&now, (struct timezone *)0))
        perror("gettimeofday: error");
    if (!pms)
    {
        epoch = now;
        return 0;
    }
    *pms = (now.tv_sec - epoch.tv_sec) * 1000;
    *pms += (now.tv_usec - epoch.tv_usec + 500) / 1000;
    return *pms;
}