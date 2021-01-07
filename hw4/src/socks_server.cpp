#include "common.h"
#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <boost/units/absolute.hpp>
#include <boost/asio.hpp>
#include "util.h"
#include "env.cpp"
#include <map>
#include <regex>
#include "firewall.cpp"
using boost::asio::ip::tcp;
using namespace std;
//#define debug
boost::asio::io_context cur_io_context;
enum clientOrDST
{
    client,
    DST
};

class SOCKS4_REQUEST_Handler
{
public:
    int VN = 4;
    bool needTransferDomainNameToIP = true;
    int CD = 1;
    string DSTIP = "0.0.0.1";
    string USERID = "";
    string DOMAIN_NAME = "";
    int DSTPORT = 80;
    //int DSTPORT = 7777;
    //string DOMAIN_NAME = "nplinux8.cs.nctu.edu.tw";
    SOCKS4_REQUEST_Handler() {}
    void parse_SOCKS4_REQUEST(char *socks4_request)
    {
        // note that if socks4_request is string ,then we need to convert receive header from char * to string ,the string will
        //terminate at null in the header message ,so we need request just be char*
        //VN | CD | DSTPORT | DSTIP(0.0.0.x) | USERID |NULL| DOMAIN NAME |NULL|
        //1 1 2 4 variable 1 variable 1
        VN = socks4_request[0];
        CD = socks4_request[1];
        // note that because char has negative ,we need to convert it to unsigned char
        DSTPORT = (unsigned char)socks4_request[2] * 256 + (unsigned char)socks4_request[3];
        //printf("socks4_request[2] %d, [3] %d\n",socks4_request[2],socks4_request[3]);
        DSTIP = to_string((unsigned char)socks4_request[4]) + "." +
                to_string((unsigned char)socks4_request[5]) + "." +
                to_string((unsigned char)socks4_request[6]) + "." +
                to_string((unsigned char)socks4_request[7]);
        smatch sm;
        regex rg("0.0.0.[1-9][0-9]*");
        if (regex_match(DSTIP, sm, rg))
        {
            needTransferDomainNameToIP = true;
        }
        else
        {
            needTransferDomainNameToIP = false;
        }
        int i;
        for (i = 8; socks4_request[i] != '\0'; i++)
        {
            USERID += socks4_request[i];
        }
        i++;
        for (; socks4_request[i] != '\0'; i++)
        {
            DOMAIN_NAME += socks4_request[i];
        }
    }
    void printInfo()
    {
        #ifdef debug
        cout << "-----socks4 parseInfo --- " << endl;
        cout << "VN:" << VN << endl
             << "CD:" << CD << endl
             << "dstport:" << DSTPORT << endl
             << "dstIP" << DSTIP << endl
             << "userid:" << USERID << endl
             << "domain_name:" << DOMAIN_NAME
             << endl
             << "needTransferdomainnametoip:" << needTransferDomainNameToIP << endl;
        #endif
    }
};
//這個class 是因為作業需要
class socks_info
{
public:
    socks_info(){};
    string source_ip;
    string source_port;
    string dest_ip;
    string dest_port;
    string command;
    string reply;
    void printInfo()
    {
        cout << "--------------------- socks info-----------------" << endl;
        cout << "<S_IP>: " << source_ip << endl;
        cout << "<S_PORT>: " << source_port << endl;
        cout << "<D_IP>: " << dest_ip << endl;
        cout << "<D_PORT>: " << dest_port << endl;
        cout << "<Command>: " << command << endl;
        cout << "<Reply>: " << reply << endl;
        cout << "-----------------socks info end--------------" << endl;
    }
};

