#include<string>
#include"serverInfo.h"
using namespace std;


serverInfo::serverInfo(string sessionId,string serverName, string port, string testFile) :sessionId(sessionId), serverName(serverName), port(port), testFile(testFile)
{
}
void serverInfo::printServerInfo(){
    cout << "-----serverInfo-----" << endl;
    cout << "sessionId:" << this->sessionId << endl ;
    cout << "port:" << this->port << endl ;
    cout << " serverName:" <<this->serverName << endl;
    cout << " testFile:" << this->testFile << endl;
    cout << "-----serverInfoEnd------" << endl;
}
bool serverInfo::isValueInfo(){
    if(!this->port.empty() && !this->serverName.empty())return true;
    return false;
}