#pragma once

extern "C" {
	#include <unistd.h>
	#include <termios.h>
	#include <sys/ioctl.h>
}

template <typename U>
static void get_term_sz(U *rows, U *cols) noexcept {
	static struct winsize max;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &max);
	*rows = (U)max.ws_row;
	*cols = (U)max.ws_col;
}


// return a backup structure or null
static struct termios * set_max_baudrate(speed_t baud = B0) noexcept {

	static struct termios bak, newattr;

	if (tcgetattr(STDIN_FILENO, &bak) != 0)
		return NULL;

	if ((newattr = bak), baud == B0) {
		for (auto b : {B230400, B115200, B57600, B38400, B19200, B9600})
			if (cfsetspeed(&newattr, b) == 0) break;
	} else {
		if (cfsetspeed(&newattr, baud) != 0)
			return NULL;
	}

	if (tcsetattr(STDIN_FILENO, TCSANOW, &newattr) != 0)
		return NULL;

	return &bak;
}
