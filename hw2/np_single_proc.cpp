#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include<algorithm>
#define QLEN 5 /* maximum connection queue length */
#define BUFSIZE 4096
extern int errno;
#include"passivesock.h"
#include"npshell.h"
#include"User.h"
#include"setting.h"
#include"npshell_single_proc.cpp"
#define MAX_SOCKET_NUM 32
#define MAX_USER_NUM 30
#define MAX_COMMAND_SIZE   2024
#define OPEN_FILE_LIMIT 1024
#include"UserPipe.h"
int echo(int fd);

/*------------------------------------------------------------------------
* main - Concurrent TCP server for ECHO service
*------------------------------------------------------------------------*/

using namespace std;
UserPipe userPipe;
User userInfo;
ForkedChildHandler forkedChildHandler;

class UserConnection{
    public:
    vector<shell> shellList;
    void createNewUser(User& userInfo,string ipString,int port,int fd,char** envp){
        userInfo.addUser(ipString,port,fd);
        shellList.push_back(shell(envp,fd));
        startUpMessage(fd);
        broadCastUserLogin(fd);
        write(fd,"% ",2);
    }
    int handleCommand(int fd){
        userInfo.printUserInfoList();
        printShellList();
        char buf[MAX_COMMAND_SIZE];
        int readNum =  read(fd,buf,sizeof(buf));
        if(readNum ==0 ) return 0;
        if(readNum<0){
            return -1;
        }
        
        else{
            if(readNum >=2){
                if(buf[readNum-2] == '\r')buf[readNum-2] = '\0';
            }
            buf[readNum-1] = '\0';
            cout << "***cur command :" << buf << endl;
            string command(buf);
            if(command == "exit"){
                return 0;
            }
            auto iter = findShellByFd(fd);
            iter->handleOneCommand(command);
            userPipe.printInfo();
        }
        return readNum;
    }
    int broadCastUserLogin(int fd){
        UserInfo userinfo ;
        userInfo.getUserInfoByFd(fd,userinfo);
        string message = "*** User '"+userinfo.nickname+"' entered from "+userinfo.ip+":"+to_string(userinfo.port)+". ***\n";
        userInfo.broadCast(message);
    }
    int startUpMessage(int fd)
    {
        string startUpMessage = "****************************************\n** Welcome to the information server. **\n****************************************\n";
        if (write(fd,startUpMessage.c_str() ,startUpMessage.size() ) < 0)
            perror("echo write error");
    }
    void printShellList(){
        cout << "----shellList total num------" << shellList.size() << endl;
        for(int i=0;i<shellList.size();i++){
            cout << "index " << i  ;
            shellList[i].printShell();
        }

    }
    vector<shell>::iterator findShellByFd(int fd){
        for(auto iter=shellList.begin();iter!=shellList.end();iter++){
            if(iter->fd == fd){
                return iter;
            }
        }
    }
    void deleteShellByFd(int fd){
        auto iter = findShellByFd(fd);
        shellList.erase(iter);
    }
    void userLogOut(int fd){
        cout << "user fd:" << fd << "log out" << endl;
        UserInfo info ;
        userInfo.getUserInfoByFd(fd,info);
        userPipe.deleteWhenUserLogOut(info.id);
        deleteShellByFd(fd);
        forkedChildHandler.cleanWhenUserExit(info.id);
        string message = "*** User '"+info.nickname+"' left. ***\n";
        userInfo.broadCast(message);
        userInfo.deleteUser(fd);
        (void)close(fd);
    }
};
UserConnection userConnection;

int main(int argc, char *argv[],char* envp[]) {
    int port ;
    struct sockaddr_in fsin; /* the from address of a client*/
    int msock;               /* master server socket */
    fd_set rfds;             /* read file descriptor set */
    fd_set afds;             /* active file descriptor set */
    int alen;                /* from-address length */
    int fd, nfds;

    switch (argc)
    {
    case 1:
        break;
    case 2:
        port = atoi(argv[1]);
        break;
    default:
        printf("usage: TCPmechod [port]\n");
    }

    msock = passiveTCP(port, QLEN);
    //nfds = getdtablesize();
    //注意 因為select的第一個參數需要的是所有的檔案描述福最大值加一
    nfds = msock+1;
    FD_ZERO(&afds);
    FD_SET(msock, &afds);
    cout << "listen on port " << port << endl;
    while (1)
    {
        cout << "!!!!!!!!!!!!!!!!!new iteration!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<< endl;
        memcpy(&rfds, &afds, sizeof(rfds));
        if (select(nfds, &rfds, (fd_set *)0, (fd_set *)0, (struct timeval *)0) < 0){
            //perror("select error");
            continue;
        }

        // msock , continuely create ssock while someone connect
        if (FD_ISSET(msock, &rfds))
        {
            cout << "---create new socket" << endl;
            int ssock;
            alen = sizeof(fsin);
            ssock = accept(msock, (struct sockaddr *)&fsin,(socklen_t*) &alen);
            if (ssock < 0)
                perror("accept error");
            
            nfds = max(nfds,ssock)+1;
            //get user ip addr
            struct sockaddr_in* pV4Addr = (struct sockaddr_in*)&fsin;
            struct in_addr ipAddr = pV4Addr->sin_addr;
            char str[INET_ADDRSTRLEN];
            inet_ntop( AF_INET, &ipAddr, str, INET_ADDRSTRLEN );
            string ipString(str);
            //userInfo.addUser(ipString,ntohs(pV4Addr->sin_port) ,ssocks);
            int port=ntohs(pV4Addr->sin_port);

            userConnection.createNewUser(userInfo,ipString,port,ssock,envp);
            FD_SET(ssock, &afds);
        }
        for (fd = 0; fd <= nfds; ++fd)
            if (fd != msock && FD_ISSET(fd, &rfds))
                // receive == 0 ,user exit
                if (userConnection.handleCommand(fd) == 0)
                {
                    userConnection.userLogOut(fd);
                    FD_CLR(fd, &afds);
                }
    }
}

