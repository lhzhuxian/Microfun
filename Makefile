CC=g++49
SCL=-lc
C11=-std=c++11
CFLAGS= -gdwarf-2 -gstrict-dwarf -Wall -Wextra
test: main.cpp
	$(CC) -o test main.cpp $(SCL) $(C11) $(CFLAGS)
clean:
	rm -f test
