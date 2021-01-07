#include"printHTML.h"
    printHTML::printHTML(){}
    void printHTML::asyncWrite(string str, tcp::socket *socket_)
    {
        //if (!test_file.eof())
        async_write(*socket_,boost::asio::buffer(str),
                                 [this](boost::system::error_code ec, std::size_t length) {
                                     if (!ec)
                                     {
                                     }
                                 });
    }
    void printHTML::output_shell(string session, string content,tcp::socket* socket_)
    {
    // convert content to html txt format
    content = boost::property_tree::xml_parser::encode_char_entities(content);
    regex unix_nexLine("\\n");
    content = regex_replace(content,unix_nexLine, "&NewLine;");
      sprintf(buf,"<script>document.getElementById('%s').innerHTML += '%s';</script>", session.c_str(), content.c_str());
      string message(buf);
      asyncWrite(message,socket_);
    }

    void printHTML::output_command(string session, string content,tcp::socket* socket_)
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
        sprintf(buf,"<script>document.getElementById('%s').innerHTML += '<b>%s</b>';</script>", session.c_str(), content.c_str());
        string message(buf);
        asyncWrite(message,socket_);
    }

  void printHTML::printConsoleLayout(vector<serverInfo>& serverInfo_list,tcp::socket* socket_)
  {
    string content = "Content-type: text/html\r\n\r\n\
<!DOCTYPE html>\
<html lang=\"en\">\
  <head>\
    <meta charset=\"UTF-8\" />\
    <title>NP Project 3 Sample Console</title>\
    <link\
      rel=\"stylesheet\"\
      href=\"https://cdn.jsdelivr.net/npm/bootstrap@4.5.3/dist/css/bootstrap.min.css\"\
      integrity=\"sha384-TX8t27EcRE3e/ihU7zmQxVncDAy5uIKz4rEkgIXeMed4M0jlfIDPvg6uqKI2xXr2\"\
      crossorigin=\"anonymous\"\
    />\
    <link\
      href=\"https://fonts.googleapis.com/css?family=Source+Code+Pro\"\
      rel=\"stylesheet\"\
    />\
    <link\
      rel=\"icon\"\
      type=\"image/png\"\
      href=\"https://cdn0.iconfinder.com/data/icons/small-n-flat/24/678068-terminal-512.png\"\
    />\
    <style>\
      * {\
        font-family: 'Source Code Pro', monospace;\
        font-size: 1rem !important;\
      }\
      body {\
        background-color: #212529;\
      }\
      pre {\
        color: #cccccc;\
      }\
      b {\
        color: #01b468;\
      }\
    </style>\
  </head>\
  <body>\
    <table class=\"table table-dark table-bordered\">\
      <thead>\
        <tr>";
        for(int i=0;i<serverInfo_list.size();i++){
          if(serverInfo_list[i].serverName.size() != 0)
          content += "<th scope=\"col\">" +serverInfo_list[i].serverName + ":" + serverInfo_list[i].port+ "</th>";
        }
        content +="\
        </tr>\
      </thead>\
      <tbody>\
        <tr>\
        ";
        for(int i=0;i<serverInfo_list.size();i++){
          if(serverInfo_list[i].serverName.size() != 0)
          content +="<td><pre id=" +serverInfo_list[i].sessionId+" class=\"mb-0\"></pre></td>" ;
        }
          content +="\
        </tr>\
      </tbody>\
    </table>\
  </body>\
</html>\
";
  asyncWrite(content,socket_);
  }