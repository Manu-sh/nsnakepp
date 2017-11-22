#pragma once
#include <iostream>

enum Content : signed char { NONE=0, FOOD=50, BLOCKED=99, SNAKE=127 };

template <typename U> class Cells {

	static_assert(!std::is_signed<U>::value, "unsigned values only");

	private:
		U x, y;
		Content ctype;

	public: 
		Cells() {};
		explicit Cells(U x, U y) : x{x}, y{y} { }

		bool operator==(const Cells<U> &c1) const { return x == c1.X() && y == c1.Y(); }
		bool operator!=(const Cells<U> &c1) const { return !operator==(c1); }

		Cells & set_ctype(Content ctype) { return this->ctype = ctype, *this; }
		void set_x(U x) { this->x = x; }
		void set_y(U y) { this->y = y; }

		Content get_ctype() const { return ctype; }
		bool isFood()  const { return ctype == FOOD; }
		bool isSnake() const { return ctype == SNAKE; }
		bool isEmpty() const { return ctype == NONE; }

		U X() const { return x; }
		U Y() const { return y; }

		template <typename T> 
			friend std::ostream & operator<<(std::ostream &out, const Cells<T> &c);

};

template <typename U> std::ostream & operator<<(std::ostream &out, const Cells<U> &c) {
	return out << "[ " << c.x << ", " << c.y << " ] (" << ((c.ctype == NONE) ? "NONE" : (c.ctype == FOOD) ? "FOOD" : (c.ctype == BLOCKED) ? "BLOCKED" : (c.ctype == SNAKE) ? "SNAKE" : nullptr) << ")";
}
