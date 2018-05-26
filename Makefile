#MPX=-mmpx -fcheck-pointer-bounds
CXXFLAGS=-D_GNU_SOURCE -Wall -Wextra -pedantic -ansi -pipe -O3 -ffast-math -std=c++11 `pkg-config --cflags ncursesw libcrypto` $(MPX)
LDLIBS=`pkg-config --libs ncursesw libcrypto`

.PHONY: all clean

all: nsnake.cpp core/SnakeEngine.cpp core/random.hpp Menu.cpp SaveGame.cpp utils.hpp
	$(CXX) $(CXXFLAGS) -o nsnakepp nsnake.cpp $(LDLIBS) 

clean:
	rm -f nsnakepp
