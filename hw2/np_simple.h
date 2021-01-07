#include"common.h"
#define QLEN 4
#define bufsize 2024
int passiveTCP(int port,int qlen);
int passivesock(int port ,string protocol, int qlen );
int TCPechod(int fd) ;
void reaper(int status);