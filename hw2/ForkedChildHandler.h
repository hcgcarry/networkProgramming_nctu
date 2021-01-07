#ifndef ForkedChild
#define ForkedChild
#include"common.h"
class ForkedChildHandler{  
    public:
    unordered_map<int,unordered_map<int,vector<int>>> globalhchildPidList;
    int totalChildNum;
    void insertChild(int id,int commandNum,int pid);
    void cleanPidOneCommand(int id,int commandNum);
    void printGlobalChildPidList();
    void cleanWhenUserExit(int id);
    void noBlockClean(int id);
};

extern ForkedChildHandler forkedChildHandler;
#endif