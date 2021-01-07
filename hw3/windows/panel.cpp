#include "common.h"
using namespace std;
class Panel{
  public:
Panel(){}
string getPanel(void){
int  N_SERVERS = 5;

string FORM_METHOD = "GET";
string FORM_ACTION = "console.cgi";
string TEST_CASE_DIR = "test_case";

string test_case_menu="";
for(int i=1;i<=10;i++){
  test_case_menu += "<option value=\"t"+to_string(i)+".txt\">t"+to_string(i)+".txt</option>";
}

string host_menu="";
for(int i=1 ;i<=12;i++){
host_menu += "<option value=\"nplinux"+to_string(i)+".cs.nctu.edu.tw\">nplinux"+to_string(i)+"</option>";
}


string pageContent = "\
Content-type: text/html\r\n\r\n\
<!DOCTYPE html>\
<html lang=\"en\">\
  <head>\
    <title>NP Project 3 Panel</title>\
    <link\
      rel=\"stylesheet\"\
      href=\"https://cdn.jsdelivr.net/npm/bootstrap@4.5.3/dist/css/bootstrap.min.css\"\
      integrity=\"sha384-TX8t27EcRE3e/ihU7zmQxVncDAy5uIKz4rEkgIXeMed4M0jlfIDPvg6uqKI2xXr2\"\
      crossorigin=\"anonymous\"\
    />\
    <lin\
      href=\"https://fonts.googleapis.com/css?family=Source+Code+Pro\"\
      rel=\"stylesheet\"\
    />\
    <lin\
      rel=\"icon\"\
      type=\"image/png\"\
      href=\"https://cdn4.iconfinder.com/data/icons/iconsimple-setting-time/512/dashboard-512.png\"\
    />\
    <style>\
      * {\
        font-family: 'Source Code Pro', monospace;\
      }\
    </style>\
  </head>\
  <body class=\"bg-secondary pt-5\">\
    <form action=\""+FORM_ACTION+"\" method=\""+FORM_ACTION+"\">\
      <table class=\"table mx-auto bg-light\" style=\"width: inherit\">\
        <thead class=\"thead-dark\">\
          <tr>\
            <th scope=\"col\">#</th>\
            <th scope=\"col\">Host</th>\
            <th scope=\"col\">Port</th>\
            <th scope=\"col\">Input File</th>\
          </tr>\
        </thead>\
        <tbody>";

for(int i=0;i<5;i++){
    pageContent+="\
          <tr>\
            <th scope=\"row\" class=\"align-middle\">Session "+to_string(i+1)+"</th>\
            <td>\
              <div class=\"input-group\">\
                <select name=\"h"+to_string(i)+"\" class=\"custom-select\">\
                  <option></option>"+host_menu+"\
                </select>\
                <div class=\"input-group-append\">\
                  <span class=\"input-group-text\">.cs.nctu.edu.tw</span>\
                </div>\
              </div>\
            </td>\
            <td>\
              <input name=\"p"+to_string(i)+"\" type=\"text\" class=\"form-control\" size=\"5\" />\
            </td>\
            <td>\
              <select name=\"f"+to_string(i)+"\" class=\"custom-select\">\
                <option></option>\
                "+test_case_menu+"\
              </select>\
            </td>\
          </tr>";
}

pageContent+="\
          <tr>\
            <td colspan=\"3\"></td>\
            <td>\
              <button type=\"submit\" class=\"btn btn-info btn-block\">Run</button>\
            </td>\
          </tr>\
        </tbody>\
      </table>\
    </form>\
  </body>\
</html>";
return pageContent;
}
};