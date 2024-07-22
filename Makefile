CXX = g++
CFLAGS = -I/usr/include/python3.8 -std=c++14 -O2 -Wall -g 
OTHER = -pthread -lcurl -lmysqlcppconn

TARGET = server
OBJS =   webserver/*.cpp http/*.cpp ChatBot/*.cpp timer/*.cpp MySQL/*.cpp log/*.cpp main.cpp 

all: $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o $(TARGET) $(OTHER)

clean:
	rm -rf $(OBJS) $(TARGET)