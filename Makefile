INCLUDE_OPT = -I /usr/local/include
LIB_OPT = -L /usr/local/lib
all:
	g++ -c Stdafx.cpp $(INCLUDE_OPT) -g
	g++ -c TestLua.cpp $(INCLUDE_OPT) -g
	g++ *.o -o TestLua.x -llua $(LIB_OPT) -g
