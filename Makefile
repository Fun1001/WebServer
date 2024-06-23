
CXX = g++
CFLAGS = -std=c++20 -O2 -Wall -g 

TARGET = server
OBJS = ./log/*.cpp ./buffer/*.cpp ./main.cpp

all:
	mkdir -p bin 
	$(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o ./bin/$(TARGET)  -pthread -lmysqlclient

clean:
	rm -rf ./bin/$(OBJS) $(TARGET)