/*
 * Daniel Nakhimovich
 *
 * Utility Header File that contains functions from
 * http://graphics.stanford.edu/~seander/bithacks.html
 *
 * Thanks Sean Eron Anderson!
 *
 */

#ifndef _UTIL_H
#define _UTIL_H

#include <cstdint>

inline uint32_t reverseBits(uint32_t b32) {
	// swap odd and even bits
	b32 = ((b32 >> 1) & 0x55555555) | ((b32 & 0x55555555) << 1);
	// swap consecutive pairs
	b32 = ((b32 >> 2) & 0x33333333) | ((b32 & 0x33333333) << 2);
	// swap nibbles ...
	b32 = ((b32 >> 4) & 0x0F0F0F0F) | ((b32 & 0x0F0F0F0F) << 4);
	// swap bytes
	b32 = ((b32 >> 8) & 0x00FF00FF) | ((b32 & 0x00FF00FF) << 8);
	// swap 2-byte long pairs
	b32 = ( b32 >> 16             ) | ( b32               << 16);
	return b32;
}

inline uint32_t numBits(uint32_t b32) {
	b32 = b32 - ((b32 >> 1) & 0x55555555);                      // reuse input as temp
	b32 = (b32 & 0x33333333) + ((b32 >> 2) & 0x33333333);       // temp
	b32 = (((b32 + (b32 >> 4)) & 0xF0F0F0F) * 0x1010101) >> 24; // count
	return b32;
}

static const int MultiplyDeBruijnBitPosition2[32] = {
	0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8,
	31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
};

inline uint32_t pow2log2(uint32_t b32) {
	return MultiplyDeBruijnBitPosition2[(uint32_t)(b32 * 0x077CB531U) >> 27];
}

#endif //_UTIL_H
