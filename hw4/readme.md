


## error
* resolver 如果是在跟async_resolve的同一個func創建,因為async_resolve call 完之後這個resolver(離開scope)就會消失 call back function會找不到resovler
* 當ftp傳完資料的時候,如果沒有把socket close,會停住
* 傳送 ,接受封包的時候要注意,因為封包裡面有些null byte, 這邊要用char* ,因為如果用string,他會在null的地方截斷
並且注意要Print出來的時後要一個一個char的Print, 如果當成string 的方式print也會截斷

* after send exe file ,it seems not can't be execution


## 設定
flashfxp : 開啟本地端的視窗 view -> single connection layout
## coding:
bind的reply 回傳的是 socks隨機開的Port(等待dst連過來) ip就寫0000就行


## part1 (socks server)
1. make
2. ./socks_server 7777
3. set web browser to it
4. then it can connet any website

## part2
set ftp proxy(need ftp client has support proxy)

## part3  (這邊是寫socks client(傳socks 的request和handle回傳的socks reply))

1. run np_single_golden  in np_single/
2. ./socks_server 7778
2. /http_server 7777
3. web browser : localhost:7777/panel_socks.cgi
4. fill in table
## part4 firewall
socks.conf will block not permit connect



## 說明
* ftp activate mode:
https://matis.pixnet.net/blog/post/22918494
