CXX=g++
CXXFLAGS=-std=c++14 -Wall -pedantic -lpthread -lboost_system -g
CXX_INCLUDE_DIRS=include
CXX_INCLUDE_PARAMS=$(addprefix -I , $(CXX_INCLUDE_DIRS))
CXX_LIB_DIRS=/usr/local/lib
CXX_LIB_PARAMS=$(addprefix -L , $(CXX_LIB_DIRS))



#############################linux
target1 = socks_server
target2 = hw4.cgi
obj_dir = obj
src_dir = src
obj_socks_server = $(addprefix $(obj_dir)/, socks_server.o firewall.o)
obj_console = $(addprefix $(obj_dir)/, printHTML.o console.o  clientSession.o serverInfo.o env.o)

part1:dir $(target2) $(target1) 

$(target1):$(obj_socks_server)
	$(CXX) $^ -o $@ $(CXXFLAGS)
$(target2):$(obj_console)
	$(CXX) $^ -o $@ $(CXXFLAGS)
#%.o:%.cpp
#	$(CXX) $< -c -o $@ $(CXX_INCLUDE_PARAMS) $(CXX_LIB_PARAMS) $(CXXFLAGS)
$(obj_dir)/%.o:$(src_dir)/%.cpp
	$(CXX) $< -c -o $@ $(CXX_INCLUDE_PARAMS) $(CXX_LIB_PARAMS) $(CXXFLAGS)

dir:
	mkdir -p obj
clean:
	rm -f $(target1) $(target2) obj/* || true
