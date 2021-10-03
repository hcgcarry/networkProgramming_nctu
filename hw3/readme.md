# there has linux version and windows version of remote batch system

## linux version
### build and run
1. make part1
2. ./http_server 7777
3. open web browser and type localhost:7777/panel.cgi
###
source code is under ./
### clean
make clean

## 說明
* smart pointer:因為IO 使用到async 的方法,所以IO呼叫完程式就離開這個程式了，系統可能會判斷這個object不會再被用到,所以刪掉這個object,這樣等async
call back function被觸發,裡面用到該object的member的時候就會出問題
http_server 負責處理網址並且
* 使用C++ smart pointer和boost library的async,使聊天室的I/O操作都變成asynchronous的形式來達成linux的select()的single-process concurrent效果，並且使其相容於windows的socket
## bin 功能說明
* http_server : 負責accept 瀏覽器的get 並且fork exec 相對應的cgi
* console.cgi: 負責read batch 然後讓np_single_golden(之前hw2 開發的聊天室)執行,執行完把執行完輸出的結果寫回瀏覽器

## windows version 
### build and run
0. environment mingw
1. make part2
2. ./http_server 7777
3. open web browser and type localhost:7777/panel.cgi
### 
source code is in windows/
### clean
make cleanWindows
