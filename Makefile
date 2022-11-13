# DO WHAT THE FUCK YOU WANT TO PUBLIC BEERWARE LICENSE v1.2
#
# Copyright (C) 2022 Jaroslav Prokop
#
# Everyone is permitted to copy and distribute verbatim or modified
# copies of this license document, and changing it is allowed as long
# as the name is changed.
#
# DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
# TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION
#
# 0. You just DO WHAT THE FUCK YOU WANT TO.
#
# 1. If we meet some day, and you think this stuff is worth it, you can buy me a beer in return.
#
# 2. THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
#	 INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
#	 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
#	 OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
#	 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR 
#	 IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

# Compiler for C++ 
CXX = g++

# Flags, that will turn on every error or warning
FLAGCMD ?=$(shell pkg-config --cflags libxml-2.0)
FLAGCMD!= pkg-config --cflags libxml-2.0
CFLAGS := -std=c++17 -static-libstdc++ -Wall -Wextra -Werror -pedantic $(FLAGCMD)

# Libraries used for DNS/TLS communication a parsing XML file
LIBCMD ?=$(shell pkg-config --libs libxml-2.0)
LIBCMD!=pkg-config --libs libxml-2.0
LIBS := -lcrypto -lssl $(LIBCMD)

# Build target
TARGET = feedreader
TEST = ./test/test.sh
DIRS = ./test/files/

all: clean feedreader

clean:
	rm -f feedreader *.o

feedreader: classes.o
	$(CXX) $(CFLAGS) -o feedreader ./src/feedreader.cpp $(LIBS)

classes.o:
	$(CXX) $(CFLAGS) -c ./src/classes.cpp

test: clean feedreader
	@bash $(TEST) --nodir

test_help:
	@bash $(TEST) --help

help: clean feedreader
	./feedreader -h
	
