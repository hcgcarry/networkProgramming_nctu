#include "common.h"
#include <cstdlib>
#include <iostream>
#ifndef console_flag
#define console_flag
#include <memory>
#include <stdio.h>
#include <utility>
#include <boost/units/absolute.hpp>
#include <map>
#include <regex>
#include<vector>
#include"printHTML.h"
#include"serverInfo.h"
using boost::asio::ip::tcp;
using namespace std;

class client : public std::enable_shared_from_this<client>
{
private:
  vector<serverInfo> serverInfo_list;
  boost::asio::ip::tcp::resolver resolveObj;
  //boost::asio::io_context* cur_io_context;
  tcp::socket clientWebBrowserSocket;
  printHTML printHTMLObj;
public:
  client(tcp::socket clientWebBrowserSocket);
  void init(string query_string);

  void parseServerInfo(string query_string);
  void printServerInfo_list();
  void start();
};


// to keep client obj alive
#endif