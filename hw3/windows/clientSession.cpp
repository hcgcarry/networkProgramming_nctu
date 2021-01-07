#include "common.h"
//#include<cstdio>
#include <fstream>
#include <boost/units/absolute.hpp>
#include <boost/asio.hpp>
#include<boost/property_tree/xml_parser.hpp>
#include<regex>
#include "serverInfo.h"
using boost::asio::ip::tcp;
//#define debug
//#include "printHTML.h"
#include"printHTML.h"

class clientSession
    : public std::enable_shared_from_this<clientSession>
{

    enum
    {
        max_length = 1024
    };
    char data_[max_length+1];
    fstream test_file;
    printHTML printHTMLObj;
    serverInfo serverInfoObj;
    tcp::socket *clientWebBrowserSocketPtr;
    tcp::socket serverSocket;
    tcp::endpoint endpoint_;
    
public:
    clientSession(serverInfo serverInfoObj,tcp::socket *clientWebBrowserSocketPtr, tcp::socket serverSocket, tcp::endpoint endpoint_)\
     : serverInfoObj(std::move(serverInfoObj)), clientWebBrowserSocketPtr(clientWebBrowserSocketPtr),serverSocket(std::move(serverSocket)), endpoint_(std::move(endpoint_))
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
        //jprintClientSession();
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
        this->serverSocket.async_connect(
            this->endpoint_,
            [this, self](const boost::system::error_code &ec) {
                if (!ec)
                {
                    self->asyncRead();
                }
            });
    }
    void asyncRead()
    {
        auto self(shared_from_this());
        serverSocket.async_read_some(boost::asio::buffer(data_, max_length),
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
                                        self->printHTMLObj.output_shell(self->serverInfoObj.sessionId, tmp,clientWebBrowserSocketPtr);
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
        bool exitFlag = false;
        if (getline(test_file,message)){
            printHTMLObj.output_command(this->serverInfoObj.sessionId, message,clientWebBrowserSocketPtr);
        }
        else
        {
            return;
        }
        if(message.size() && message.back() == '\r' ){
            message.erase( message.length()-1, 1);
        }
        message+='\n';
        auto self(shared_from_this());
        serverSocket.async_write_some(boost::asio::buffer(message),
                                 [this, self, message](boost::system::error_code ec, std::size_t length) {
                                     if (!ec)
                                     {
                                         #ifdef debug
                                         cout << endl<<"*****send data ******" << endl;
                                         cout << message << endl;
                                         cout << "******send data end*****" << endl;
                                         #endif
                                         //printHTML.outputShe;
                                         if(message != "exit") self->asyncRead();
                                     }
                                 });
    }
};