#include"common.h"
#include <map>
#include <regex>
class env{
  public:
  map<string,string> envMap;
  vector<string>envName={
    "REQUEST_METHOD",
    "REQUEST_URI",
    "QUERY_STRING",
    "HTTP_HOST",
    "SERVER_PROTOCOL",
    "SERVER_ADDR",
    "SERVER_PORT",
    "REMOTE_ADDR",
    "REMOTE_PORT"
    };
  env(){}
  void parseAndsetEnvMapVariable(string requestHeader,string clientIP,string clientPort){

    smatch sm;
    regex reg("GET \\/(.*) (.*)");
    regex_search(requestHeader,sm,reg);
    string request_uri = sm.str(1);
    stringstream ss(request_uri);
    string server_protocol = sm.str(2);
    string cgiName;
    getline(ss,cgiName,'?');
    string query_string ="";
    getline(ss,query_string);

    //getline(ss);;
    //envMap["QUERY_STRING"] = 
    /*
    for(auto item:sm){
      cout << item << endl;
    }
    */
    smatch sm2;
    regex reg2("Host: (.*):([0-9]*)");
    regex_search(requestHeader,sm2,reg2);
    string port = sm2[2];
    string host = sm2[1];
    envMap["REQUEST_METHOD"] = "GET";
    envMap["REQUEST_URI"] = "/"+request_uri;
    envMap["QUERY_STRING"] = query_string;
    envMap["HTTP_HOST"] = host ;
    envMap["SERVER_PROTOCOL"] = server_protocol;
    envMap["SERVER_ADDR"] = host;
    envMap["SERVER_PORT"] = port;
    envMap["REMOTE_ADDR"] = clientIP;
    envMap["REMOTE_PORT"] = clientPort;
    printEnvMap();
  }
  void printEnvMap(){
    cout << "---- envMap-----" << endl;
    for(auto item:envMap){
      cout << "envName:" << item.first << " envValue " << item.second << endl;
    }
    cout << "---- envMap End-----" << endl;
  }
} ;