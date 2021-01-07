#include <fcntl.h>
#include <bits/stdc++.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include<stdlib.h>
#include <pwd.h>
#include <unistd.h>
#include<string>
using namespace std;
#include"User.h"
#define debug 
#include"UserPipe.h"
#include"ForkedChildHandler.h"

class env
{
    int fd;

public:
    map<string, vector<string>> env_list;

    env(){}
    vector<string> getEnvVar(string envName)
    {
        return env_list[envName];
    }
    int envCount()
    {
        return env_list.size();
    }
    /*
    env(char** envp,int fd)
    {
        this->fd = fd;
        for(int i=0;envp[i];i++){
            string curEnv(envp[i]);
            stringstream ss(curEnv);
            string key,value;
            getline(ss,key,'=');
            getline(ss,value);
            setEnv(key,value);
        }
        setEnv("PATH","bin:.");
    }
    */
    env(int fd)
    {
        this->fd = fd;
        setEnv("PATH","bin:.");
    }
    void printAllEnv(){
        cout << "----------all Env ------------" << endl;
        for(auto& item:env_list){
            cout << item.first << " " << getEnv(item.first) << endl;
        }
    }

    void printEnv(string envName)
    {
        string env_str = "";
        if(env_list.find(envName) == env_list.end()) return ;
        for (int i = 0; i < env_list[envName].size(); i++)
        {
            if (i)
                env_str += ":" + env_list[envName][i];
            else
                env_str = env_list[envName][i];
        }
        //cout << env_str << endl;
        env_str+='\n';
        write(fd,env_str.c_str(),env_str.size());
    }
    string getEnv(string envName){
        string env_str = "";
        if(env_list.find(envName) == env_list.end()) return "";
        for (int i = 0; i < env_list[envName].size(); i++)
        {
            if (i)
                env_str += ":" + env_list[envName][i];
            else
                env_str = env_list[envName][i];
        }
        return env_str;
    }
    void setEnv(string key, string value)
    {
        stringstream ss(value);
        string tmp;
        env_list[key].clear();
        while (getline(ss, tmp, ':'))
        {
            env_list[key].push_back(tmp);
        }
    }
    char** envp(){
        vector<string> tmpResult;
        for(auto enVar:env_list){
            string singleVar ="";
            singleVar+=enVar.first+"=";
            for(int i=0;i<enVar.second.size();i++){
                if(i)singleVar+=":"+enVar.second[i];
                else{
                    singleVar+=enVar.second[i];
                }
            }
            tmpResult.push_back(singleVar);
        }
        char **envp = (char **)malloc(sizeof(char *) * (tmpResult.size()+1));
        int i=0;
        for (i = 0; i < tmpResult.size(); i++)
        {
            envp[i] = (char *)tmpResult[i].c_str();
        }
        envp[i]= NULL;
        return envp;
    }

    
};
class execlamJumpPipe
{
    int fd;
public:
    int curCommandNum;
    //// start_end_fd_list [start,end,stdin,stdout,stderr,stdoutPipeORstdoutStderrPipe]
    vector<vector<int>> start_end_fd_list;

