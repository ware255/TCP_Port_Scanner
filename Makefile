tcp_port_scanner:
    g++ src/main.cpp -o tcp_port_scanner -Wall

clean:
    rm -fr $(TARGET)
