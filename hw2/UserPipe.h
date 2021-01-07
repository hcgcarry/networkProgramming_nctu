#ifndef userPipeFlag
#define userPipeFlag
#include"common.h"
#include"ForkedChildHandler.h"
typedef struct userPipeInfo{
    int senderId;
    int receiverId;
    int ioIn;
    int ioOut;
    int senderCommandNum;
}UserPipeInfo;

class UserPipe 
{
public:
    //four variable below used to record if current command has receive or send action
    //need to be clean every command
    UserPipeInfo curSendUserPipe;
    UserPipeInfo curReceiveUserPipe;
    string curReceiveCommand;
    string curSendCommand;
    bool curSuccessSendMessage ;
    bool curSuccessReceiveMessage ;
    bool curSendMessage ;
    bool curReceiveMessage ;
    bool curReceiveErrorUserNotFound;
    bool curReceiveErrorPipeNotExist;
    bool curSendMessageErrorUserNotFound;
    bool curSendMessageErrorPipeNotExist;
    int ifErrorNullFd;
    //// start_end_fd_list [sendId,receiveId,stdin,stdout]
    vector<UserPipeInfo> userPipeList;
    UserPipe();
    void curIsReceiveCleanFd(int curId);
    void cleanAfterOneCommand();
    void printInfo();
    void cleanAllFd();
    int numOfuserPipe();
    void curWillReceiveSetting(int receiverId,int senderId,string command);
    void appendPipe(int senderId,int receiverId,string command,int commandNum);
    void checkThenSetStdout(int curId,int &newStdout);
    void checkThenSetStdin(int curId,int &newStdin);
    vector<userPipeInfo>::iterator getUserPipeInfoById(int senderId,int receiverId);
    void deleteWhenUserLogOut(int id);
    void cleanIfErrorReceiveOrSend();

};
extern UserPipe userPipe;
#endif