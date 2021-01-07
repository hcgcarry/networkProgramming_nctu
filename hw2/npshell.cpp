#include <bits/stdc++.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include<stdlib.h>
#include <fcntl.h>
#include <pwd.h>
#include <unistd.h>
#include<string>
using namespace std;
//#define debug 

class env
{

public:
    char** envp;
    vector<string> envp_name_list;
    env(){};
    void initEnv(char** envp){
        this->envp = envp;
        //printEnvp();
        for(int i=0;envp[i];i++){
            string tmp(envp[i]);
            stringstream ss(tmp);
            getline(ss,tmp,'=');
            envp_name_list.push_back(tmp);
        }
        setEnv("PATH","bin:.");
    }

    void printEnv(string name){
        char* tmp = getenv(name.c_str());
        if(tmp){
            string result(tmp);
            cout << result << endl;
        }
    }
    void setEnv(string name,string value){
        if(setenv(name.c_str(),value.c_str(),1) <0){
            perror("setenv error");
        }
        bool hasInEnvp_name_list  = false;
        for(int i=0;i<envp_name_list.size();i++){
            if(envp_name_list[i] == name){
                hasInEnvp_name_list = true;
            }
        }
        if(!hasInEnvp_name_list){
            char ** result = (char**) malloc(sizeof(char*)*(envp_name_list.size()+4));
            for(int i=0;envp[i];i++){
                result[i] = envp[i];
            }
            string envKeyValue = name+"="+value;
            char* newEnvar = (char*)malloc((envKeyValue.size()+1)*sizeof(char));
            strcpy(newEnvar,envKeyValue.c_str());
            envp[envp_name_list.size()] = newEnvar;
            envp[envp_name_list.size()+1] = NULL;
            envp_name_list.push_back(name);
            this->envp = envp;
        }
    }
    string getEnv(string name){
        char* tmp = getenv(name.c_str());
        string result(tmp);
        return result;
    }
    /*
    char** getEnvp(){
        int count=0;
        for(int i=0;envp[i];i++){
            count++;
        }
        char** result = 
    }
    */
    void printEnvp(){
        for(int i=0;envp[i];i++){
            cout << envp[i] <<endl;
        }

    }

};

class execlamJumpPipe
{
public:
    int curCommandNum;
    //// start_end_fd_list [start,end,stdin,stdout,stderr,stdoutPipeORstdoutStderrPipe]
    vector<vector<int>> start_end_fd_list;
    execlamJumpPipe() {}
    void curIsEndCommandAndcleanFd(){
        // remove first and close Fd
        for(auto iter = start_end_fd_list.begin();iter != start_end_fd_list.end();){
            if((*iter)[1]==curCommandNum){
                close((*iter)[2]);
                close((*iter)[3]);
                start_end_fd_list.erase(iter);
            }
            else{
                iter++;
            }

        }
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
        cout << "---ExeclamJumpPipe_stdio_fd-- "<< endl;
        cout << "curCommandNum" << curCommandNum<< endl;
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
    redirectFile(){};
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
            close(beforeAndCurPipe[0]);
        }
        if(beforeAndCurPipe[1]!=1){
            close(beforeAndCurPipe[1]);
        }
    }
    void cleanCurAndAfterPipe(){
        if(curAndFollowPipe[0]!=0){
            close(curAndFollowPipe[0]);
        }
        if(curAndFollowPipe[1]!=1){
            close(curAndFollowPipe[1]);
        }
    }

};

class parseCommand
{
    public:
    vector<vector<string>> command_list;
    execlamJumpPipe curExePipe = execlamJumpPipe();
    vector<int*> childStdioList;
    //jumpPipe curJumpPipe = jumpPipe();
    Pipe curPipe = Pipe();
    //record
    int curCommandNum = 0;
    //flag
    redirectFile curRedirectFile  = redirectFile();
    /////////////function
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
    parseCommand(){};

