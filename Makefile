tcp_port_scanner:
	g++ src/main.cpp -o tcp_port_scanner

clean:
	rm -f tcp_port_scanner
