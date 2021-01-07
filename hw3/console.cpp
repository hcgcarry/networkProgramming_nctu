#include "common.h"
#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdio.h>
#include <utility>
#include <boost/units/absolute.hpp>
#include <boost/asio.hpp>
#include "util.h"
#include <map>
#include <regex>
#include "env.cpp"
#include"printHTML.cpp"
#include"clientSession.cpp"
#include"serverInfo.h"
using boost::asio::ip::tcp;
using namespace std;


class client
{
private:
  vector<serverInfo> serverInfo_list;
  boost::asio::io_context cur_io_context;
  boost::asio::ip::tcp::resolver resolveObj;
  env envObj;
  printHTML printHTMLObj;

public:
  client():resolveObj(cur_io_context){
    parseServerInfo();
    #ifdef debug
    printServerInfo_list();
    //envObj.getEnv();
    envObj.printEnvMap();
    #endif
  };
  
  void parseServerInfo()
  {
    envObj.getEnv();
    string query_string = envObj.envMap["QUERY_STRING"];
//string query_string ="h0=nplinux8.cs.nctu.edu.tw&p0=7778&f0=t1.txt&h1=nplinux8.cs.nctu.edu.tw&p1=7776&f1=t2.txt&h2=&p2=&f2=&h3=&p3=&f3=&h4=&p4=&f4=";
//string query_string ="h0=nplinux8.cs.nctu.edu.tw&p0=7778&f0=t2.txt";
    //cout << "------query_string " << query_string << endl;
    regex reg("h([0-9])=([^&]*)&p[0-9]=([0-9]*)&f[0-9]=([^&]*)&?");
    smatch sm;
    while (regex_search(query_string, sm, reg))
    {
      /*
      for(auto item:sm){
        cout << item << endl;
      }
      */
      string sessionId = "s"+sm.str(1);
      string serverName = sm.str(2);
      string serverPort = sm.str(3);
      if(!serverName.empty() && !serverPort.empty()){
        serverInfo_list.push_back(serverInfo(sessionId,serverName, serverPort, sm.str(4)));
      }
      query_string = sm.suffix().str();
    }
  }
  void printServerInfo_list(){
    cout << "-----serverInfo_list------" << endl;
    for(auto item:serverInfo_list){
      item.printServerInfo();
    }
    cout << "-----serverInfo_list end------" << endl;
  }

  void start()
  {
    /// print layout
    /*
    int activateSessionIndex[5]= {0};
    for(int i=0;i<4;i++){
      if(serverInfo_list[i].serverName.size() != 0){
        activateSessionIndex[i] = 1;
      }
    }
    */
    printHTMLObj.printLayout(serverInfo_list);
    ////
    for (auto &serverInfo : this->serverInfo_list)
    {
      this->resolveObj.async_resolve(
          serverInfo.serverName,
          serverInfo.port,
          [this, serverInfo](
              const boost::system::error_code &ec,
              boost::asio::ip::tcp::resolver::results_type results) {
            if (!ec)
            {
              std::make_shared<clientSession>(
                  serverInfo,
                  std::move(boost::asio::ip::tcp::socket(this->cur_io_context)),
                  results.begin()->endpoint())->start();
            }
            else{
                cout << ec.category().name() << " : " << ec.value() << " : " << ec.message();
            }
          });
    }
    this->cur_io_context.run();
  }
};


int main(void){

  client client;
  client.start();
}
/*
int main(void){
  try{

  client client;
  client.start();
  }
  catch (std::exception &e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }
}
*/