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
int cbb::nlm;
cbb cbb::stack[2048];
cbb cbb::bnm;
cbb::board cbb::jumps[16][64];

cbb::cbb(const char sboard[32], int player) {
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
}

inline int cbb::score(int player) {
	uint32_t pb = player ? cb.b : cb.w;
	uint32_t ob = player ? cb.w : cb.b;
	return numBits(pb)-numBits(ob);
}

int *cbb::aiPickMove(int timeLimit) {
	int i, nnlm, sind, dd = 0;
	unsigned long d, maxd = 0;
	int score = 0, maxscore = 0;
	int *pick = new int[3]{-1,0,0};

	time_t end, start = time(NULL);
	genLegalMoves();
	if (nlm == 0) {        // computer player lost
		return pick;
	} else if (nlm == 1) { // only one valid move
		pick[0] = 0;
	} else {               // do search
		nnlm = nlm;
		while (++maxd<sizeof(stack)/sizeof(lms)) { // iterative deepening
			for (i = 0; i < nnlm; i++) {           // start search at each legal move
				stack[0] = lms[i];
				sind = 1;
				d = 1;
				while (sind-- > 0) {               // pop the stack if not empty
					if((score = stack[sind].score(!p)) > maxscore) {
						maxscore = score;
						pick[0] = i;
					}
					if (d < maxd) {
						stack[sind].genLegalMoves(&stack[sind]);
						sind+=nlm;
						if (nlm > 0) d++;
					} else {
						if(++dd>nlm) {
							d--;
							dd=0;
						}
					}
				}
			}
		}
	}
	*this = lms[pick[0]];
	end = time(NULL);
	pick[1] = std::difftime(end,start);
	pick[2] = maxd;
	return pick;
}

bool cbb::humanPickMove(int pick) {
	if (pick < nlm) {
		*this = lms[pick];
		return true;
	}
	return false;
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
		for (int j = 1; j < nlm; j++)
			jumps[i][j] = {0,0,0};

	genLegalMoves();

	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < nlm; j++) {
			if (j > 0 && !jumps[i][j].nonempty())
				jumps[i][j] = jumps[i][j-1];
			else {
				jumps[i][j].k |= jumps[i][j].w & 15;
				if (p) flipboard(jumps[i][j]);
			}
		}
	}
	for (int c = 0; c < nlm; c++) {
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
