/*
 * Daniel Nakhimovich
 *
 * cbb.cpp: (checkerboard bit board class)
 *   - implements constructor and display functions
 *   - implements functions for finding valid moves
 *   - implements functions for updating board with chosen moves
 *
 */
#include "cbb.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstring>

cbb cbb::lms[64];
cbb::board cbb::jumps[16][64];
uint32_t cbb::nlm;

cbb::cbb(const char sboard[32], int player, int time) {
	for (int i = 0; i < 32; i++) {
		switch (sboard[i]) {
			case '1':
				cb.w |= 1<<i;
				break;
			case '2':
				cb.b |= 1<<i;
				break;
			case '3':
				cb.w |= 1<<i;
				cb.k |= 1<<i;
				break;
			case '4':
				cb.b |= 1<<i;
				cb.k |= 1<<i;
				break;
		}
	}
	p = player;
	tl = time;
}



bool cbb::humanPickMove(uint32_t pick) {
	if (pick < nlm) {
		*this = lms[pick];
		return true;
	}
	return false;
}

time_t cbb::aiPickMove() {
	time_t start = time(NULL);
	genLegalMoves();
	time_t end = time(NULL);
	if (lms[0].cb.nonempty()) {
		*this = lms[0];
		return std::difftime(end,start);
	} else {
		return -1;
	}
}

void cbb::printcb() {
	std::cout << "\033[1;30;47m";
	for (int r = 0; r < 8; r++) {
		for (int c = 0; c < 4; c++) {
			if (r % 2 == 0 || c > 0) std::cout << "██████";
			std::cout << "  " << std::setw(2) << std::setfill('0') << (4*r+c) << "  ";
		}
		if (r % 2 != 0) std::cout << "██████";
		std::cout << "\n";
		for (int c = 0; c < 4; c++) {
			if (r % 2 == 0 || c > 0) std::cout << "██████";
			int ind = 4*r+c;
			if (cb.k>>ind & 1) std::cout << " ⟆";
			else std::cout << "  ";

			if (cb.w>>ind & 1) std::cout << "⊂⊃";
			else if (cb.b>>ind & 1) std::cout << "⋐⋑";
			else std::cout << "  ";

			if (cb.k>>ind & 1) std::cout << "⟅ ";
			else std::cout << "  ";
		}
		if (r % 2 != 0) std::cout << "██████";
		std::cout << "\n";
		for (int c = 0; c < 4; c++) {
			if (r % 2 == 0 || c > 0) std::cout << "██████";
			std::cout << "      ";
		}
		if (r % 2 != 0) std::cout << "██████";
		std::cout << "\n";
	}
	std::cout << "\033[0m";
}

uint32_t cbb::printlms() {
	for (int i = 0; i < 16; i++)
		for (uint32_t j = 1; j < nlm; j++)
			jumps[i][j] = {0,0,0};

	genLegalMoves();

	for (int i = 0; i < 16; i++) {
		for (uint32_t j = 0; j < nlm; j++) {
			if (j > 0 && !jumps[i][j].nonempty())
				jumps[i][j] = jumps[i][j-1];
			else
				if (p) flipboard(jumps[i][j]);
		}
	}
	for (uint32_t c = 0; c < nlm; c++) {
		uint32_t start;
		uint32_t end;
		uint32_t sxoe;

		if (p) {
			start = cb.b;
			end = lms[c].cb.b;
		} else {
			start = cb.w;
			end = lms[c].cb.w;
		}

		if (p) flipboard(cb);
		if (getWjumpers()) {
			if (p) flipboard(cb);

			int k = 0;
			uint32_t start2;

			if (p) {
				end = jumps[k][c].b;
			} else {
				end = jumps[k][c].w;
			}
			sxoe = start ^ end;
			start2 = end;
			start &= sxoe;
			end &= sxoe;
			std::cout << "[" << std::setw(2) << std::setfill('0') << c << "] ";
			std::cout << std::setw(2) << std::setfill('0') << std::log2(start);
			std::cout << " -> " << std::setw(2) << std::setfill('0') << std::log2(end);
			while (jumps[k][c] != lms[c].cb && k < 15) {
				k++;
				if (p) {
					end = jumps[k][c].b;
				} else {
					end = jumps[k][c].w;
				}
				sxoe = start2 ^ end;
				start2 = end;
				end &= sxoe;
				std::cout << " -> " << std::setw(2) << std::setfill('0') << std::log2(end);
			}
			std::cout << "\n";
		} else {
			if (p) flipboard(cb);

			sxoe = start ^ end;
			start &= sxoe;
			end &= sxoe;
			std::cout << "[" << std::setw(2) << std::setfill('0') << c << "] ";
			std::cout << std::setw(2) << std::setfill('0') << std::log2(start);
			std::cout << " -> " << std::setw(2) << std::setfill('0') << std::log2(end) << "\n";
		}

	}
	return nlm;
}
