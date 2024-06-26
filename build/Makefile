CXX = g++
CFLAGS = -std=c++20 -O2 -Wall -g 

TARGET = server
  OBJS = ../log/*.cpp \
		 ../buffer/*.cpp \
		 ../pool/*.cpp \
		 ../main.cpp \

all: $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o ./bin/$(TARGET)  -pthread -lmysqlcppconn

clean:
	rm -rf ./bin/$(OBJS) $(TARGET)
