#pragma once
#include <iostream>
#include <limits>
#include <cstring>
#include <stdexcept>
#include <initializer_list>

extern "C" {
	#include <signal.h>
	#include <menu.h>
}

/*
	this is a trivial class to provide a simple menu for nsnake:

	1) isn't manage resize operation
	2) choice len must to be a reasonable value

	you MUST call initscr() before create an Menu obj (and remember to call endwin())
	remember also to refresh your stdscr after that you have call Menu::render()
	*choice[] MUST TO BE NULL TERMINATED
*/

struct Geom { int xsz, ysz, xoff, yoff; };

class Menu {

	public:
		Menu(const char *const *const choice, const Geom &gm);
		Menu(const Menu &) = delete;
		Menu(Menu &&)      = delete;
		~Menu() { delwin(wmenu); }

		Menu & operator=(const Menu &) = delete;
		Menu & operator=(Menu &&)      = delete;

		bool resize(const Geom &gm) noexcept;
	
		/* toclr: all windows on background that should be clear when the menu is displayed
		the return value is the index of the element selected into menu by user:
		const char *choice[] = { "ONE", "TWO", "THREE", NULL }; if the user select "THREE",
		render() return his index that is 2  */

		unsigned char render(std::initializer_list<WINDOW *> toclr = {}) noexcept; 

	private:
		const char *const *const choice{};
		unsigned char choice_len{};

		WINDOW *wmenu{};
		Geom gm;

};

Menu::Menu(const char *const *const choice, const Geom &gm): choice{choice}, gm(gm) {

	if (!choice_len) while(choice[choice_len]) choice_len++;

	if (!(wmenu = newwin(gm.xsz, gm.ysz, gm.xoff, gm.yoff)))
		throw std::runtime_error("can't create a menu window");

	keypad(wmenu, TRUE), nodelay(wmenu, TRUE), wtimeout(wmenu, 0);
}


bool Menu::resize(const Geom &gm) noexcept {

	if (wresize(wmenu, gm.xsz, gm.ysz) == ERR)
		return false;

	this->gm.xsz = gm.xsz;
	this->gm.ysz = gm.ysz;

	if (mvwin(wmenu, gm.xoff, gm.yoff) == ERR)
		return false;

	this->gm.xoff = gm.xoff;
	this->gm.yoff = gm.yoff;

	return true;
}

unsigned char Menu::render(std::initializer_list<WINDOW *> toclr) noexcept {

	unsigned char i = 0;

	// clear all windows on background
	for (auto w : toclr)
	       	wclear(w), wrefresh(w);

	wrefresh(wmenu);
	for (int c; ((c = wgetch(wmenu)), true); wrefresh(wmenu)) {

		switch(c) {
			case KEY_UP:
				i = i > 0 ? i-1 : i;
				break;
			case KEY_DOWN:
				i = i < choice_len-1 ? i+1 : i;
				break;
			case KEY_ENTER:
			case '\n':
			case '\r':
				wclear(wmenu);
				wrefresh(wmenu);
				return i;
		}

		box(wmenu, 0, 0);
		for (int j = 0; j < choice_len; j++) {
			if (j == i) wattron(wmenu, A_REVERSE);

			mvwprintw(wmenu, j+1, 1, "%s", choice[j]);
			wattroff(wmenu, A_REVERSE);
		}
		napms(30);
	}

       	/* unreacheable */
	return UCHAR_MAX;
}

#if 0
int main() {

	const char *voices[] = { "zero", "uno", "due", "tre", NULL };
	unsigned char i = 0;

	initscr();
	Menu m {voices, {40, 40, 10, 10}};

	i = m.render();
	refresh();
	std::cout << "\n => " << voices[i] << "\n";

	napms(1000);
	endwin();
	return 0;

}
#endif
