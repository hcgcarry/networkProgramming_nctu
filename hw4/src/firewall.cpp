#include "common.h"
#include <fstream>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

class firewall
{
public:
    firewall() {}
    static bool checkIFPassFireWall(string command, string ip)
    {
        fstream confFile;
        confFile.open("socks.conf", ios::in);
        string line;
        bool permitFlag = false;
        vector<string> ipSplitResult;
        boost::split(ipSplitResult, ip, boost::is_any_of("."), boost::token_compress_on);
        while (getline(confFile, line))
        {
            regex ruleRegex("permit ([cb]) (\\d+|\\*)\\.(\\d+|\\*)\\.(\\d+|\\*)\\.(\\d+|\\*)");
            smatch ruleSm;
            regex_match(line, ruleSm, ruleRegex);
            string rule_command;
            if ((ruleSm[1].str() == "c" && command == "CONNECT" )||(ruleSm[1].str() == "b" && command == "BIND"))
            {
                int permitCount = 0;
                for(int i=2;i<6;i++){
                    if (ruleSm[i].str() == "*" || ruleSm[i].str() == ipSplitResult[i-2]){
                        permitCount ++;
                    }
                }
                if(permitCount == 4){
                    permitFlag = true;
                    break;
                }
            }
        }
        return permitFlag;
    }
};