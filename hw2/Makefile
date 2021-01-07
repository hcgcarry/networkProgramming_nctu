
OBJ:= npshell_simple.o np_simple.o passivesock.o
OBJtest:= npshell.o np_simple.o passivesock.o

OBJ2:=npshell_single_proc.o np_single_proc.o passivesock.o User.o UserPipe.o ForkedChildHandler.o
target1=np_simple
target2=np_single_proc


target1:$(OBJ)
	g++ -g -o $(target1) $(OBJ)

%.o:%.cpp
	g++ -g -c -o $@ $<


target2:$(OBJ2)
	g++ -g -o $(target2) $(OBJ2)

test:$(OBJtest)
	g++ -g -o $(target1) $(OBJtest)

run1:
	./np_simple 7001
run2:
	./np_single_proc 7001


clean:
	rm $(OBJ1) $(OBJ2) np_simple np_single_proc *.o || true