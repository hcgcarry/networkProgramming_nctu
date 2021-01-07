
#include "UserPipe.h"
#include "User.h"
//four variable below used to record if current command has receive or send action
//need to be clean every command
UserPipe::UserPipe()
{
    cleanAfterOneCommand();
}
//// start_end_fd_list [sendId,receiveId,stdin,stdout]
void UserPipe::curIsReceiveCleanFd(int curId)
{
    // remove first and close Fd
    if (curSuccessReceiveMessage== true)
    {
        auto iter = getUserPipeInfoById(curReceiveUserPipe.senderId,curId);
        close(iter->ioIn);
        close(iter->ioOut);
        userPipeList.erase(iter);
    }
}

void UserPipe::deleteWhenUserLogOut(int id)
{
    auto iter = userPipeList.begin();
    for (; iter != userPipeList.end();)
    {
        if (iter->receiverId == id || iter->senderId == id)
        {
            close(iter->ioIn);
            close(iter->ioOut);
            if (iter->receiverId == id)
            {
                forkedChildHandler.cleanPidOneCommand(iter->senderId, iter->senderCommandNum);
            }
            userPipeList.erase(iter);
        }
        else
        {
            iter++;
        }
    }
}
void UserPipe::cleanAfterOneCommand()
{
    curSuccessReceiveMessage = false;
    curSuccessSendMessage = false;
    curReceiveCommand = "";
    curSendCommand = "";
    curSendUserPipe ={};
    curReceiveUserPipe = {};
    curSendMessage = false;
    curReceiveMessage = false;
    curSendMessageErrorPipeNotExist = false;
    curSendMessageErrorUserNotFound = false;
    curReceiveErrorPipeNotExist = false;
    curReceiveErrorUserNotFound = false;
    ifErrorNullFd = -1;
    cleanIfErrorReceiveOrSend();
}
void UserPipe::cleanIfErrorReceiveOrSend()
{
    if (curReceiveErrorUserNotFound || curSendMessageErrorPipeNotExist ||
        curReceiveErrorPipeNotExist || curSendMessageErrorUserNotFound)
    {
        close(ifErrorNullFd);
    }
    ifErrorNullFd = -1;
}
void UserPipe::printInfo()
{
    cout << "-----------userPipeList--------will remain---" << endl;
    for (auto item : userPipeList)
    {
        cout << "senderId " << item.senderId << " receiverId " << item.receiverId
             << " ioIn " << item.ioIn << " ioOut " << item.ioOut << endl;
    }
    cout << "-----------userPipeListEnd-----------" << endl;
}
//for forked child
void UserPipe::cleanAllFd()
{
    for (auto item : userPipeList)
    {
        close(item.ioIn);
        close(item.ioOut);
    }
    cleanIfErrorReceiveOrSend();
}
int UserPipe::numOfuserPipe()
{
    return userPipeList.size();
}
void UserPipe::curWillReceiveSetting(int receiverId, int senderId, string command)
{
    curReceiveUserPipe.receiverId = receiverId;
    curReceiveUserPipe.senderId = senderId;
    curReceiveCommand= command;
    curReceiveMessage = true;
}

