#include "common.h"
//#include<cstdio>
#include<regex>
#include <fstream>
#include <boost/units/absolute.hpp>
#include <boost/asio.hpp>
#include<boost/property_tree/xml_parser.hpp>
#include<regex>
#include"socks_client.cpp"
#include "serverInfo.h"
//#define debug
using boost::asio::ip::tcp;
//#define debug
//#include "printHTML.h"

extern boost::asio::io_context cur_io_context;


class clientSession
    : public std::enable_shared_from_this<clientSession>
{
    enum
    {
        max_length = 1024
    };
    char data_[max_length+1];
    fstream test_file;
    //printHTML printHTMLObj;
    serverInfo serverInfoObj;
    tcp::socket socks_socket;
    tcp::endpoint socks_endpoint;
public:
    socks_client socks_client_obj;
    clientSession(serverInfo serverInfoObj, tcp::socket socks_socket, tcp::endpoint socks_endpoint) : serverInfoObj(std::move(serverInfoObj)), socks_socket(std::move(socks_socket)), socks_endpoint(std::move(socks_endpoint))
    {
        
        init();
    }
    void init(){
        string filePath = "./test_case/" + serverInfoObj.testFile;
        //cout << "-------filePath " << filePath << endl;
        //cout << test_file.fail() << endl;
        this->test_file.open(filePath, ios::in);
        if (test_file.fail()) 
        {
            //perror("open file error");
            cout << "!!!!!!!!open test file error!!!!! "<< endl;
            exit(0);
        }

    }
    void start()
    {
        //serverInfoObj.printServerInfo();
        //jprintClientSession();
        socks_client_obj.makeSOCKS4Header(stoi(serverInfoObj.port),"0.0.0.1",serverInfoObj.serverName);
        //socks_client_obj.printSocks4Header();
        asyncConnect();
    }
    void printClientSession()
    {
        cout << "-------clientSession------" << endl;
        serverInfoObj.printServerInfo();
        cout << "-------clientSessionEnd------" << endl;
    }
    void asyncConnect()
    {
        auto self = this->shared_from_this();
        // Call asynchronous connect function provided by boost.
        this->socks_socket.async_connect(
            this->socks_endpoint,
            [this, self](const boost::system::error_code &ec) {
                if (!ec)
                {
                    //cout << "connect to socks server success"  << endl;
                    self->sendSocks4Header();
                }
            });
    }
    void sendSocks4Header(){
        //socks_client_obj.printSocks4Header();
        char *message = socks_client_obj.socks4Header;
        int messageLen = socks_client_obj.socks4HeaderLen;
        auto self(shared_from_this());
        boost::asio::async_write(this->socks_socket,boost::asio::buffer(message,messageLen) ,
         [message, this, self](boost::system::error_code ec, std::size_t length) {
             //cout << "------send socks4 header success------" << endl;
             self->receiveSocks4Reply();
         });
    }
    void receiveSocks4Reply(){
        auto self(shared_from_this());
        socks_socket.async_read_some(boost::asio::buffer(data_, max_length),
        [this,self](boost::system::error_code ec,std::size_t length){
            if(!ec){
                if(self->socks_client_obj.replyIsSuccess(data_)){
                    //cout << "-----reply is success-----" << endl;
                    self->asyncRead();
                }
                else{
                    //cout << "----reply is fail-----" << endl;
                }
            }
            else{
            }
        });
    }
    void asyncRead()
    {
        auto self(shared_from_this());
        socks_socket.async_read_some(boost::asio::buffer(data_, max_length),
                                [this, self](boost::system::error_code ec, std::size_t length) {
                                    if (!ec)
                                    {
                                        data_[length] = '\0';
                                        string tmp(data_);

                                        #ifdef debug
                                        cout <<endl<< "###########receive data length :" << length <<"############# " << endl;
                                        cout << tmp << endl;
                                        cout << "#################receive data end#############" << endl;
                                        #endif
                                        self->output_shell(self->serverInfoObj.sessionId, tmp);
                                        // check if we can send 
                                        regex re("% ");
                                        smatch sm;
                                        if(regex_search(tmp,sm,re))
                                        {
                                            
                                            self->asyncWrite();
                                        }
                                        else
                                        {
                                            self->asyncRead();
                                        }
                                    }
                                });
    }
    void output_shell(string session, string content)
    {
    // convert content to html txt format
    content = boost::property_tree::xml_parser::encode_char_entities(content);
    regex unix_nexLine("\\n");
    content = regex_replace(content,unix_nexLine, "&NewLine;");

        printf("<script>document.getElementById('%s').innerHTML += '%s';</script>", session.c_str(), content.c_str());
        cout << flush;
    }

    void output_command(string session, string content)
    {
        // convert windows txt \r\n to \n
        if(content.size() && content.back() == '\r' ){
            content.erase( content.length()-1, 1);
        }
        // convert content to html txt format
        content = boost::property_tree::xml_parser::encode_char_entities(content);
        content += "\n";
        regex unix_nexLine("\\n");
        content = regex_replace(content,unix_nexLine, "&NewLine;");
        printf("<script>document.getElementById('%s').innerHTML += '<b>%s</b>';</script>", session.c_str(), content.c_str());
        cout << flush;
    }
    void asyncWrite()
    {
        
        string message;
        //if (!test_file.eof())
        if (getline(test_file,message)){
            output_command(this->serverInfoObj.sessionId, message);
            
            if(message.size() && message.back() == '\r' ){
                message.erase( message.length()-1, 1);
            }
            message+="\n";
            //cout << "message size" << message.size() << endl;
            //const char *tmp = message.c_str();
            /*
            for(int i=0;i<message.size();i++){
                printf("1 %d",message[i]);
            }
            for(int i=0;i<message.size();i++){
                printf("2 %d",tmp[i]);
            }
            */
        }
        else
        {
            return;
        }
        auto self(shared_from_this());
        socks_socket.async_write_some(boost::asio::buffer(message),
                                 [this, self, message](boost::system::error_code ec, std::size_t length) {
                                     if (!ec)
                                     {
                                         #ifdef debug
                                         cout << endl<<"*****send data ******" << endl;
                                         cout << message << endl;
                                         cout << "******send data end*****" << endl;
                                         #endif
                                         //printHTML.outputShe;
                                         if(message!="exit") self->asyncRead();
                                     }
                                 });
    }
};