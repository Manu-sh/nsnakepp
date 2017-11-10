#-march=native -p -g -Wall -Wextra

CXX=g++
CXXFLAGS=-D_GNU_SOURCE -pipe -O3 -ffast-math -std=c++11 
LDFLAGS=-lcrypto -lncursesw -ltinfo

# there are some problems on rpi with raspbian
CURSW_H=/usr/include/ncursesw/curses.h
ifneq ($(wildcard $(CURSW_H)),)
	CXXFLAGS+=-include $(CURSW_H)
endif

all: nsnake.cpp SnakeEngine.cpp Menu.cpp Cells.hpp utils.hpp
	$(CXX) $(CXXFLAGS) -o nsnake nsnake.cpp $(LDFLAGS)
