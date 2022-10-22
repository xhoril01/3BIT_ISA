# Compiler for C++ 
CXX = g++

# Flags, that will turn on every error or warning
CFLAGS = -Wall -Wextra -Werror -pedantic -I /usr/include/libxml2/

# Libraries used for DNS/TLS communication a parsing XML file
LIBS= -lcrypto -lssl -lxml2

# Build target
TARGET = feedreader

all: clean feedreader

clean:
	rm -f feedreader *.o

feedreader: classes.o
	$(CXX) $(CFLAGS) -o feedreader ./src/feedreader.cpp $(LIBS)

classes.o:
	$(CXX) $(CFLAGS) -c ./src/classes.cpp

test_help:
	