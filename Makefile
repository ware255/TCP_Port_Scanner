TARGET = tcp_port_scanner
SRCS   = main.cpp
CC     = g++

tcp_port_scanner:
    $(CC) $(SRCS) -o $(TARGET) -Wall

clean:
    rm -fr $(TARGET)
