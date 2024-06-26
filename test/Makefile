CXX = g++
CFLAGS = -std=c++20 -O2 -Wall -g 

TARGET = test
OBJS = ../log/*.cpp \
       ../buffer/*.cpp \
	   ./test.cpp \

all: $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o $(TARGET)  -pthread -lmysqlclient

clean:
	rm -rf ../bin/$(OBJS) $(TARGET)