    execlamJumpPipe(){ }
    execlamJumpPipe(int fd) {
        this->fd = fd;
    }
    void curIsEndCommandAndcleanFd(){
        // remove first and close Fd
        for(auto iter = start_end_fd_list.begin();iter != start_end_fd_list.end();iter++){
            if((*iter)[1]==curCommandNum){
                close((*iter)[2]);
                close((*iter)[3]);
                //start_end_fd_list.erase(iter);
            }
        }
    }
    void curIsEndThenErasePipe(){
        // remove first and close Fd
        for(auto iter = start_end_fd_list.begin();iter != start_end_fd_list.end();){
            if((*iter)[1]==curCommandNum){
                start_end_fd_list.erase(iter);
            }
            else{
                iter++;
            }

        }
    }
    vector<int> curIsEndGetStartCommandNum(){
        vector<int> result;
        for(auto item:start_end_fd_list){
            result.push_back(item[0]);
        }
        return result;
        
    }
    void cleanAllFd(){
        for(auto item:start_end_fd_list){
            close(item[2]);
            close(item[3]);
        }
    }
    int numOfExePipe(){
        return start_end_fd_list.size();
    }
    void appendStdoutStderrPipe(int jumpNum)
    {
        int stdio_fd[3] = {-1, -1, -1};
        int endCommandNum= curCommandNum+ jumpNum;
        //if exepipe exist
        auto item = findCommandEndExePipe(endCommandNum);
        if(item.empty()){
            if(pipe(stdio_fd)<0) perror("pipe open error");
            stdio_fd[2] = stdio_fd[1];
        }
        else{
            stdio_fd[1]=stdio_fd[2] = item[3];
            stdio_fd[0] = item[2];
        }
        //
        start_end_fd_list.push_back(vector<int> {curCommandNum,endCommandNum,\
        stdio_fd[0],stdio_fd[1],stdio_fd[1],1});
    }
    void appendStdoutPipe(int jumpNum)
    {
        int stdio_fd[3] = {-1, -1, -1};
        int endCommandNum= curCommandNum+ jumpNum;
        //if exepipe exist
        auto item = findCommandEndExePipe(endCommandNum);
        if(item.empty()){
            pipe(stdio_fd);
            stdio_fd[2] = stdio_fd[1];
        }
        else{
            stdio_fd[1]=stdio_fd[2] = item[3];
            stdio_fd[0] = item[2];
        }
        //
        start_end_fd_list.push_back(vector<int> {curCommandNum,endCommandNum,\
        stdio_fd[0],stdio_fd[1],stdio_fd[1],0});
    }
    void printExeclamJumpPipe(){
        cout << "***curCommandNum" << curCommandNum<< endl;
        cout << "---ExeclamJumpPipe_stdio_fd--- will remain- "<< endl;
        for(auto item:start_end_fd_list){
            for(auto i:item){
                cout << i << " ";
            }
            cout << endl;
        }
        cout << "---ExeclamJumpPipe_stdio_fd_end-- "<< endl;
    }


    void setStdoutAndStderr(int &oldStdout, int &oldStderr)
    {
        auto item = findCommandStratExePipe(curCommandNum);
        if(!item.empty()){
            //whether is !n or |n
            if(item[5] == 0){
                oldStdout =  item[3];
            }
            else{
                oldStdout = oldStderr =  item[3];
            }
        }
    }
    void setStdin(int &oldStdin)
    {
        auto item = findCommandEndExePipe(curCommandNum);
        if(!item.empty()){
            oldStdin =  item[2];
        }
    }
    vector<int> findCommandEndExePipe(int commandNum){
        for(auto item:start_end_fd_list){
            if(item[1] == commandNum){
                return item;
            }
        }
        return vector<int>{};
    }
    vector<int> findCommandStratExePipe(int commandNum){
        for(auto item:start_end_fd_list){
            if(item[0] == commandNum){
                return item;
            }
        }
        return vector<int>{};
    }
    bool whetherCommandEndExePipe(int commandNum){
        if(findCommandEndExePipe(commandNum).empty()){
            return false;
        }
        return true;
    }
    bool whetherCommandStartExePipe(int commandNum){
        if(findCommandStratExePipe(commandNum).empty()){
            return false;
        }
        return true;
    }
    bool whetherCurCommandEndExePipe(){
        return whetherCommandEndExePipe(curCommandNum);
    }
    bool whetherCurCommandStratExePipe(){
        return whetherCommandStartExePipe(curCommandNum);
    }
};
class redirectFile{
    public:
    string fileName;
    bool hasRedirectFileFlag = false;
    int redirectFileFd = -1;
    redirectFile(){
    };
    void openRedirectFileAndSetStdio(vector<int>& curChildStdioStderr){
        if(hasRedirectFileFlag){
            redirectFileFd = open(fileName.c_str(), O_CREAT | O_WRONLY | O_TRUNC,S_IRUSR|S_IWUSR);
            if(redirectFileFd <0)perror("open file error:");
            //cout << "file fd " <<  redirectFileFd  << endl;
            curChildStdioStderr[1] = redirectFileFd;
        }
    }
    void cleanFd(){
        if(hasRedirectFileFlag)close(redirectFileFd);
    }
};


class Pipe
{
public:
    int pipeCount = 0;
    int remainNeedCreatePipeCount=0;
    int beforeAndCurPipe[2] = {0,1};
    int curAndFollowPipe[2] = {0,1};

