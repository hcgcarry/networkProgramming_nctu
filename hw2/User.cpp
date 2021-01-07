#include "User.h"

User::User(){
    maxUserNum = MAX_USER_NUM;
} 

void User::printUserInfoList(){
    cout << "-----------userInfoList -----------" << endl;
    for(auto& item:userInfoList){
        
        cout << "id " << item.id << " ip " << item.ip << " port " << item.port \
        << " fd " << item.fd << " nickname " << item.nickname << " shellListIndex" << endl;
    }
    cout << "---------userInfoListEnd-----------" << endl;
}
void User::tell(int receiverId,int senderId,string message){
    UserInfo receiverInfo;
    UserInfo senderInfo;
    userInfo.getUserInfoById(senderId,senderInfo);
    if(!userInfo.getUserInfoById(receiverId,receiverInfo)){
        message = "*** Error: user #"+to_string(receiverId)+" does not exist yet. ***\n";
        write(senderInfo.fd,message.c_str(),message.size());
        return ;
    }
    message=  "*** "+ senderInfo.nickname+" told you ***: " + message;
    write(receiverInfo.fd,message.c_str(),message.size());
}

void User::addUser(string ip, int port, int fd)
{
    //select min id
    //sortUserInfoList();
    UserInfo newInfo;
    for (int i = 1; i <= maxUserNum; i++)
    {
        bool hasExist = false;
        for (int j = 0; j < userInfoList.size(); j++)
        {
            if (userInfoList[j].id == i)
            {
                hasExist = true;
            }
        }
        if (!hasExist)
        {
            newInfo.id = i;
            break;
        }
    }
    newInfo.fd =fd;
    newInfo.ip = ip;
    newInfo.port  = port;
    newInfo.nickname = "(no name)";
    userInfoList.push_back(newInfo);
}
vector<UserInfo>::iterator User::getUserInfoIterByFd(int fd){
    auto iter = userInfoList.begin();
    for(;iter!=userInfoList.end();iter++){
        if(iter->fd == fd){
            return iter;
        }
    }
    return iter;
}

bool User::getUserInfoByFd(int fd,UserInfo& userinfo){
    auto iter = getUserInfoIterByFd(fd);
    if(iter == userInfoList.end()){
        cout << "userInfo not found ,fd:"<< fd << endl;
        return false;
    }
    else{
        userinfo = (*iter);
        return true;
    }
}
bool User::getUserInfoById(int id,UserInfo& userinfo){
    int fd=-1;
    for(auto item:userInfoList){
        if(item.id == id) fd = item.fd;
    }
    if(fd == -1){
        cout << "getUserInfoById not found ,id:" << id << endl;
        return false;
    }
    auto iter = getUserInfoIterByFd(fd);
    userinfo = (*iter);
    return true;
}
void User::deleteUser(int fd)
{
    auto iter = getUserInfoIterByFd(fd);
    if(iter != userInfoList.end()){
        userInfoList.erase(iter);
    }
    else{
        cout << "deleteUser not found" << endl;
    }
}

void User::who(int myfd){
    sortUserInfoListById();
    string str =  "<ID>\t<nickname>\t<IP:port>\t<indicate me>\n" ;
    for(auto& item:userInfoList){
        str+= to_string(item.id) +"\t" +item.nickname+ "\t" + item.ip +":"+ to_string(item.port) \
        +"\t" ;
        if(myfd == item.fd) str+= "<-me" ;
        str+= '\n';
    }
    write(myfd,str.c_str(),str.size());

}
void User::name(int myfd,string newName){
    //check if repeated
    for(auto item:userInfoList){
        if(item.nickname == newName && item.fd != myfd){
            string message =  "*** User '"+newName+"' already exists. ***\n";
            write(myfd,message.c_str(),message.size());
            return ;
        }
    }
    auto iter = getUserInfoIterByFd(myfd);
    iter->nickname = newName;
    string message = "*** User from "+iter->ip+":"+to_string(iter->port)+" is named '"+newName+"'. ***\n";
    this->broadCast(message);
}
void User::sortUserInfoListById(){
    sort(this->userInfoList.begin(),this->userInfoList.end(),\
    [](UserInfo& n1,UserInfo& n2){return n1.id < n2.id;});
}

void User::broadCast(string message){
    for(auto &item:userInfoList){
        write(item.fd,message.c_str(),message.size());
    }
}


/*
    void sortUserInfoList(){
        sort(userInfoList.begin(),userInfoList.end(),sortUserInfoById);
    }
    */

/*
int sortUserInfoById(UserInfo info1,UserInfo info2){
    if(info2.id > info1.id)return 1;
    return 0;
}
*/