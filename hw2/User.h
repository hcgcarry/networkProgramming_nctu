
#include"common.h"
#ifndef  UserHeader
#define UserHeader

typedef struct userInfo{
    int id;
    string ip;
    int port;
    int fd;
    string nickname;
}UserInfo;

class User{
    vector<UserInfo> userInfoList;
    int maxUserNum ;
    public:
    User();
    void addUser(string ip,int port,int fd);
    void deleteUser(int fd);
    bool getUserInfoByFd(int fd,UserInfo& userinfo);
    void printUserInfoList();
    void sortUserInfoListById();
    void who(int myfd);
    void name(int myfd,string newName);
    void broadCast(string message);
    void tell(int senderFd,int receiverFd,string message);
    bool getUserInfoById(int Id,UserInfo& userinfo);
    vector<UserInfo>::iterator getUserInfoIterByFd(int fd);
};

extern User userInfo;

#endif