    Pipe(){};
    void printCurAndFollowPipe(){
        cout << "---------pipeCount-------- " << pipeCount << endl;
        cout << "remainNeedCreatePipeCount" << remainNeedCreatePipeCount<< endl;
        cout << "beforeAndCurPipe " << beforeAndCurPipe[0] << " " << beforeAndCurPipe[1] << endl;
        cout << "curAndFollowPipe " << curAndFollowPipe[0] << " " << curAndFollowPipe[1] << endl;
    }
    void openPipeAndsetStdio(vector<int>& curChildStdioStderr){
        curChildStdioStderr[0] = beforeAndCurPipe[0];
        if(remainNeedCreatePipeCount){
            pipe(curAndFollowPipe);
            cout <<"-----create pipe" <<  curAndFollowPipe[0] << " " << curAndFollowPipe[1] << endl;
            remainNeedCreatePipeCount--;
            curChildStdioStderr[1] = curAndFollowPipe[1];
        }
    }
    void updateBeforeAndCurPipe(){
            //update
            beforeAndCurPipe[0] = curAndFollowPipe[0];
            beforeAndCurPipe[1] = curAndFollowPipe[1];
    }
    void cleanBeforeAndCurPipeFd(){
        if(beforeAndCurPipe[0]!=0){
            //cout << "close  beforeAndCurPipe index 0 fd" << beforeAndCurPipe[0] << endl;
            close(beforeAndCurPipe[0]);
        }
        if(beforeAndCurPipe[1]!=1){
            //cout << "close  beforeAndCurPipe index 1 fd" << beforeAndCurPipe[1] << endl;
            close(beforeAndCurPipe[1]);
        }
    }
    void cleanCurAndAfterPipe(){
        if(curAndFollowPipe[0]!=0){
            //cout << "close  curAndFollowPipe index 0 fd" << curAndFollowPipe[0] << endl;
            close(curAndFollowPipe[0]);
        }
        if(curAndFollowPipe[1]!=1){
            //cout << "close  curAndFollowPipe index 1 fd" << curAndFollowPipe[1] << endl;
            close(curAndFollowPipe[1]);
        }
    }

};

class parseCommand
{
    public:
    int fd;
    int id;
    //data structure
    vector<vector<string>> command_list;
    execlamJumpPipe curExePipe ;
    vector<int*> childStdioList;
    //jumpPipe curJumpPipe = jumpPipe();
    Pipe curPipe = Pipe();
    //record
    int curCommandNum = 0;
    //flag
    redirectFile curRedirectFile;
    /////////////function
    parseCommand(){}
    parseCommand(int fd,int id){
        curExePipe = execlamJumpPipe(fd);
        this->fd = fd;
        this->id = id;
    };
    void initParseCommand(){
        curCommandNum++;
        curExePipe.curCommandNum = curCommandNum;
        curRedirectFile = redirectFile();
        curPipe = Pipe();
    }
    
    void printCommand_list(){
        cout << "---command_list---" << endl;
        for(int i=0;i<command_list.size();i++){
            
            for(auto item :command_list[i]){
                cout << item << " " ;
            }
            cout << endl;
        }
        cout << "---command_list_end---" << endl;
    }
    void printChildStdioList(){
        cout << "---childStdioList---" << endl;
        for(auto item:childStdioList){
            for(int i=0;i<3;i++){
                cout << "i" << i << " " <<  item[i] << " ";
            }
            cout << endl;

        }
        cout << "---childStdioList_end---" << endl;

    }
    void cleanFd(){
        curRedirectFile.cleanFd();
    }
    void cleanAfterOneCommand(){
        //curPipe = Pipe();
        command_list.clear();
        //free childstdiolist
        for(auto item:childStdioList){
            //if(item) free(item);
        }
        childStdioList.clear();
    }

