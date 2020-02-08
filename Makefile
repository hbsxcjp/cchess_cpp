# 参考《C语言核心技术》第19章
vpath %.h src
vpath %.cpp src
vpath %.o obj

CC = g++
CFLAGS = -Wall -std=c++11  -fexec-charset=gbk  -iquote # -g 
#LDFLAGS = -L/C/msys32/mingw32/lib -lpcre16 lib/pdcurses.a
P = ./
PO = obj/
OBJS = $(PO)jsoncpp.o $(PO)Tools.o $(PO)Piece.o $(PO)Seat.o $(PO)Board.o $(PO)ChessManual.o $(PO)main.o

a.exe: $(OBJS)
	$(CC) -Wall -o $@ $^ $(LDFLAGS) 
	
$(OBJS): $(PO)%.o : $(P)%.cpp
	$(CC) $(CFLAGS) -o $@ -c $<

#$(OBJS): $(P)*.h
#dependencies: $(OBJS: .o=.c)
#	$(CC) -M $^ > $@\
#
#include dependencies

.PHONY: clean
clean:
	rm a.exe obj/*.o
