#pragma once
#include <cmath>

extern "C" {
	#include <unistd.h>
	#include <termios.h>
	#include <sys/ioctl.h>
	#include <openssl/rand.h>
}

#define MIN(A,B) (((A) < (B)) ? (A) : (B))
#define MAX(A,B) (((A) > (B)) ? (A) : (B))

template <typename U>
static U rand_in_range(U from, U to) noexcept {
	static U rbuf;
	RAND_bytes((unsigned char *)&rbuf, (int)sizeof rbuf);
	return (((U)std::abs(rbuf)) % (to-from+1)) + from;
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
