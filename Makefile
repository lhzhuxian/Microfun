CC=g++49
SCL=-lc
C11=-std=c++11
CFLAGS= -gdwarf-2 -gstrict-dwarf -Wall -Wextra
ALL: client test
client: client.cpp common.hpp
	$(CC) -o client client.cpp $(SCL) $(C11) $(CFLAGS)
test: main.cpp connection.o common.hpp 
	$(CC) -o test main.cpp connection.o http_parser.o $(SCL) $(C11) $(CFLAGS)
connection.o: connection.cpp common.hpp http_parser.o http_parser.h
	$(CC) -c connection.cpp $(SCL) $(C11) $(CFLAGS)
clean:
	rm -f test connection.o client
