## debug
* if we didn't clean the redundant pipes fd ,other child which read from this pipe will not exit,
because it think it still have stdin will come in

* argv ,envp need terminated by NULL , or it will let execve error

* if cin read EOF , cin >> num;
it will return -1,but not store -1 in num 
so correct usage:if((cin >> num )) < 0 ){} 


## build
* you need to recompile program in bin with command folder .cpp file
* make
* ./npshell

