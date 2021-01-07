#include "common.h"
#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <boost/units/absolute.hpp>
#include <boost/asio.hpp>
#include"env.cpp"
#include<map>
#include <regex>
#include"console.h"
#include"panel.cpp"
using boost::asio::ip::tcp;
using namespace std;
boost::asio::io_context cur_io_context;
vector<std::shared_ptr<client>> consoleClientVector;

typedef struct connectState
{
  string requestHeader;
  string cgiName;
} ConnectState;

class session
    : public std::enable_shared_from_this<session>
{
private:
  ConnectState connectStateObj;
  env envObj;
  printHTML printHTMLObj;
public:
  tcp::socket socket_;
  enum
  {
    max_length = 1024
  };
  char data_[max_length];
  session(tcp::socket socket)
      : socket_(std::move(socket))
  {
    
  }
  void start()
  {
    do_read();
  }

private:
  void do_read()
  {
    auto self(shared_from_this());
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
                            [this, self](boost::system::error_code ec, std::size_t length) {
                              if (!ec)
                              {
                                cout << "----receive data ----" << endl;
                                self->parseConnectString();
                                self->do_write();
                              }
                            });
  }
  void parseConnectString()
  {
    /*
    cout << socket_.remote_endpoint().address().to_string() << endl;
    cout << socket_.remote_endpoint().port() << endl;
    */
   // parse requestheader and set envMap  to get connection information
    string clientIP = socket_.remote_endpoint().address().to_string() ;
    int clientPort =  socket_.remote_endpoint().port() ;
    #ifdef debug
    cout << this->data_ << endl;
    #endif
    string tmp(data_);
    envObj.parseAndsetEnvMapVariable(tmp,clientIP,to_string(clientPort));
   ////// get filename(cigName)
    string fileName;
    stringstream ss(tmp);
    getline(ss, tmp, '/');
    string tmp2;
    getline(ss, tmp2, '?');
    stringstream ss2(tmp2);
    ss2 >> fileName ;
    this->connectStateObj.cgiName = fileName;
    this->connectStateObj.requestHeader = tmp;
    //self->forkChild(fileName, self->socket_.native_handle());
    //self->socket_.close();
  }

  void execCGI()
  {
    if(this->connectStateObj.cgiName == "console.cgi"){
      string query_string  = envObj.envMap["QUERY_STRING"] ;
      std::make_shared<client>(std::move(socket_))->init(query_string);
    }
    else if(this->connectStateObj.cgiName == "panel.cgi"){
      Panel panelObj;
      string panelHTML = panelObj.getPanel();
      printHTMLObj.asyncWrite(panelHTML,&socket_);
    }
  }

  void do_write()
  {
    string respondHeader = "HTTP/1.0 200 OK\r\n";
    auto self(shared_from_this());
    boost::asio::async_write(socket_, boost::asio::buffer(respondHeader),
                             [this, self](boost::system::error_code ec, std::size_t /*length*/) {
                               if (!ec)
                               {
                                 self->execCGI();
                               }
                             });
  }
};

class server
{
private:
  tcp::acceptor acceptor_;

public:
  server(short port)
      : acceptor_(cur_io_context, tcp::endpoint(tcp::v4(), port))
  {
    //: acceptor_(cur_io_context, tcp::endpoint(tcp::v4(), port))
    this->acceptor_.set_option(boost::asio::detail::socket_option::integer<SOL_SOCKET, SO_REUSEADDR> (1));
    //this->acceptor_.set_option(boost::asio::detail::socket_option::integer<SOL_SOCKET, SO_REUSEPORT> (1));

    this->run();
  }
  void run()
  {
    this->do_accept();
    cur_io_context.run();
  }

private:
  void do_accept()
  {
    acceptor_.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket) {
          if (!ec)
          {
            //if (child_pid != 0)
              std::make_shared<session>(std::move(socket))->start();
              this->do_accept();
          }
        });
  }
};

int main(int argc, char *argv[])
{
  {
    if (argc != 2)
    {
      std::cerr << "Usage: async_tcp_echo_server <port>\n";
      return 1;
    }
    int port = atoi(argv[1]);
    cout << "---------run on port -------------:" << port << endl;
    //boost::asio::io_context io_context;
    server s(port);

    //io_context.run();
  }

  return 0;
}

/*
int main(int argc, char *argv[])
{
  try
  {
    if (argc != 2)
    {
      std::cerr << "Usage: async_tcp_echo_server <port>\n";
      return 1;
    }
    int port = atoi(argv[1]);
    cout << "---------run on port -------------:" << port << endl;

    //boost::asio::io_context io_context;

    server s(port);

    //io_context.run();
  }
  catch (std::exception &e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}

*/