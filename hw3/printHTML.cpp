#include"common.h"
#include"serverInfo.h"

class printHTML
{

public:
  void output_shell(string session, string content)
  {
    printf("<script>document.getElementById('%s').innerHTML += '%s';</script>", session.c_str(), content.c_str());
    cout << flush;
  }

  void output_command(string session, string content)
  {
    printf("<script>document.getElementById('%s').innerHTML += '%s';</script>", session.c_str(), content.c_str());
    cout << flush;
  }

  void printLayout(vector<serverInfo>& serverInfo_list)
  {
    printf("Content-type: text/html\r\n\r\n");
    printf("\
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
        <tr>");
        for(int i=0;i<serverInfo_list.size();i++){
          if(serverInfo_list[i].serverName.size() != 0)
          cout <<"<th scope=\"col\">" << serverInfo_list[i].serverName << ":" << serverInfo_list[i].port<< "</th>";
        }
        printf("\
        </tr>\
      </thead>\
      <tbody>\
        <tr>\
        ");
        for(int i=0;i<serverInfo_list.size();i++){
          if(serverInfo_list[i].serverName.size() != 0)
          cout <<"<td><pre id=" <<serverInfo_list[i].sessionId<<" class=\"mb-0\"></pre></td>" ;
        }
          printf("\
        </tr>\
      </tbody>\
    </table>\
  </body>\
</html>\
");
  }
};