#pragma once
#include <stdexcept>
#include <vector>
#include <limits>

#include "../utils.hpp"
#include "Cells.hpp"

enum Movement : signed char { UP, DOWN, LEFT, RIGHT };
enum class GameStatus : char { NONE, WIN, LOST };

static bool inline areOpposite(Movement a, Movement b) {
	return ((a == UP && b == DOWN) || (a == DOWN && b == UP)) 
		|| ((a == LEFT && b == RIGHT) || (a == RIGHT && b == LEFT));
}

/* https://en.wikipedia.org/wiki/Sprite_(computer_graphics)
simulate a SPRITE: If you want optimize you can create (for example) a friend method that create 
2 ncurses windows the first fixed as a background, then you could use a second window for print only
the snake and food; infact replace snake and food into buf isn't an easy way beacause the buffer 
contains also other character like newline and borders */

template <typename U = unsigned char, typename UU = unsigned short>
class SEngine {

	static_assert(sizeof(UU) > sizeof(U), "generic typename UU is <= U, this can be dangerous");
	static_assert(!std::is_signed<U>::value && !std::is_signed<UU>::value, "unsigned values only");
	using Cell = Cells<U>;

	private:
		const static signed char offset[4][2];
		size_t length;
		wchar_t *buffer;
		std::vector<std::vector<Cell>> board;
		std::vector<Cell *> snake;
		Movement prv_mv = (Movement)-1;

		U xsz, ysz, food;
		UU score;

	public:
		explicit SEngine(U xsz, U ysz, U food, UU score);
		~SEngine() { free(buffer); }

		GameStatus move(Movement mv);
		UU get_score() const { return score; }
		UU get_food()  const { return food;  }
	
		const wchar_t * to_wstr(bool isFirstCall = false);

	private:
		bool isInvalidMovement(Cell &head, Movement mv) {
			return  head.X()+offset[mv][0] <= -1 || head.X()+offset[mv][0] >= xsz || 
				head.Y()+offset[mv][1] <= -1 || head.Y()+offset[mv][1] >= ysz;
		}

};

template <typename A, typename B>
const signed char SEngine<A,B>::offset[4][2] = { { -1, 0 }, { 1, 0 }, { 0, -1 }, { 0, 1 } };

template <typename U, typename UU>
SEngine<U,UU>::SEngine(U xsz, U ysz, U food, UU score) 
	: xsz{xsz}, ysz{ysz}, food{food}, score{score}, buffer{NULL} {

	bool flag = true;

	if (xsz < 9 || ysz < 9)
		throw std::runtime_error("board size too small");

	if (food <= 0)
		throw std::runtime_error("food must to be at least 1");

	board.resize(xsz);
	for (U x = 0; x < xsz && (board[x].resize(ysz), true); x++) {
		for (U y = 0; y < ysz; y++) {
			board[x][y].set_x(x);
			board[x][y].set_y(y);
			board[x][y].set_ctype(NONE);

			if (flag && x != xsz/2 && y != ysz/2 && ysz != ysz/2+1 && random_range((U)1, ysz) < 2) {
				board[x][y].set_ctype(FOOD);
				flag = false;
				food--;
			}	
		}
	}

	snake.resize(2);
	snake[0] = &board[xsz/2][ysz/2].set_ctype(SNAKE);
	snake[1] = &board[xsz/2][ysz/2+1].set_ctype(SNAKE);
	prv_mv   = LEFT; // default orientation
	to_wstr(true);

}

template <typename U, typename UU>
GameStatus SEngine<U,UU>::move(Movement mv) {

	// Oooo => oOoo (you can't do this)
	if (areOpposite(prv_mv, mv))
		return GameStatus::NONE;

	if (isInvalidMovement(*snake[0], mv))
		return GameStatus::LOST;

	std::vector<Cell *> cp {snake};
	snake[0] = &board[snake[0]->X()+offset[mv][0]][snake[0]->Y()+offset[mv][1]]; // set the new snake head

	if (snake[0]->isSnake())
		return GameStatus::LOST;

	// move the snake
	for (U i = 1; i < snake.size(); i++) {
		snake[i]->set_ctype(NONE);
		snake[i] = cp[i-1];
		snake[i]->set_ctype(SNAKE);
	}

	if (snake[0]->isFood()) {

		snake.push_back(&board[snake.back()->X()][snake.back()->Y()]);
		snake.back()->set_ctype(SNAKE);
		score++;

		// add new food
		for (U x, y;;) {

			x = random_range((U)0, (U)(xsz-1));
			y = random_range((U)0, (U)(ysz-1));

			// the type of snake[0] is set later so you can't check it with isSnake()
			if (!board[x][y].isSnake() && board[x][y] != *snake[0]) { 
				board[x][y].set_ctype(FOOD);
				break;
			}

		}

		if (--food == 0)
			return GameStatus::WIN;
	}

	// must to be the last operation
	snake[0]->set_ctype(SNAKE);
	prv_mv = mv;

	return GameStatus::NONE;

}

