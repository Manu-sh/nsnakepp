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
