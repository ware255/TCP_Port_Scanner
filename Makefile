TARGET = tcp_port_scanner
CC     = g++
SRCS   = src/main.cpp
CFLAG  = -Wall

tcp_port_scanner:
	$(CC) $(SRCS) -o $(TARGET) $(CFLAG)

clean:
	rm -f $(TARGET)
