#pragma once
#pragma GCC diagnostic ignored "-Wreorder"
#include <iostream>
#include <stdexcept>
#include <limits>
#include <initializer_list>
#include <signal.h>
#include <menu.h>

#include "utils.hpp"

/*
	you MUST call initscr() before create an Menu obj (and remember to call endwin())
	remember also to refresh your stdscr after that you have call Menu::render()
	ncurses use integer as argument of function like newwin, so you should use only types < INT_MAX
	*choice[] MUST TO BE NULL TERMINATED
*/

#define NO_RESIZE_EXCEPTION

namespace SMenu {

#ifndef NO_RESIZE_EXCEPTION
	volatile bool isResized;
	static void sighandler(int dummy) { isResized = true; }

	class resize_exception: public std::runtime_error {
		public: resize_exception(const char *msg): std::runtime_error(msg) {}
	};
#endif

	template <typename U = int> class Menu {

		static_assert(std::numeric_limits<U>::lowest() >= std::numeric_limits<int>::lowest() &&
				std::numeric_limits<U>::max()  <= std::numeric_limits<int>::max(), "template type too big");

		private:
			const char **choice;
			unsigned char choice_len;

			WINDOW *wmenu;
			U xsz, ysz, xstart, ystart;

		public:
			explicit Menu(const char *choice[], U xsz, U ysz, U xstart, U ystart);
			~Menu() { delwin(wmenu); }
			bool resize(U xsz, U ysz, U xstart, U ystart);
			unsigned char render(std::initializer_list<WINDOW *> toclr); // return the index of choice

	};

	template <typename U> Menu<U>::Menu(const char *choice[], U xsz, U ysz, U xstart, U ystart)
		: wmenu{NULL}, choice_len{0}, choice{choice}, xsz{xsz}, ysz{ysz}, xstart{xstart}, ystart{ystart}
	{
		if (!choice_len) for (;choice[choice_len];choice_len++);

		if (!(wmenu = newwin(xsz, ysz, xstart, ystart)))
			throw std::runtime_error("can't create a menu window");

		keypad(wmenu, TRUE), nodelay(wmenu, TRUE), wtimeout(wmenu, 0);
	}

	template <typename U> bool Menu<U>::resize(U xsz, U ysz, U xstart, U ystart) {

		if (wresize(wmenu, xsz, ysz) == ERR)
			return false;

		this->xsz = xsz;
		this->ysz = ysz;

		if (mvwin(wmenu, xstart, ystart) == ERR)
			return false;

		this->xstart = xstart;
		this->ystart = ystart;

		return true;
	}

	template <typename U> unsigned char Menu<U>::render(std::initializer_list<WINDOW *> toclr) {

		unsigned char i = 0;
		const char *cur = choice[i];

		// clear all windows on background
		for (auto w : toclr) wclear(w), wrefresh(w);

#ifndef NO_RESIZE_EXCEPTION
		SMenu::isResized = false;
		signal(SIGWINCH, sighandler);
#endif

		wrefresh(wmenu);
		for (int c; ((c = wgetch(wmenu)), true); wrefresh(wmenu)) {

#ifndef NO_RESIZE_EXCEPTION
			// TODO handle internally ? nxstart = xsz*nxsz/xstart
			if (isResized)
				throw resize_exception("handle this exception using SMenu::resize()");
#endif
		
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
				if (j == i) {
					wattron(wmenu, A_REVERSE);
					cur = choice[j];
				}

				mvwprintw(wmenu, j+1, 1, "%s", choice[j]);
				wattroff(wmenu, A_REVERSE);
			}
			napms(30);
		}

	}

#if 0
	int main() {

		const char *voices[] = { "zero", "uno", "due", "tre", NULL };
		unsigned char i = 0;

		initscr();
		Menu<unsigned char> m {voices, 40, 40, 10, 10};

		i = m.render({});
		refresh();
		std::cout << "\n => " << voices[i] << "\n";

		napms(1000);
		endwin();
		return 0;

	}
#endif
}
