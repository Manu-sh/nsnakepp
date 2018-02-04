#-march=native -p -g -Wall -Wextra

CXX=g++
CXXFLAGS=-D_GNU_SOURCE -pipe -O3 -ffast-math -std=c++11 `pkg-config --cflags ncursesw libcrypto`
LDFLAGS=`pkg-config --libs ncursesw libcrypto`

.PHONY: all clean

all: nsnake.cpp SnakeEngine.cpp Menu.cpp utils.hpp
	$(CXX) $(CXXFLAGS) -o nsnakepp nsnake.cpp $(LDFLAGS) 

clean:
	rm -f nsnakepp