    void parse(string command)
    {
        string curString;
        stringstream ss(command);
        vector<string> localCommandList;
        while (ss >> curString)
        {
            if (curString == ">")
            {
                command_list.push_back(localCommandList);
                curRedirectFile.hasRedirectFileFlag= true;
                ss >> curRedirectFile.fileName;
                break;
            }
            else if (curString[0] == '!')
            {
                int jumpnum = stoi(curString.substr(1));
                command_list.push_back(localCommandList);
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
                    command_list.push_back(localCommandList);
                    localCommandList.clear();
                    curExePipe.appendStdoutPipe(jumpnum);
                }
            }
            else
            {
                localCommandList.push_back(curString);
            }
        }
        if (!curExePipe.whetherCurCommandStratExePipe() && !curRedirectFile.hasRedirectFileFlag)
        {
            command_list.push_back(localCommandList);
        }
    }

    int childNum()
    {
        return command_list.size();
    }

};

class shell
{
    //NPShell shellNPShell = NPShell();
    env shellEnv =env();
    parseCommand curParseCommand = parseCommand();
    string inputBuffer;
    int numChildRunning = 0;

public:
    string home_dir(){
        return getpwuid(getuid())->pw_dir;
    }
    shell(char** envp){
        shellEnv.initEnv(envp);
    }
    void commandHandler(string command)
    {
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
        #ifdef debug
        //curParseCommand.printChildStdioList();
        #endif
        //vector<int> unknownCommandIndex= findAllUnknownCommandIndex(curParseCommand.command_list);
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
            /// 
            #ifdef debug
            curParseCommand.curPipe.printCurAndFollowPipe();
            #endif

            /// consider unknowcommand ,we need to close stdio stderr of child before unknown 
            /*
            if(find(unknownCommandIndex.begin(),unknownCommandIndex.end(),i+1) != unknownCommandIndex.end()){
                curChildStdioStderr[1] = curChildStdioStderr[2] = -1;
            }
            */
            ///// fork process

            string filePath="";
            findExecutableFilePath(curParseCommand.command_list[i][0],filePath);
            int tmp = handForkProcess(filePath,curParseCommand.command_list[i],
                                            curChildStdioStderr[0],curChildStdioStderr[1],curChildStdioStderr[2]);

            lastChildPid = tmp > 0 ? tmp:lastChildPid;
            curParseCommand.cleanFd();
            if(i)curParseCommand.curPipe.cleanBeforeAndCurPipeFd();
            curParseCommand.curPipe.updateBeforeAndCurPipe();
            //cout << i  << "i" <<" lastchildpid" << lastChildPid << endl;
            waitChild(-1,false);
            if (i == curParseCommand.childNum() - 1)
            {
                curParseCommand.curExePipe.curIsEndCommandAndcleanFd();
                
                //curParseCommand.curPipe.clean();
                //waitpid(lastChildPid,NULL,0);
                waitChild(lastChildPid,true);
            }
            
        }
    }
    void waitChild(int lastChildPid,bool isLastChild)
    {
        if (numChildRunning == 500)
        {
            while(numChildRunning>200+curParseCommand.curExePipe.numOfExePipe()){
                wait(NULL);
                numChildRunning--;
            }
        }
        if(isLastChild) {
           if(curParseCommand.curExePipe.whetherCurCommandStratExePipe()){
           }
           else if(curParseCommand.curExePipe.numOfExePipe()){
                waitpid(lastChildPid,NULL,0);
           }
           else {
                waitpid(lastChildPid,NULL,0);
                while(numChildRunning){
                    wait(NULL);
                    numChildRunning--;
                }
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
        return false;
    }
    void cleanAfterOnecommand(){
        inputBuffer.clear();
        #ifdef debug
        cout << "#####remaining  child running :" << numChildRunning << endl;
        #endif
        //numChildRunning = 0;
        curParseCommand.cleanAfterOneCommand();
    }
    void getInputChar()
    {
        char curInput;
        cin.get(curInput);
        if(cin.eof()){
            exit(0);

        } 
        if (curInput == '\n')
        {
            string logfilename = home_dir();
            logfilename += "/.npshell_history";
            //cout << "logfilename" << logfilename << endl;
            int fd= open(logfilename.c_str(), O_CREAT | O_WRONLY | O_APPEND,S_IRUSR|S_IWUSR);
            string tmpbuffer = inputBuffer+"\n";
            write(fd,tmpbuffer.c_str(),tmpbuffer.size()+1);
            close(fd);
            if(fd<0)perror("open file error:");

            commandHandler(inputBuffer);
            cleanAfterOnecommand();
            cout << "% ";
        }
        else
        {
            inputBuffer.push_back(curInput);
        }
    }

    void loopGetInputChar()
    {
        while (1)
        {
            getInputChar();
        }
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
        else if (buildInFucnName == "showenvp")
        {
            shellEnv.printEnvp();
            return true;
        }
        return false;
    }

    //check whether it is executable file
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
    pid_t handForkProcess(string filePath,vector<string> &commandAndParam, int newStdin, int newStdout, int newStderr)
    {
        #ifdef debug
        cout << "######filePath " << filePath << " newStdin " << newStdin << " newstdout " << newStdout << " newstderr " << newStderr << endl;
        #endif
        int childPid = -1;
        string command = commandAndParam[0];
        bool hasExeFlag = false;
        // create envp;
        char** envp = shellEnv.envp;
        /*
        for(int i=0;envp[i];i++){
            cout << envp[i] <<endl;
        }
        */
        //create parameter
        int j=0;
        char **argv = (char **)malloc(sizeof(char *) * (commandAndParam.size()+1));
        for (j = 0; j < commandAndParam.size(); j++)
        {
            argv[j] = (char *)commandAndParam[j].c_str();
        }
        argv[j] = NULL;
        // cout parameter
        /*cout << "commandAndParam" << endl;
        for (int i = 0; i < commandAndParam.size(); i++)
        {
            cout << commandAndParam[i] << " ";
        }
        cout << endl;
        */

        //char* argv[]={"Test", "Test_1", (char*)0};
        //char* envp[]={"Test", "Test_1", (char*)0};
        //////////fork
        if ((childPid = fork()) <0)
        {
            perror("can't fork");
            exit(1);
        }
        //child
        else if (childPid == 0)
        {
            //cout << "fork child_pid" << getpid() << endl;
            if (newStdin != 0)
            {
                close(0);
                dup(newStdin);
            }
            if (newStdout == -1) {
                close(1);
            }
            else if(newStdout != 1){
                close(1);
                dup(newStdout);
            }
            if (newStderr == -1) {
                close(2);
            }
            else if(newStderr != 2){
                close(2);
                dup(newStderr);
            }
            curParseCommand.curExePipe.cleanAllFd();
            curParseCommand.curPipe.cleanBeforeAndCurPipeFd();
            curParseCommand.curPipe.cleanCurAndAfterPipe();
            if(filePath =="") {
                cerr << "Unknown command: [" << commandAndParam[0] << "]." << endl;
                exit(0);
            }
            if(execve(filePath.c_str(), argv, envp) <0){
                perror("!!!!!!!!!!!!execve error!!!!!!!!!!!!!:");
            }
        }
        //parent
        else
        {
            numChildRunning++;
        }
        
        return childPid;
    }

    bool is_file_exist(string fileName)
    {
        std::ifstream infile(fileName);
        return infile.good();
    }
};

int npshell(int argc, char *argv[], char *envp[])
{
    /*
    for(int i=0;i<5;i++){
        cout << envp[i] << endl;
    }
    for(int i=0;envp[i];i++){
        cout << envp[i] << endl;
    }
    */
    
    shell curshell = shell(envp);
    cout << "% ";
    curshell.loopGetInputChar();

    return 0;
}