class socks4_reply_handler
{
public:
    int reply_header_len = 8;
    char reply_header[8];
    int VN = 0;
    int CD = 90;
    socks4_reply_handler()
    {
        reply_header[0] = 0;
        reply_header[1] = 90;
        reply_header[3] = 0;
        reply_header[4] = 0;
        reply_header[5] = 0;
        reply_header[6] = 0;
        reply_header[7] = 0;
    }
    void setCD(int CD)
    {
        reply_header[1] = CD;
    }
    void setBindIP(string IP)
    {
        regex re("(\\d+)\\.(\\d+)\\.(\\d+)\\.(\\d+)");
        smatch sm;
        regex_match(IP, sm, re);
        reply_header[4] = stoi(sm[1].str());
        reply_header[5] = stoi(sm[2].str());
        reply_header[6] = stoi(sm[3].str());
        reply_header[7] = stoi(sm[4].str());
    }
    void setBindPort(int port)
    {
        reply_header[2] = port / 256;
        reply_header[3] = port % 256;
    }
    void printInfo()
    {
        #ifdef debug
        cout << "----------reply header--------" << endl;
        for (int i = 0; i < reply_header_len; i++)
        {
            printf("%d ", reply_header[i]);
        }
        cout << endl;
        cout << "----------reply header end--------" << endl;
        #endif
    }
};
class bindHandler
{
public:
    int port;
    string ip = "0.0.0.0";
    //bindHandler(int port):port(port){}
    bindHandler() {}
};
class socks_server_session
    : public std::enable_shared_from_this<socks_server_session>
{
private:
    boost::asio::ip::tcp::resolver resolveObj;
    //env envObj;
public:
    bindHandler bindHandlerObj;
    socks_info socks_info_obj;
    tcp::socket client_socket;
    tcp::socket DST_socket;
    tcp::endpoint DST_endpoint;
    socks4_reply_handler socks4_reply_handlerObj;
    SOCKS4_REQUEST_Handler socks4_request_handler_obj;
    enum
    {
        max_length = 1024
    };
    char client_dataBuffer[max_length];
    char DST_dataBuffer[max_length];
    socks_server_session(tcp::socket client_socket, int port)
        : client_socket(std::move(client_socket)), DST_socket(cur_io_context), resolveObj(cur_io_context)
    {
    }
    /*
    socks_server_session(tcp::socket client_socket)
        : client_socket(std::move(socket)), resolveObj(cur_io_context)
    {
    }
    */
    void start()
    {
        this->read_request_header();
    }

private:
    void read_request_header()
    {
        auto self(shared_from_this());
        client_socket.async_read_some(boost::asio::buffer(client_dataBuffer, max_length),
                                      [this, self](boost::system::error_code ec, std::size_t length) {
                                          if (!ec)
                                          {
                                              cout << "!!!!!!!!read_request_header receiver data!!!!!!!!" << endl;
                                              this->parseConnectString();
                                              //this->do_write();
                                          }
                                          else
                                          {
                                              cout << "!!!!!async_read request header fault" << endl;
                                              cout << ec.category().name() << " : " << ec.value() << " : " << ec.message() << endl;
                                          }
                                      });
    }

    void parseConnectString()
    {
        /*
    cout << client_socket.remote_endpoint().address().to_string() << endl;
    cout << client_socket.remote_endpoint().port() << endl;
    */
        socks4_request_handler_obj.parse_SOCKS4_REQUEST(client_dataBuffer);
        socks4_request_handler_obj.printInfo();

        string clientIP = client_socket.remote_endpoint().address().to_string();
        int clientPort = client_socket.remote_endpoint().port();
        socks_info_obj.source_ip = clientIP;
        socks_info_obj.source_port = to_string(clientPort);

        // conver domain to ip if need
        if (socks4_request_handler_obj.needTransferDomainNameToIP == true)
        {
            DomainToIP(this->socks4_request_handler_obj.DOMAIN_NAME, to_string(socks4_request_handler_obj.DSTPORT));
        }
        else
        {
            this->DST_endpoint.address(boost::asio::ip::address_v4::from_string(socks4_request_handler_obj.DSTIP));
            this->DST_endpoint.port((unsigned short)socks4_request_handler_obj.DSTPORT);
            this->bindOrConnect();
        }
    }
    void bindOrConnect()
    {
        socks_info_obj.dest_ip = this->DST_endpoint.address().to_string();
        socks_info_obj.dest_port = to_string(this->DST_endpoint.port());
        if (this->socks4_request_handler_obj.CD == 1)
        {
            socks_info_obj.command = "CONNECT";
        }
        else if (this->socks4_request_handler_obj.CD == 2)
        {
            socks_info_obj.command = "BIND";
        }
        firewall();
        /// 到這邊socks_info_obj 剩下 reply
        /// 如果firewall 擋掉 這邊要給reply
        ////
    }
    void firewall(){
        if(firewall::checkIFPassFireWall(socks_info_obj.command,socks_info_obj.dest_ip)){
            cout << "----firewall accept----" << endl;
            if (this->socks4_request_handler_obj.CD == 1)
            {
                this->asyncConnectDST();
            }
            else if (this->socks4_request_handler_obj.CD == 2)
            {
                this->bind();
            }
        }
        else{
            cout << "----firewall reject----" << endl;
            this->socks_info_obj.reply = "Reject";
            this->socks_info_obj.printInfo();
            socks4_reply_handler socks4_reply_handlerObj;
            socks4_reply_handlerObj.setCD(91);
            this->sendReply(&this->client_socket,socks4_reply_handlerObj.reply_header, 8);
        }
    }
    void asyncReadData(tcp::socket *sender_socket, tcp::socket *receiver_socket, clientOrDST clientOrDSTObj)
    {
        //cout << "-----asyncReadData --------" << clientOrDSTObj << endl;
        auto self(shared_from_this());
        char *databuffer;
        if (clientOrDSTObj == clientOrDST::client)
        {
            databuffer = client_dataBuffer;
        }
        else
        {
            databuffer = DST_dataBuffer;
        }
        (*sender_socket).async_read_some(boost::asio::buffer(databuffer, max_length), [this, self, databuffer, sender_socket, receiver_socket, clientOrDSTObj](boost::system::error_code ec, std::size_t length) {
            if (!ec)
            {
                databuffer[length] = '\0';
                //string message(databuffer);

#ifdef debug
                if (clientOrDSTObj == clientOrDST::client)
                    cout << "client";
                else
                    cout << "DST";
                cout << "###########receive data success############# " << endl;
                cout << databuffer << endl;
                cout << "#################receive data end#############" << endl;
#endif
                //self->output_shell(self->serverInfoObj.sessionId, tmp);
                // check if we can send

                self->asyncWriteData(databuffer, length, sender_socket, receiver_socket, clientOrDSTObj);
            }
            else
            {
                if (clientOrDSTObj == clientOrDST::client){
                    cout << "client";
                    self->client_socket.close();
                }
                else
                {
                    cout << "DST";
                    self->DST_socket.close();
                }
                cout << "!!!!!async_read fault" << endl;
                cout << ec.category().name() << " : " << ec.value() << " : " << ec.message() << endl;
            }
        });
    }
    void asyncWriteData(char *message, int messageLen, tcp::socket *sender_socket, tcp::socket *receiver_socket, clientOrDST clientOrDSTObj)
    {
        //if (!test_file.eof())
        //cout << "----asyncwritedata -----" << clientOrDSTObj << endl;
        auto self(shared_from_this());
        (*receiver_socket).async_write_some(boost::asio::buffer(message, messageLen), [message, this, self, sender_socket, receiver_socket, clientOrDSTObj](boost::system::error_code ec, std::size_t length) {
            if (!ec)
            {
                #ifdef debug
                if (clientOrDSTObj == clientOrDST::client)
                    cout << "client";
                else
                    cout << "DST";
                cout << "###########send data success############# " << endl;
                cout << message << endl;
                cout << "#################send data end#############" << endl;
                #endif
                self->asyncReadData(sender_socket, receiver_socket, clientOrDSTObj);
            }
            else
            {
                if (clientOrDSTObj == clientOrDST::client)
                    cout << "client";
                else
                    cout << "DST";
                cout << "!!!!!async_write fault" << endl;
                cout << ec.category().name() << " : " << ec.value() << " : " << ec.message() << endl;
            }
        });
    }
    void sendReplyAndStartRelay(tcp::socket *socketPtr, char *message, int messageLen)
    {
        socks_info_obj.printInfo();
        auto self(shared_from_this());
        /*
        for(int i=0;i<messageLen;i++){
            printf("%d",message[i]);
        }
        cout << endl;
        cout << "reply socket ptr" << socketPtr << endl;
        cout << endl;
        */
        boost::asio::async_write((*socketPtr), boost::asio::buffer(message, messageLen), [message, this, self](boost::system::error_code ec, std::size_t length) {
            if (!ec)
            {
                cout << "###########send reply and start relay success ############ " << endl;
                //cout << message << endl;
                self->asyncReadData(&self->client_socket, &self->DST_socket, clientOrDST::client);
                self->asyncReadData(&self->DST_socket, &self->client_socket, clientOrDST::DST);
            }
            else
            {
                cout << "!!!!!send reply and start relay fault" << endl;
                cout << ec.category().name() << " : " << ec.value() << " : " << ec.message() << endl;
            }
        });
    }
    void DomainToIP(string DomainName, string port)
    {
        auto self(shared_from_this());
        this->resolveObj.async_resolve(
            DomainName,
            port,
            [this, self](
                const boost::system::error_code &ec,
                boost::asio::ip::tcp::resolver::results_type results) {
                if (!ec)
                {
                    self->DST_endpoint = results.begin()->endpoint();
                    string resultIP = results.begin()->endpoint().address().to_string();
                    self->socks4_request_handler_obj.DSTIP = resultIP;
                    cout << "-------domainToIP    resultIP " << resultIP << endl;
                    //self->do_write();
                    self->bindOrConnect();
                }
                else
                {
                    cout << ec.category().name() << " : " << ec.value() << " : " << ec.message() << endl;
                }
            });
    }
    ////////////////////////////////////// connect
    void asyncConnectDST()
    {
        auto self = this->shared_from_this();
        // Call asynchronous connect function provided by boost.
        //cout << "dst endpoint ,ip:" << this->DST_endpoint.address() << " port "
        //<< this->DST_endpoint.port() << endl;

        this->DST_socket.async_connect(
            this->DST_endpoint,
            [this, self](const boost::system::error_code &ec) {
                if (!ec)
                {
                    cout << "----connect to DST success----" << endl;
                    self->socks4_reply_handlerObj.setCD(90);
                    self->socks_info_obj.reply = "Accept";
                    self->sendReplyAndStartRelay(&self->client_socket, self->socks4_reply_handlerObj.reply_header, 8);
                }
                else
                {

                    self->socks4_reply_handlerObj.setCD(91);
                    self->socks4_reply_handlerObj.printInfo();
                    self->socks_info_obj.reply = "Reject";
                    self->socks_info_obj.printInfo();
                    self->sendReply(&self->client_socket, self->socks4_reply_handlerObj.reply_header, 8);
                    //self->asyncRead(&self->DST_socket, &(self->client_socket), clientOrDST::DST);
                    cout << "!!!!!async_connect fault" << endl;
                    cout << ec.category().name() << " : " << ec.value() << " : " << ec.message() << endl;
                }
            });
    }
    /////////////////////bind
    void bind()
    {
        /// first reply
        bindAcceptor();
        /// reply
        socks4_reply_handler socks4_reply_handlerObj;
        socks4_reply_handlerObj.setCD(90);
        socks4_reply_handlerObj.setBindIP(this->bindHandlerObj.ip);
        socks4_reply_handlerObj.setBindPort(this->bindHandlerObj.port);
        socks4_reply_handlerObj.printInfo();
        socks_info_obj.reply = "Accept";
        sendReply(&client_socket, socks4_reply_handlerObj.reply_header, 8);
    }
    void bindAcceptor()
    {
        //auto  bindAcceptor_shared_ptr= std::make_shared<tcp::acceptor>(cur_io_context,tcp::endpoint(tcp::v4(),0));
        auto bindAcceptor_shared_ptr = std::make_shared<tcp::acceptor>(cur_io_context, tcp::endpoint(tcp::v4(), 0));
        bindHandlerObj.port = bindAcceptor_shared_ptr->local_endpoint().port();
        auto self(shared_from_this());
        bindAcceptor_shared_ptr->async_accept(
            [this, self, bindAcceptor_shared_ptr](boost::system::error_code ec, tcp::socket socket) {
                if (!ec)
                {
                    ////////// reply 2
                    cout << "#############bind Accept success##########" << endl;
                    self->DST_socket = std::move(socket);
                    socks4_reply_handler socks4_reply_handlerObj;
                    socks4_reply_handlerObj.setCD(90);
                    socks4_reply_handlerObj.setBindIP(this->bindHandlerObj.ip);
                    socks4_reply_handlerObj.setBindPort(this->bindHandlerObj.port);
                    socks4_reply_handlerObj.printInfo();
                    self->socks_info_obj.reply = "Accept";
                    self->sendReplyAndStartRelay(&client_socket, socks4_reply_handlerObj.reply_header, 8);
                }
                else
                {
                    cout << "!!!!!!!bindDST Accept fail" << endl;
                    cout << ec.category().name() << " : " << ec.value() << " : " << ec.message() << endl;
                    self->socks_info_obj.reply = "Reject";
                    socks4_reply_handlerObj.setCD(91);
                    self->sendReply(&client_socket, socks4_reply_handlerObj.reply_header, 8);
                }
            });
    }
    void sendReply(tcp::socket *socketPtr, char *message, int messageLen)
    {
        socks_info_obj.printInfo();
        auto self(shared_from_this());
        boost::asio::async_write((*socketPtr), boost::asio::buffer(message, messageLen), [message, this, self](boost::system::error_code ec, std::size_t length) {
            if (!ec)
            {
                cout << "###########send reply success ############ " << endl;
            }
            else
            {
                cout << "!!!!!send reply fault" << endl;
                cout << ec.category().name() << " : " << ec.value() << " : " << ec.message() << endl;
            }
        });
    }
};