    void parse(string command)
    {
        string curString;
        stringstream ss(command);
        vector<string> localCommandList;
        while (ss >> curString)
        {
            if (curString[0] == '>')
            {
                if(curString.size() ==1){
                    curRedirectFile.hasRedirectFileFlag= true;
                    ss >> curRedirectFile.fileName;
                    break;
                }
                else {
                    int receiverId= stoi(curString.substr(1));
                    userPipe.appendPipe(id,receiverId,command,curCommandNum);
                }
            }
            else if (curString[0] == '<')
            {
                int senderId= stoi(curString.substr(1));
                userPipe.curWillReceiveSetting(id,senderId,command);
            }
            else if (curString[0] == '!')
            {
                int jumpnum = stoi(curString.substr(1));
                curExePipe.appendStdoutStderrPipe(jumpnum);
            }
            else if (curString[0] == '|')
            {
                if (curString.size() == 1)
                {
                    command_list.push_back(localCommandList);
                    localCommandList.clear();
                    curPipe.pipeCount++;
                    curPipe.remainNeedCreatePipeCount = curPipe.pipeCount;
                }
                else
                {
                    int jumpnum = stoi(curString.substr(1));
                    curExePipe.appendStdoutPipe(jumpnum);
                }
            }
            else
            {
                localCommandList.push_back(curString);
            }
        }
        command_list.push_back(localCommandList);
    }

    int childNum()
    {
        return command_list.size();
    }

};

class shell
{
    //NPShell shellNPShell = NPShell();
    env shellEnv ;
    parseCommand curParseCommand ;
    string inputBuffer;
    int numChildRunning = 0;
    string curCommand ;

public:
    int fd ;
    int id;
    shell(char** envp,int fd){
        UserInfo info;
        userInfo.getUserInfoByFd(fd,info);
        this->fd = fd;
        this->id = info.id;
        shellEnv = env(fd);
        curParseCommand= parseCommand(fd,id);
    }
    void commandHandler(string command)
    {
        curCommand = command;
        if (command.empty()) return;

        pid_t lastChildPid;
        curParseCommand.initParseCommand();
        curParseCommand.parse(command);
        //curParseCommand.createProcessStdioFd();
        #ifdef debug
        curParseCommand.curExePipe.printExeclamJumpPipe();
        //curParseCommand.curPipe.printPipeList();
        curParseCommand.printCommand_list();
        #endif
        //////// check whether is build function
        if (curParseCommand.childNum() == 1)
        {
            if (handBuildInFunc(curParseCommand.command_list[0]))
                return;
        }
        ///////// is executable file
        for (int i = 0; i < curParseCommand.childNum(); i++)
        {
            ///////////////set stdio stderr
            /// pipe
            vector<int> curChildStdioStderr = {0,1,2};
           curParseCommand.curPipe.openPipeAndsetStdio(curChildStdioStderr);
            ///exepipe
            if(i==0 ){
                curParseCommand.curExePipe.setStdin(curChildStdioStderr[0]);
            }
            if(i==curParseCommand.childNum()-1){
                curParseCommand.curExePipe.setStdoutAndStderr(curChildStdioStderr[1],curChildStdioStderr[2]);
                /// consider redirect to file
                curParseCommand.curRedirectFile.openRedirectFileAndSetStdio(curChildStdioStderr);
            }
            /// userPipe
            if(i==0 ){
                userPipe.checkThenSetStdin(id,curChildStdioStderr[0]);
            }
            if(i==curParseCommand.childNum()-1){
                userPipe.checkThenSetStdout(id,curChildStdioStderr[1]);
            }
            /// 
            #ifdef debug
            curParseCommand.curPipe.printCurAndFollowPipe();
            #endif

            ///// fork process

            int tmp = handForkProcess(curParseCommand.command_list[i][0],curParseCommand.command_list[i],
                                            curChildStdioStderr[0],curChildStdioStderr[1],curChildStdioStderr[2]);

            //clean fd  and wait
            curParseCommand.cleanFd();
            if(i)curParseCommand.curPipe.cleanBeforeAndCurPipeFd();
            curParseCommand.curPipe.updateBeforeAndCurPipe();
            lastChildPid = tmp > 0 ? tmp:lastChildPid;
            waitChild(-1,false);
            if (i == curParseCommand.childNum() - 1)
            {
                curParseCommand.curExePipe.curIsEndCommandAndcleanFd();
                userPipe.curIsReceiveCleanFd(id);
                userPipe.cleanIfErrorReceiveOrSend();
                waitChild(lastChildPid,true);
            }
            
        }
    }
    void waitChild(int lastChildPid,bool isLastChild)
    {
        if (forkedChildHandler.totalChildNum > 100)
        {
            while(waitpid(-1, NULL, WNOHANG)>0){
                forkedChildHandler.totalChildNum--;
            }
        }
        if(isLastChild) {
           if(curParseCommand.curExePipe.whetherCurCommandEndExePipe()){
               if(!userPipe.curSuccessSendMessage){
                    forkedChildHandler.cleanPidOneCommand(id,curParseCommand.curCommandNum);
                    auto needCleanCommandNumList = curParseCommand.curExePipe.curIsEndGetStartCommandNum();
                    for(auto item:needCleanCommandNumList){
                            forkedChildHandler.cleanPidOneCommand(id,item);
                    }
                    curParseCommand.curExePipe.curIsEndThenErasePipe();
               }
               else{
                    curParseCommand.curExePipe.curIsEndThenErasePipe();
               }
           }
           if(userPipe.curSuccessReceiveMessage){
               forkedChildHandler.cleanPidOneCommand(userPipe.curReceiveUserPipe.senderId,userPipe.curReceiveUserPipe.senderCommandNum);
               //this code is not good,we should wait child of exepipe which direct to userpipe sender explicity
               forkedChildHandler.noBlockClean(userPipe.curReceiveUserPipe.senderId);
           }
           if(userPipe.curSuccessSendMessage|| curParseCommand.curExePipe.whetherCurCommandStratExePipe()){
           }
           else {
               forkedChildHandler.cleanPidOneCommand(id,curParseCommand.curCommandNum);
           }
            while(waitpid(-1, NULL, WNOHANG)>0){
                forkedChildHandler.totalChildNum--;
            }
        }

    }