void UserPipe::appendPipe(int senderId, int receiverId, string command, int commandNum)
{
    //int fd[2];
    //pipe(fd);
    //item.ioIn = fd[0];
    //item.ioOut = fd[1];
    UserPipeInfo item;
    curSendCommand= command;
    item.receiverId = receiverId;
    item.senderId = senderId;
    item.senderCommandNum = commandNum;
    curSendUserPipe = item;
    curSendMessage = true;
}
void UserPipe::checkThenSetStdout(int curId, int &newStdout)
{
    if (curSendMessage)
    {

        UserInfo senderInfo;
        UserInfo receiverInfo;
        int senderId = curSendUserPipe.senderId;
        int receiverId = curSendUserPipe.receiverId;
        userInfo.getUserInfoById(senderId, senderInfo);
        if (!userInfo.getUserInfoById(receiverId, receiverInfo))
        {
            newStdout = open("/dev/null", O_WRONLY);
            ifErrorNullFd = newStdout;
            if (newStdout < 0)
            {
                perror("open /dev/null error");
            }
            newStdout = ifErrorNullFd;
            string message = "*** Error: user #" + to_string(receiverId) + " does not exist yet. ***\n";
            write(senderInfo.fd, message.c_str(), message.size());
            curSendMessageErrorUserNotFound = true;
        }
        else if (getUserPipeInfoById(senderId, receiverId) != userPipeList.end())
        {
            newStdout = open("/dev/null", O_WRONLY);
            ifErrorNullFd = newStdout;
            if (newStdout < 0)
            {
                perror("open /dev/null error");
            }
            newStdout = ifErrorNullFd;
            string message = "*** Error: the pipe #" + to_string(senderId) + "->#" + to_string(receiverId) + " already exists. ***\n";
            write(senderInfo.fd, message.c_str(), message.size());
            curSendMessageErrorPipeNotExist = true;
        }
        else
       {
            curSuccessSendMessage = true;
            int fd[2];
            pipe(fd);
            curSendUserPipe.ioIn = fd[0];
            curSendUserPipe.ioOut = fd[1];
            userPipeList.push_back(curSendUserPipe);

            newStdout = fd[1];
            string message = "*** " + senderInfo.nickname + " (#" + to_string(senderInfo.id) + ") just piped '" + curSendCommand + "' to " + receiverInfo.nickname + " (#" + to_string(receiverInfo.id) + ") ***\n";
            userInfo.broadCast(message);
        }
    }
}
void UserPipe::checkThenSetStdin(int curId, int &newStdin)
{
    if (curReceiveMessage)
    {
        UserInfo senderInfo;
        string message;
        UserInfo receiverInfo;
        int receiverId = curReceiveUserPipe.receiverId;
        int senderId = curReceiveUserPipe.senderId;
        userInfo.getUserInfoById(receiverId, receiverInfo);
        if (!userInfo.getUserInfoById(senderId, senderInfo))
        {
            newStdin = open("/dev/null", O_RDONLY);
            ifErrorNullFd = newStdin;
            if (newStdin < 0)
            {
                perror("open /dev/null error");
            }
            newStdin = ifErrorNullFd;
            message = "*** Error: user #" + to_string(senderId) + " does not exist yet. ***\n";
            write(receiverInfo.fd, message.c_str(), message.size());
            curReceiveErrorUserNotFound = true;
        }
        else if (getUserPipeInfoById(senderId, receiverId) == userPipeList.end())
        {
            newStdin = open("/dev/null", O_RDONLY);
            ifErrorNullFd = newStdin;
            if (newStdin < 0)
            {
                perror("open /dev/null error");
            }
            newStdin = ifErrorNullFd;
            message = "*** Error: the pipe #" + to_string(senderId) + "->#" + to_string(receiverId) + " does not exist yet. ***\n";
            write(receiverInfo.fd, message.c_str(), message.size());
            curReceiveErrorPipeNotExist = true;
        }
        else
        {
            curSuccessReceiveMessage = true;
            auto curUserPipeInfo = getUserPipeInfoById(senderId, receiverId);
            curReceiveUserPipe.senderCommandNum = curUserPipeInfo->senderCommandNum;
            newStdin = curUserPipeInfo->ioIn;
            message = "*** " + receiverInfo.nickname + " (#" \
            + to_string(receiverInfo.id) + ") just received from " \
            + senderInfo.nickname + " (#" + to_string(senderInfo.id) + ") by '" \
            + curReceiveCommand+ "' ***\n";
            userInfo.broadCast(message);
        }
    }
}
vector<userPipeInfo>::iterator UserPipe::getUserPipeInfoById(int senderId, int receiverId)
{
    auto iter = userPipeList.begin();
    for (; iter != userPipeList.end(); iter++)
    {
        if ((*iter).receiverId == receiverId && (*iter).senderId == senderId)
        {
            return iter;
        }
    }
    return iter;
}