class server
{
private:
    //boost::asio::io_context cur_io_context;
    tcp::acceptor acceptor_;
    int port;

public:
    server(short port)
        : port(port), acceptor_(cur_io_context, tcp::endpoint(tcp::v4(), port))
    {
        //: acceptor_(cur_io_context, tcp::endpoint(tcp::v4(), port))
        //this->acceptor_.set_option(boost::asio::detail::client_socketoption::integer<SOL_SOCKET, SO_REUSEADDR>(1));
        //this->acceptor_.set_option(boost::asio::detail::client_socketoption::integer<SOL_SOCKET, SO_REUSEPORT>(1));
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
                    pid_t child_pid;
                    cur_io_context.notify_fork(
                        cur_io_context.fork_event::fork_prepare);

                    while ((child_pid = fork()) < 0)
                    {
                        waitpid(-1, NULL, WNOHANG);
                    }
                    if (child_pid == 0)
                    //if (child_pid == 0)
                    {
                        cur_io_context.notify_fork(
                            cur_io_context.fork_event::fork_child);

                        this->acceptor_.close();

                        std::make_shared<socks_server_session>(std::move(socket), this->port)->start();
                    }
                    else
                    {
                        //while (1)
                        usleep(100);

                        cur_io_context.notify_fork(
                            cur_io_context.fork_event::fork_parent);

                        socket.close();

                        this->do_accept();
                    }
                }
                else
                {
                    cout << "!!!!!!!asyncAccept fault" << endl;
                    cout << ec.category().name() << " : " << ec.value() << " : " << ec.message() << endl;
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