    vector<int> findAllUnknownCommandIndex(vector<vector<string>>& command_list){
        //because we need to close all stdout fd which before unknown command
        vector<int> unknownCommandIndexList;
        string tmp="";
        for(int i=0;i<command_list.size();i++){
            if(!findExecutableFilePath(command_list[i][0],tmp ) ){
                unknownCommandIndexList.push_back(i);
            }
        }
        return unknownCommandIndexList;
    }
    
    bool isBuildInFunction(string command){
        if (command== "setenv"){
            return true;
        }
        else if (command== "printenv")
        {
            return true;
        }
        else if (command== "exit")
        {
            return true;
        }
        else if(command== "who" || command == "yell" || command=="tell" || command =="name"){
            return true;
        }
        return false;
    }
    void cleanAfterOnecommand(){
        userPipe.cleanAfterOneCommand();
        inputBuffer.clear();
        #ifdef debug
        //cout << "#####remaining  child running :" << numChildRunning << endl;
        forkedChildHandler.printGlobalChildPidList();
        #endif
        //numChildRunning = 0;
        curParseCommand.cleanAfterOneCommand();
    }
    void handleOneCommand(string command)
    {
        cout << "***cur shell fd" << fd << endl;
        commandHandler(command);
        cleanAfterOnecommand();
        //cout << "% ";
        write(fd,"% ",2);
    }

    bool handBuildInFunc(vector<string> &commandAndParam)
    {
        string buildInFucnName = commandAndParam[0];
        if (buildInFucnName == "setenv")
        {
            if(commandAndParam.size()==1)return true;
            else if(commandAndParam.size()==2){
                shellEnv.setEnv(commandAndParam[1],"");
            }
            else{
                shellEnv.setEnv(commandAndParam[1], commandAndParam[2]);
            }
            return true;
        }
        else if (buildInFucnName == "printenv")
        {
            if(commandAndParam.size()==1)return true;
            shellEnv.printEnv(commandAndParam[1]);
            return true;
        }
        else if (buildInFucnName == "exit")
        {
            exit(0);
        }
        else if (buildInFucnName == "who")
        {
            userInfo.who(fd);
            return true;
        }
        else if (buildInFucnName == "name")
        {
            userInfo.name(fd,commandAndParam[1]);
            return true;
        }
        else if (buildInFucnName == "yell")
        {
            auto myInfo = userInfo.getUserInfoIterByFd(fd);
            string message ="*** "+myInfo->nickname+" yelled ***: ";
            stringstream ss(curCommand);
            string tmp  = "yell ";
            message += curCommand.substr(tmp.size());
            message+='\n';
            /*
            for(int i=1;i<commandAndParam.size();i++){
                if(i>1) message+=" ";
                message += commandAndParam[i];
            }
            */
            userInfo.broadCast(message);
            return true;
        }
        else if (buildInFucnName == "tell")
        {

            int receiverId = stoi(commandAndParam[1]);
            stringstream ss(curCommand);
            string tmp;
            ss >> tmp;
            ss >> tmp;
            string message ="";
            ss >> message;
            while(ss >> tmp){
                message += " " + tmp;
            }
            message+='\n';
            userInfo.tell(receiverId,id,message);
            return true;
        }
        return false;
    }

