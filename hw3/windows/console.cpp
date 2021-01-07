#include "common.h"
#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdio.h>
#include <utility>
#include <boost/units/absolute.hpp>
#include <map>
#include <regex>
#include<vector>
#include"printHTML.h"
#include"clientSession.cpp"
#include"serverInfo.h"
#include"console.h"
using boost::asio::ip::tcp;
using namespace std;
extern vector<std::shared_ptr<client>> consoleClientVector;

  client::client(tcp::socket clientWebBrowserSocket):resolveObj(cur_io_context),clientWebBrowserSocket(std::move(clientWebBrowserSocket)){
    #ifdef debug
    //printServerInfo_list();
    //envObj.getEnv();
    //envObj.printEnvMap();
    #endif
  };
  void client::init(string query_string){
    parseServerInfo(query_string);
    this->start();
  }

  void client::parseServerInfo(string query_string)
  {
    //envObj.printEnvMap();
    //string query_string = "h0=nplinux8.cs.nctu.edu.tw&p0=7778&f0=t1.txt&h1=&\
p1=&f1=&h2=&p2=&f2=&h3=&p3=&f3=&h4=&p4=&f4=";
//string query_string ="h0=nplinux8.cs.nctu.edu.tw&p0=7778&f0=t1.txt&h1=nplinux8.cs.nctu.edu.tw&p1=7776&f1=t2.txt&h2=&p2=&f2=&h3=&p3=&f3=&h4=&p4=&f4=";
//string query_string ="h0=nplinux8.cs.nctu.edu.tw&p0=7778&f0=t2.txt";
//parse query_string
    cout << "------query_string " << query_string << endl;
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
  void client::printServerInfo_list(){
    cout << "-----serverInfo_list------" << endl;
    for(auto item:serverInfo_list){
      item.printServerInfo();
    }
    cout << "-----serverInfo_list end------" << endl;
  }

  void client::start()
  {
    printHTMLObj.printConsoleLayout(serverInfo_list,&clientWebBrowserSocket);
    this->printServerInfo_list();
    ////
    for (auto &serverInfo : this->serverInfo_list)
    {
      auto self(shared_from_this());
      consoleClientVector.push_back(self);
      resolveObj.async_resolve(
          serverInfo.serverName,
          serverInfo.port,
          [this,self, serverInfo](
              const boost::system::error_code &ec,
              boost::asio::ip::tcp::resolver::results_type results) {
            if (!ec)
            {
              std::make_shared<clientSession>(
                  serverInfo,
                  &clientWebBrowserSocket,
                  std::move(boost::asio::ip::tcp::socket(cur_io_context)),
                  results.begin()->endpoint())->start();
            }
            else{
                cout << ec.category().name() << " : " << ec.value() << " : " << ec.message();
            }
          });
    }
  }


// to keep client obj alive