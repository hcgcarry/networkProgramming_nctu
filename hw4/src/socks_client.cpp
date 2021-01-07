#include"common.h"
class socks_client{
    public:
    char socks4Header[100];
    int socks4HeaderLen;
    socks_client(){}
    void makeSOCKS4Header(int DSTPort,string DSTIP,string domainName){
        socks4Header[0] = 4;
        socks4Header[1] = 1;
        socks4Header[2] = DSTPort/256;
        socks4Header[3] = DSTPort%256;
        socks4Header[4] = 0;
        socks4Header[5] = 0;
        socks4Header[6] = 0;
        socks4Header[7] = 1;
        socks4Header[8] = NULL;
        int i=9;
        for(auto item:domainName){
            socks4Header[i] = item;
            i++;
        }
        socks4Header[i] = NULL;
        socks4HeaderLen = i;
    }
    void printSocks4Header(){
        cout << "-------------socks4Header----------" << endl;
        for(int i=0;i<socks4HeaderLen;i++){
            printf("%d ",socks4Header[i]);
        }
        cout << endl;
        cout << "-------------socks4Headerend----------" << endl;
    }
    bool replyIsSuccess(char* reply){
        if(reply[1] == 90){
            return true;
        }
        else{
            return false;
        }
    }
};