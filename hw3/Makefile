CXX=g++
CXXFLAGS=-std=c++14 -Wall -pedantic -lpthread -lboost_system -g
CXX_INCLUDE_DIRS=include
CXX_INCLUDE_PARAMS=$(addprefix -I , $(CXX_INCLUDE_DIRS))
CXX_LIB_DIRS=/usr/local/lib
CXX_LIB_PARAMS=$(addprefix -L , $(CXX_LIB_DIRS))



#############################linux
target1 = http_server
target2 = console.cgi
obj_dir = obj
src_dir = src
obj_source_http_server = http_server.o util.o env.o
obj_source_console = printHTML.o console.o  clientSession.o serverInfo.o env.o

part1:clean $(target2) $(target1)

$(target2):$(obj_source_console)
	$(CXX) $^ -o $@ $(CXXFLAGS)
$(target1):$(obj_source_http_server)
	$(CXX) $^ -o $@ $(CXXFLAGS)
%.o:%.cpp
	$(CXX) $< -c -o $@ $(CXX_INCLUDE_PARAMS) $(CXX_LIB_PARAMS) $(CXXFLAGS)
#$(obj_dir)/%.o:$(src_dir)/%.cpp
#	$(CXX) $< -c -o $@ $(CXX_INCLUDE_PARAMS) $(CXX_LIB_PARAMS) $(CXXFLAGS)

clean:
	rm -f $(target1) $(target2) *.o || true

run:
	./http_server 7777
#########################################windows
win_CXXFLAGS=-std=c++14 -lws2_32 -lwsock32 -g 
win_CXX_INCLUDE_FLAG=-Iwindows\include

obj_dir=obj
windows_dir = windows
windows_server_src = $(wildcard $(windows_dir)/*.cpp)
windows_server_obj = $(windows_server_src:$(windows_dir)/%.cpp=$(obj_dir)/%.o)

#windows_server_obj = obj/http_server.o obj/panel.o obj/printHTML.o obj/serverInfo.o obj/console.o obj/clientSession.o  obj/env.o
windows_target = cgi_server.exe


.PHONY: test


#test: ; $(info $$windows_server_obj is [${windows_server_obj}])echo Hello world
#test1: ; $(info $$windows_server_src is [${windows_server_src}])echo Hello world


part2:$(windows_target)

$(windows_target):$(windows_server_obj)
	$(CXX) $^ -o $@ $(win_CXXFLAGS) $(win_CXX_INCLUDE_FLAG)

$(obj_dir)/%.o:$(windows_dir)/%.cpp
	$(CXX) $< -c -o $@ $(win_CXXFLAGS) $(win_CXX_INCLUDE_FLAG)

cleanWindows:
	del $(obj_dir)\*.o 

runWindows:
	./cgi_server.exe 7777