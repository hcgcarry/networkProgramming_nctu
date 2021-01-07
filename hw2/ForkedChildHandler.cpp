#include"ForkedChildHandler.h"
void ForkedChildHandler::insertChild(int id,int commandNum,int pid){
    globalhchildPidList[id][commandNum].push_back(pid);
    totalChildNum++;
}
void ForkedChildHandler::cleanPidOneCommand(int id,int commandNum){
    for(auto item:globalhchildPidList[id][commandNum]){
        waitpid(item,NULL,0);
        totalChildNum--;
    }
    globalhchildPidList[id].erase(commandNum);
}
void ForkedChildHandler::noBlockClean(int id){
    for(auto line:globalhchildPidList[id]){
        for(auto pid:line.second){
            if( waitpid(pid,NULL,WNOHANG)>0){
                totalChildNum--;
            }
        }
    }
}

void ForkedChildHandler::printGlobalChildPidList(){
    cout << "----------printGlobalChildPidList---will remain-----"<<endl;
    cout << "--totalchildNum " << totalChildNum << endl;
    for(auto User:globalhchildPidList){
        cout << "---User ID:"<< User.first <<"---"<< endl;
        for(auto command:User.second){
                cout << "commandNum:" << command.first<< endl;
            for(auto pid:command.second){
                cout << pid << " ";
            }
            cout << endl;
        }
        cout << endl;
    }
}
void ForkedChildHandler::cleanWhenUserExit(int id){
    auto User = globalhchildPidList[id];
    for(auto command:User){
        for(auto pid:command.second){
            waitpid(pid,NULL,0);
            totalChildNum--;
        }
    }
    globalhchildPidList.erase(id);
}