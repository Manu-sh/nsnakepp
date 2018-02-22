# -Wall -Wextra -pedantic
CXXFLAGS=-D_GNU_SOURCE -pipe -O3 -ffast-math -std=c++11 `pkg-config --cflags ncursesw libcrypto`
LDLIBS=`pkg-config --libs ncursesw libcrypto`

.PHONY: all clean

all: nsnake.cpp core/SnakeEngine.cpp core/random.hpp Menu.cpp SaveGame.cpp utils.hpp
	$(CXX) $(CXXFLAGS) -o nsnakepp nsnake.cpp $(LDLIBS) 

clean:
	rm -f nsnakepp
