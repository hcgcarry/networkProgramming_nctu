#ifndef serverInfoTag
#define serverInfoTag
#include<string>
#include"common.h"

class serverInfo
{
public:

  std::string sessionId;
  std::string serverName;
  std::string port;
  std::string testFile;

  serverInfo() = delete;
  serverInfo(string sessionId,string serverName, string port, string testFile) ;
  bool isValueInfo();
  void printServerInfo();
};
#endif