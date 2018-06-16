#include <iostream>
#include <memory>
#include <csignal>
#include <locale>

extern "C" {
	#include <curses.h>
}

#include "core/SnakeEngine.cpp"

#include "utils.hpp"
#include "Menu.cpp"
#include "SaveGame.cpp"

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef SEngine<uchar,ushort> SnakeEngine;

#define BOARD_XSZ (ushort)20
#define BOARD_YSZ (ushort)40
#define WIN_XSZ BOARD_XSZ+5
#define WIN_YSZ BOARD_YSZ*2+5
#define WMENU_XSZ BOARD_XSZ
#define WMENU_YSZ BOARD_YSZ*2
#define WXOFFSET(TERM_XSZ) (TERM_XSZ/2 - BOARD_XSZ/2)
#define WYOFFSET(TERM_YSZ) (TERM_YSZ/2 - BOARD_YSZ)

#define MSG_END_OF_STAGE(_MSG_, _LV_, _SCORE_)		      \
do {							      \
	napms(300);					      \
	while (getch() != ERR);                               \
	clear();					      \
	printw("%s\n\tscore: %u\n\tlevel: ", _MSG_, _SCORE_); \
	for (uchar i = 1; i <= _LV_; i++)	              \
		printw("\u2605 ");			      \
	refresh();					      \
	while (getch() == ERR);			              \
	napms(500);					      \
} while(0)


#define RENDER(_BOARD_, _LV_)					           \
do {								           \
	std::wstring ws;				                   \
	ws += L"stage: ("  + std::to_wstring(_LV_)+L")";                   \
	ws += L"\tscore: " + std::to_wstring(_BOARD_->get_score());        \
	ws += L"\tfood: "  + std::to_wstring(_BOARD_->get_food()) + L"\n"; \
	mvaddwstr(0, 0, _BOARD_->to_wstr());                               \
	addwstr(ws.c_str());					           \
	refresh();						           \
} while(0)


struct termios *tty_reset;

/* 
unfortunately as per standard:
Terminating the program without leaving the current block (e.g., by calling the function
std::exit(int) (18.5)) does not destroy any objects with automatic storage duration (12.4).
*/

[[noreturn]] static void sighandler(int unused) {

	extern struct termios *tty_reset;
	endwin();

	if (tty_reset) 
		tcsetattr(STDIN_FILENO, TCSANOW, tty_reset);

	exit(0);
}

class JmpHandle {

	public:
		enum class Jmp: char {
			JMP_MV,      /* 0 - resume */
			JMP_NEWGAME, /* 1 - new game */
			JMP_EXIT,    /* 2 - exit */
			JMP_PAUSED
		};

		explicit JmpHandle(const Geom &gm) {
			m = std::unique_ptr<Menu>(new Menu(choice, gm));
		}

		Jmp recvkev() noexcept {

			int key = getch(); 
			while (getch() != ERR);

			switch(key) {
				case KEY_UP:    mov[0] = UP;    mov[1] = false; return Jmp::JMP_MV;
				case KEY_DOWN:  mov[0] = DOWN;  mov[1] = false; return Jmp::JMP_MV;
				case KEY_LEFT:  mov[0] = LEFT;  mov[1] = false; return Jmp::JMP_MV;
				case KEY_RIGHT: mov[0] = RIGHT; mov[1] = false; return Jmp::JMP_MV;

				case 'q': case ' ': case '\r': case '\n': /* display menu */
				{
					Jmp jmp = (Jmp)m->render({stdscr});
					return (mov[1] = (jmp == Jmp::JMP_MV)) ? Jmp::JMP_PAUSED :jmp;
				}
			}

			return mov[1] ? Jmp::JMP_PAUSED : Jmp::JMP_MV;
		}

		int recvmov() const noexcept { return mov[0]; }

	private:
		const char *choice[4] = { "Resume", "New game", "Exit", NULL };
		std::unique_ptr<Menu> m;
		int mov[2] = {-1, 1}; /* mov, isPaused? */

};

using std::unique_ptr;
using std::setlocale;

int main() {

	tty_reset = set_max_baudrate();
	signal(SIGINT, sighandler);
	setlocale(LC_ALL, "");
	initscr();

	extern int LINES, COLS;
	unsigned short term_xsz = LINES, term_ysz = COLS;

	wresize(stdscr, WIN_XSZ, WIN_YSZ); 
	mvwin(stdscr, WXOFFSET(term_xsz), WYOFFSET(term_ysz));

	keypad(stdscr, TRUE), nodelay(stdscr, TRUE), 
		curs_set(0), timeout(0), noecho(), cbreak();

_new_game:

	/*  destructors are called: http://en.cppreference.com/w/cpp/language/goto */
	SaveGame d;
	JmpHandle jh{ { WMENU_XSZ, WMENU_YSZ, WXOFFSET(term_xsz), WYOFFSET(term_ysz) } };
	unique_ptr<SnakeEngine> stage{new SnakeEngine(BOARD_XSZ, BOARD_YSZ, d.lv_food, d.score)};

	do {

		switch(jh.recvkev()) {
			case JmpHandle::Jmp::JMP_MV:
				break;
			case JmpHandle::Jmp::JMP_NEWGAME:
				d.delsavegame();
				goto _new_game;
			case JmpHandle::Jmp::JMP_EXIT:
				raise(SIGINT);
			case JmpHandle::Jmp::JMP_PAUSED:
				RENDER(stage, d.lv);
				napms(130);
				continue;
		}

		switch (stage->move(((Movement)jh.recvmov()))) {

			case GameStatus::NONE:
				RENDER(stage, d.lv);
				break;

			case GameStatus::LOST:
				RENDER(stage, d.lv);
				MSG_END_OF_STAGE("YOU LOST", d.lv, d.score);
				// d.delsavegame();
				raise(SIGINT);

			case GameStatus::WIN:
				RENDER(stage, d.lv);
				d.nextlevel();
				d.savegame();

				MSG_END_OF_STAGE("YOU WIN", d.lv, d.score);
				stage.reset(new SEngine<>(BOARD_XSZ, BOARD_YSZ, d.lv_food, d.score));
		}

		napms(d.speed);

	} while(1);

	return 0;
}
