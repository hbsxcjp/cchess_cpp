# 参考《C语言核心技术》第20章
#vpath %.h src
#vpath %.cpp src
#vpath %.o obj

CC = g++
CFLAGS = -Wall -std=c++11 -fexec-charset=gbk #-g 
#LDFLAGS = -L/C/msys32/mingw32/lib -lpcre16 lib/pdcurses.a
SP = src/
OP = obj/
OBJS = $(OP)Tools.o $(OP)Piece.o $(OP)Seat.o $(OP)Board.o $(OP)ChessManual.o $(OP)Console.o $(OP)main.o
#OBJS = $(OP)Console.o $(OP)main.o
FIXEDOBJ = $(OP)jsoncpp.o # 固定的目标文件，一般只编译一次

a.exe: $(OBJS) $(FIXEDOBJ)
	$(CC) -Wall -o $@ $^ $(LDFLAGS) 
	
$(OBJS): $(OP)%.o : $(SP)%.cpp
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJS): $(OP)%.o : $(SP)%.h $(SP)ChessType.h

$(FIXEDOBJ): $(OP)%.o : $(SP)%.cpp
	$(CC) $(CFLAGS) -o $@ -c $<


#以下依赖条件替换为所有.c文件，通过-M预处理发现所有.h文件的变动
#dependencies: $(OBJS:.o=.cpp)
#dependencies: $(SP)*.cpp
#	$(CC) -M $^ > $@
#
#include dependencies

.PHONY: clean
clean:
	rm a.exe obj/*.o