    //check whether it is file
    //don't use this
    bool findExecutableFilePath(string fileName,string& ifExistReturnPath){
        string path = shellEnv.getEnv("PATH");
        stringstream ss(path);
        string pathItem;
        while(getline(ss,pathItem,':')){
            string filePath = pathItem + "/" + fileName;
            if (is_file_exist(filePath))
            {
                ifExistReturnPath = filePath;
                return true;
            }
        }
        return false;

    }
    pid_t handForkProcess(string fileName,vector<string> &commandAndParam, int newStdin, int newStdout, int newStderr)
    {
        #ifdef debug
        cout << "######fileName " << fileName<< endl;
        cout <<" newStdin " << newStdin << " newstdout " << newStdout << " newstderr " << newStderr << endl;
        #endif
        int childPid = -1;
        string command = commandAndParam[0];
        bool hasExeFlag = false;
        //create parameter
        int j=0;
        char **argv = (char **)malloc(sizeof(char *) * (commandAndParam.size()+1));
        for (j = 0; j < commandAndParam.size(); j++)
        {
            argv[j] = (char *)commandAndParam[j].c_str();
        }
        argv[j] = NULL;
        // envp
        // cout parameter
        /*cout << "commandAndParam" << endl;
        for (int i = 0; i < commandAndParam.size(); i++)
        {
            cout << commandAndParam[i] << " ";
        }
        cout << endl;
        */

        //char* argv[]={"Test", "Test_1", (char*)0};
        //char* envp[]={"Test", "Test_1", NULL};
        char** envp = shellEnv.envp();
        /*
        for(int i=0;envp[i];i++){
            cout << envp[i] <<endl;
        }
        */
        //////////fork
        while ((childPid = fork()) <0)
        {
            waitpid(-1,NULL,0);
        }
        //child
        if (childPid == 0)
        {
            close(0);
            close(1);
            close(2);
            dup(fd);
            dup(fd);
            dup(fd);
            //cout << "fork child_pid" << getpid() << endl;
            if (newStdin != 0)
            {
                //cout << "---child close 0 dup" << newStdin << endl;
                close(0);
                dup(newStdin);
            }
            if(newStdout != 1){
                //cout << "---child close 1 dup" << newStdout<< endl;
                close(1);
                dup(newStdout);
            }
            if(newStderr != 2){
                //cout << "---child close 2 dup" << newStderr<< endl;
                close(2);
                dup(newStderr);
            }
            //we should close all unused fd because that if child has fd point to stdin of pipe
            //,and it's self is read from stdout of pipe,pipe won't close since there is fd point stdin of pipe
            //,it assume there still have data to come in
            userPipe.cleanAllFd();
            curParseCommand.curExePipe.cleanAllFd();
            curParseCommand.curPipe.cleanBeforeAndCurPipeFd();
            curParseCommand.curPipe.cleanCurAndAfterPipe();
            // find executable file
            string path = shellEnv.getEnv("PATH");
            stringstream ss(path);
            string pathItem;
            while(getline(ss,pathItem,':')){
                string filePath = pathItem + "/" + fileName;
                if(execve(filePath.c_str(), argv,envp) <0){
                }
            }
            char buf[2024];
            sprintf(buf, "Unknown command: [%s].\n" ,commandAndParam[0].c_str());
            write(fd,buf,strlen(buf));
            exit(0);
        }
        //parent
        else
        {
            forkedChildHandler.insertChild(id,curParseCommand.curCommandNum,childPid);
        }
        
        return childPid;
    }

    bool is_file_exist(string fileName)
    {
        std::ifstream infile(fileName);
        return infile.good();
    }
    void printShell(){
        cout << "fd " << fd << "id" << id<< endl;
        //shellEnv.printAllEnv();
    }
};
