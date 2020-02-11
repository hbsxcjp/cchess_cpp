# �ο���C���Ժ��ļ�������20��
#vpath %.h src
#vpath %.cpp src
#vpath %.o obj

CC = g++
CFLAGS = -Wall -std=c++11 -fexec-charset=gbk #-g 
#LDFLAGS = -L/C/msys32/mingw32/lib -lpcre16 lib/pdcurses.a
SP = src/
OP = obj/
OBJS = $(OP)Tools.o $(OP)Piece.o $(OP)Seat.o $(OP)Board.o $(OP)ChessManual.o $(OP)main.o
JSONOBJ = $(OP)jsoncpp.o

a.exe: $(OBJS) $(JSONOBJ)
	$(CC) -Wall -o $@ $^ $(LDFLAGS) 
	
$(OBJS): $(OP)%.o : $(SP)%.cpp
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJS): $(SP)*.h

$(JSONOBJ): $(OP)%.o : $(SP)%.cpp
	$(CC) $(CFLAGS) -o $@ -c $<

#�������������滻Ϊ����.c�ļ���ͨ��-MԤ������������.h�ļ��ı䶯
#dependencies: $(OBJS:.o=.cpp)
#dependencies: $(SP)*.cpp
#	$(CC) -M $^ > $@
#
#include dependencies

.PHONY: clean
clean:
	rm a.exe obj/*.o