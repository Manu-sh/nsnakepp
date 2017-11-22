#-march=native -p -g -Wall -Wextra

CXX=g++
CXXFLAGS=-D_GNU_SOURCE -pipe -O3 -ffast-math -std=c++11
LDFLAGS=-lcrypto -lncursesw -ltinfo
.PHONY: all clean dlink_ver mtx_ver

# workaround for raspbian
CURSW_H=/usr/include/ncursesw/curses.h
ifneq ($(wildcard $(CURSW_H)),)
	CXXFLAGS+=-include $(CURSW_H)
endif

all: dlink_ver

dlink_ver: nsnake.cpp dlink_ver/SnakeEngine.cpp Menu.cpp utils.hpp
	$(CXX) $(CXXFLAGS) -o nsnakepp nsnake.cpp $(LDFLAGS) -include 'dlink_ver/SnakeEngine.cpp'

mtx_ver: nsnake.cpp mtx_ver/SnakeEngine.cpp mtx_ver/Cells.hpp Menu.cpp utils.hpp
	$(CXX) $(CXXFLAGS) -o nsnakepp nsnake.cpp $(LDFLAGS) -include 'mtx_ver/SnakeEngine.cpp'

clean:
	rm -f nsnakepp