/* horrible but fast, however should be rewritten, we draw borders and newline only the first time
to avoid making too many calls to I/O functions we write on memory and for the same reason (overhead) we don't call
std functions like wstringstream */

template <typename U, typename UU> 
const wchar_t * SEngine<U,UU>::to_wstr(bool isFirstCall) {

#define ISHEAD(N) ((N == *snake[0]))

	size_t k = ((ysz+1) << 1)+3;
	U x, y;

	if (isFirstCall) goto _draw_all;

	for (x = 0; x < xsz; x++) {
		for (y = 0; y < ysz; y++) {

			if (y == 0) k+=2;

			if (ISHEAD(board[x][y])) {
				buffer[k++] = L'\u2588';
				buffer[k++] = L'\u2588';
			} else if (board[x][y].isSnake()) {
				buffer[k++] = L'\u2588';
				buffer[k++] = L'\u2588';
			} else if (board[x][y].isFood()) {
				buffer[k++] = L'\u25cf';
				buffer[k++] = L' ';
			} else {
				buffer[k++] = L' ';
				buffer[k++] = L' ';
			}

			if (y == ysz-1) k+=3;
		}
	}
	return buffer;

_draw_all:

	// we need of +2 extra rows and cols for borders: (xsz+2)*(ysz+2)
	// we use 2 character for representing each element: (ysz+2)*(ysz+2)*2
	// we have one newline for each row (including borders): +(xsz+2)

	// with xsz=20, ysz=40:  22*42*2+22

#define LENGTH(_XSZ_,_YSZ_) (((_XSZ_+2)*(_YSZ_+2) << 1) + _XSZ_ + 2)

	// big buf for wstring representation, + 1 for L'\0'
	this->buffer = (wchar_t *)malloc( ((this->length = LENGTH(xsz, ysz)) + 1) * sizeof(wchar_t));
#undef LENGTH

	// 2 * (ysz+1) + 2+1
	for (k = 0, y = 0; y <= ysz; y++) {
		buffer[k++] = L'\u2592';
		buffer[k++] = L'\u2592';
	}

	buffer[k++] = L'\u2592';
	buffer[k++] = L'\u2592';
	buffer[k++] = L'\n';

	// xsz*ysz*2 + (xsz*(2+2+1))
	for (x = 0; x < xsz; x++) {
		for (y = 0; y < ysz; y++) {

			if (y == 0) {
				buffer[k++] = L'\u2592';
				buffer[k++] = L'\u2592';
			}

			k+=2;

			if (y == ysz-1) {
				buffer[k++] = L'\u2592';
				buffer[k++] = L'\u2592';
				buffer[k++] = L'\n';
			}
		}
	}

	for (y = 0; y <= ysz; y++) {
		buffer[k++] = L'\u2592';
		buffer[k++] = L'\u2592';
	}

	buffer[k++] = L'\u2592';
	buffer[k++] = L'\u2592';
	buffer[k++] = L'\n';
	buffer[k]   = L'\0';
	return buffer;

	// 2*41+2+1 + 40*20*2 + (20*(2+2+1)) + 2*41+2+1 = 1870
	// +1 (newline) final *2 is because every cell is represented with 2 character
	// throw std::runtime_error( " should be equal: "+std::to_string(k)+" == "+ std::to_string(length));
#undef ISHEAD
}

#if 0
// reassuring but slow
template <typename U, typename UU> 
std::wstring SEngine<U,UU>::to_wstr() const {

#define ISHEAD(N) ((N == *snake[0]))

	U x, y;
	std::wstringstream ws;
	const static wchar_t *blk = L"\u2592\u2592";

	for (y = 0; y <= ysz; y++)
		ws << blk;
	ws << blk << L'\n';

	for (x = 0; x < xsz; x++) {
		for (y = 0; y < ysz; y++) {

			if (y == 0) ws << blk; 

			if (ISHEAD(board[x][y]))
				ws << L"\u2588\u2588";
			else if (board[x][y].isSnake())
				ws << L"\u2588\u2588";
			else if (board[x][y].isFood())
				ws << L"\u25cf ";
			else
				ws << L"  ";

			if (y == ysz-1)
				ws << blk << L'\n';
		}
	}

	for (y = 0; y <= ysz; y++)
		ws << blk;
	ws << blk << L'\n';

#undef ISHEAD
	return ws.str();
}
#endif
