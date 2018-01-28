#include <iostream>
#include <cstdio>
#include <memory>
#include <curses.h>
#include <signal.h>

#include "utils.hpp"
#include "Menu.cpp"

#define BOARD_XSZ (unsigned short)20
#define BOARD_YSZ (unsigned short)40
#define WIN_XSZ BOARD_XSZ+5
#define WIN_YSZ BOARD_YSZ*2+5
#define WMENU_XSZ BOARD_XSZ
#define WMENU_YSZ BOARD_YSZ*2
#define WXOFFSET(TERM_XSZ) (TERM_XSZ/2 - BOARD_XSZ/2)
#define WYOFFSET(TERM_YSZ) (TERM_YSZ/2 - BOARD_YSZ)

#define MSG_END_OF_STAGE(_MSG_, _LV_, _SCORE_)		      \
do {							      \
	napms(300);					      \
	fflush(stdin);					      \
	clear();					      \
	printw("%s\n\tscore: %u\n\tlevel: ", _MSG_, _SCORE_); \
	for (unsigned char i = 1; i <= _LV_; i++)	      \
		printw("\u2605 ");			      \
	refresh();					      \
	while (getch() == ERR);			              \
	napms(500);					      \
} while(0)

#define RENDER(_BOARD_, _LV_)					                   \
	do {								           \
		std::wstring ws;				                   \
		ws += L"stage: ("  + std::to_wstring(_LV_)+L")";                   \
		ws += L"\tscore: " + std::to_wstring(_BOARD_->get_score());        \
		ws += L"\tfood: "  + std::to_wstring(_BOARD_->get_food()) + L"\n"; \
		mvaddwstr(0, 0, _BOARD_->to_wstr());                               \
		addwstr(ws.c_str());					           \
		refresh();						           \
	} while(0)


struct SaveGame {

	unsigned short speed, score;
	unsigned char lv_food, lv;

	SaveGame(unsigned short speed  = 150, unsigned short score  =   0,
		 unsigned char lv_food =   5, unsigned char lv      =   1) noexcept {

		if (!loadgame()) {
			this->speed   = speed;
			this->score   = score;
			this->lv_food = lv_food;
			this->lv      = lv;
		}

	}

	void nextlevel() noexcept {
		score   += lv_food;
		speed   -=(speed-2 > 0 ? 2 : 0); // ignore underflow, unreacheable for humans players
		lv_food +=(lv_food+5 < std::numeric_limits<unsigned char>::max() ? 5 : 0);
		lv++;
	}

	bool loadgame() noexcept { 

		FILE *file;
		unsigned short lv_food, lv;

		if (!(file = std::fopen("checkpoint.sav", "r")))
			return false;

		std::fscanf(file, "%hu\t%hu\t%hu\t%hu", &speed, &score, &lv_food, &lv);

		// workaround
		this->lv_food = (unsigned char)lv_food;
		this->lv      = (unsigned char)lv;

		std::fclose(file);
		return true;

	}

	bool savegame() {

		FILE *file;
		if (!(file = std::fopen("checkpoint.sav", "w+")))
			return false;

		std::fprintf(file, "%hu\t%hu\t%hu\t%hu", speed, score, lv_food, lv);
		std::fclose(file);
		return true;

	}

	void delsavegame() noexcept { remove("checkpoint.sav"); }
};

// ERR (from curses.h) if isn't a mv
static inline int curskeyAsMv(int key) noexcept {
	switch(key) {
		case KEY_UP:    return UP;
		case KEY_DOWN:  return DOWN;
		case KEY_LEFT:  return LEFT;
		case KEY_RIGHT: return RIGHT;
	}
	
	return ERR;
}

struct termios *tty_reset;

static void sighandler(int sig) {

	extern struct termios *tty_reset;
	endwin();

	if (tty_reset) 
		tcsetattr(STDIN_FILENO, TCSANOW, tty_reset);

	exit(0);
}

int main() {

	const char *choice[] = { "Resume", "New game", "Exit", NULL };
	unsigned short term_xsz = 0, term_ysz = 0;
	int mv = ERR, hold = ERR;
	bool pause = true;

	tty_reset = set_max_baudrate();
	signal(SIGINT, sighandler);
	get_term_sz(&term_xsz, &term_ysz);
	setlocale(LC_ALL, "");
	initscr();

	wresize(stdscr, WIN_XSZ, WIN_YSZ); 
	mvwin(stdscr, WXOFFSET(term_xsz), WYOFFSET(term_ysz));

	keypad(stdscr, TRUE), nodelay(stdscr, TRUE), 
	curs_set(0), timeout(0), noecho(), cbreak();

	start_color();
	init_pair(1, COLOR_CYAN, COLOR_BLACK);
	attron(COLOR_PAIR(1));

_new_game:

	SaveGame d;
	SMenu::Menu<> menu { choice, WMENU_XSZ, WMENU_YSZ, WXOFFSET(term_xsz), WYOFFSET(term_ysz) };
	std::unique_ptr<SEngine<>> stage(new SEngine<>(BOARD_XSZ, BOARD_YSZ, d.lv_food, d.score));

       	RENDER(stage, d.lv);
	refresh();

	for (volatile int c; ((c = getch()), true); napms(d.speed)) { 

		if ((hold = curskeyAsMv(c)) != ERR) {
			mv    = areOpposite((Movement)mv, (Movement)hold) ? mv : hold;
			pause = false;
			goto _move;
		} 

		if (c == ' ' || c == 'q' || c == '\r' || c == '\n') {

			pause = true;
			unsigned char i = menu.render({stdscr});
			RENDER(stage, d.lv);

			if (choice[i] == choice[1]) {
				d.delsavegame();
				pause = true;
				stage.reset(nullptr);
				goto _new_game;
			} else if (choice[i] == choice[2]) {
				goto _exit;
			}

		}

		if (pause) continue;
_move:
		switch (stage->move((Movement)mv)) {

			case GameStatus::NONE:

				RENDER(stage, d.lv);
				break;

			case GameStatus::LOST:

				RENDER(stage, d.lv);
				MSG_END_OF_STAGE("YOU LOST", d.lv, d.score);
				// d.delsavegame();
				goto _exit;

			case GameStatus::WIN:
				{
					RENDER(stage, d.lv);
					d.nextlevel();
					d.savegame();

					MSG_END_OF_STAGE("YOU WIN", d.lv, d.score);
					stage.reset(new SEngine<>(BOARD_XSZ, BOARD_YSZ, d.lv_food, d.score));
				}
				break;

		}

	}

_exit:
	refresh();
	endwin();
	tcsetattr(STDIN_FILENO, TCSANOW, tty_reset);
	return EXIT_SUCCESS;
}
