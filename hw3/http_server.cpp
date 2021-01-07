#include "common.h"
#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <boost/units/absolute.hpp>
#include <boost/asio.hpp>
#include "util.h"
#include"env.cpp"
#include<map>
#include <regex>
using boost::asio::ip::tcp;
using namespace std;

typedef struct connectState
{
  string requestHeader;
  string cgiName;
} ConnectState;

class session
    : public std::enable_shared_from_this<session>
{
private:
  boost::asio::ip::tcp::resolver resolveObj;
  ConnectState connectStateObj;
  env envObj;
public:
  tcp::socket socket_;
  enum
  {
    max_length = 1024
  };
  char data_[max_length];
  session(tcp::socket socket,boost::asio::io_context &cur_io_context)
      : socket_(std::move(socket)),resolveObj(cur_io_context)
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
                                this->parseConnectString();
                                //this->do_write();
                              }
                            });
  }
  void parseConnectString()
  {
    /*
    cout << socket_.remote_endpoint().address().to_string() << endl;
    cout << socket_.remote_endpoint().port() << endl;
    */
    string clientIP = socket_.remote_endpoint().address().to_string() ;
    int clientPort =  socket_.remote_endpoint().port() ;
    cout << this->data_ << endl;
    string tmp(data_);
    envObj.parseAndsetEnvMapVariable(tmp,clientIP,to_string(clientPort));
    string fileName;
    stringstream ss(tmp);
    getline(ss, tmp, '/');
    string tmp2;
    getline(ss, tmp2, '?');
    stringstream ss2(tmp2);
    ss2 >> fileName ;
    this->connectStateObj.cgiName = fileName;
    this->connectStateObj.requestHeader = tmp;
    //this->execChild(fileName);
    //self->forkChild(fileName, self->socket_.native_handle());
    //self->socket_.close();
    this->serverDomainToIP(envObj.envMap["SERVER_ADDR"] ,envObj.envMap["SERVER_PORT"]);
  }
  void serverDomainToIP(string serverDomainName,string port){
    auto self(shared_from_this());
      this->resolveObj.async_resolve(
        serverDomainName,
        port,
          [self](
              const boost::system::error_code &ec,
              boost::asio::ip::tcp::resolver::results_type results) {
            if (!ec)
            {
              string resultIP =  results.begin()->endpoint().address().to_string();
              self->envObj.envMap["SERVER_ADDR"]  =  resultIP;
              //cout << "-------resultIP "<<resultIP << endl;
              self->do_write();
            }
            else{
                cout << ec.category().name() << " : " << ec.value() << " : " << ec.message();
            }
          });
  }

  void execChild()
  {
    envObj.setEnv();
    
    envObj.printEnv();
    //cout << "---exeName " << exeName << endl;
    string exeName = connectStateObj.cgiName;
    //kchar filePath = boost::util absolute();
    // (char*)exeName.c_str()
    //string dirPath = get_current_dir_name();
    string dirPath = ".";
    //cout << dirPath << endl;
    string filePath = dirPath + "/" + exeName;
    cout << "-------- filepath " << filePath << endl;
    //char *argvlist[] = {"/usr/bin/python3", (char *)filePath.c_str(), NULL};
    char *argvlist[] = {(char*)filePath.c_str(), (char *)filePath.c_str(), NULL};
    //if (childPid == 0)
    dup2(this->socket_.native_handle(), 0);
    dup2(this->socket_.native_handle(), 1);
    dup2(this->socket_.native_handle(), 2);

    //char *argv[] = {NULL};
    if (execv(argvlist[0], argvlist ) < 0)
    //if (execl(exeName.c_str(), exeName.c_str(), nullptr) < 0)
    {
      perror("!!!!!!!execve error!!!!!!!!!!!");
    }
    exit(0);
  }

  void do_write()
  {
    string respondHeader = "HTTP/1.0 200 OK\r\n";
    auto self(shared_from_this());
    boost::asio::async_write(socket_, boost::asio::buffer(respondHeader),
                             [this, self](boost::system::error_code ec, std::size_t /*length*/) {
                               if (!ec)
                               {
                                 this->execChild();
                               }
                             });
  }
};

class server
{
private:
  boost::asio::io_context cur_io_context;
  tcp::acceptor acceptor_;

public:
  server(short port)
      : acceptor_(cur_io_context, tcp::endpoint(tcp::v4(), port))
  {
    //: acceptor_(cur_io_context, tcp::endpoint(tcp::v4(), port))
    this->acceptor_.set_option(boost::asio::detail::socket_option::integer<SOL_SOCKET, SO_REUSEADDR> (1));
    this->acceptor_.set_option(boost::asio::detail::socket_option::integer<SOL_SOCKET, SO_REUSEPORT> (1));

    this->run();
  }
  void run()
  {
    this->do_accept();
    this->cur_io_context.run();
  }

private:
  void do_accept()
  {
    acceptor_.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket) {
          if (!ec)
          {
            pid_t child_pid;
            this->cur_io_context.notify_fork(
                this->cur_io_context.fork_event::fork_prepare);

            while ((child_pid = fork()) < 0)
            {
              waitpid(-1, NULL, WNOHANG);
            }
            //if (child_pid != 0)
            if (child_pid == 0)
            {
              this->cur_io_context.notify_fork(
                  this->cur_io_context.fork_event::fork_child);

              this->acceptor_.close();

              std::make_shared<session>(std::move(socket),this->cur_io_context)->start();
            }
            else
            {
              usleep(100);

              this->cur_io_context.notify_fork(
                  this->cur_io_context.fork_event::fork_parent);

              socket.close();

              this->do_accept();
            }
          }
          else
          {
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