CC=g++49
SCL=-lc
C11=-std=c++11
CFLAGS= -gdwarf-2 -gstrict-dwarf -Wall -Wextra
THREAD= -pthread
ALL: client test connection.o request.o
client: client.cpp 
	$(CC) -o client client.cpp $(SCL) $(C11) $(CFLAGS)
test: main.cpp connection.o request.o
	$(CC) -o test main.cpp connection.o request.o http_parser.o  $(SCL) $(C11) $(CFLAGS) $(THREAD)
connection.o: connection.cpp
	$(CC) -c connection.cpp $(SCL) $(C11) $(CFLAGS)
request.o: request.cpp
	$(CC) -c request.cpp $(SCL) $(C11) $(CFLAGS)
clean:
	rm -f test connection.o client request.o
