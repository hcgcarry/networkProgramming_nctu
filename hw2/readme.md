

## concurrent connection
* user 連上的給一個shell, shell都是在同一個process下 , 可互相溝通
1. make target1
2. ./np_simple 7777
3. another terminal : telnet localhost 7777
* 說明:user可以連上聊天室,並且user之間可以透過pipe倆倆互相溝通，也可以使用broadcase的方法與大家溝通


## single process 
* user 連上 就給fork一個新的 shell ,shell 之間不相干
1. make target2
2. ./np_single_proc 7777
3. another terminal: telnet localhost 7779
