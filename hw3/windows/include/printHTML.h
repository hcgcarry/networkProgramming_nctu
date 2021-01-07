
#include"common.h"
#include"serverInfo.h"
#include <boost/units/absolute.hpp>
#include <boost/asio.hpp>
#include<boost/property_tree/xml_parser.hpp>
#ifndef printHTML_flag
#define printHTML_flag
#include<regex>
using boost::asio::ip::tcp;
class printHTML
{
    char buf[4096];
public:
    printHTML();
    void asyncWrite(string str, tcp::socket *socket_);
    void output_shell(string session, string content,tcp::socket* socket_);
    void output_command(string session, string content,tcp::socket* socket_);

  void printConsoleLayout(vector<serverInfo>& serverInfo_list,tcp::socket* socket_);
};
#endif