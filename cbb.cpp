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
#include <chrono>
#include <random>
#include <cmath>
#include <cstring>

cbb cbb::lms[MAX_LMS];
int cbb::nlm;
cbb cbb::stack[STACK_SIZE];
cbb::board cbb::jumps[MAX_CJUMPS][MAX_LMS];

cbb::cbb(const char sboard[32], int player) {
	for (int i = 0; i < 32; i++) {
		switch (sboard[i]) {
			case '1':
				cb.b |= 1<<i;
				break;
			case '2':
				cb.w |= 1<<i;
				break;
			case '3':
				cb.b |= 1<<i;
				cb.k |= 1<<i;
				break;
			case '4':
				cb.w |= 1<<i;
				cb.k |= 1<<i;
				break;
		}
	}
	p = player;
}

std::mt19937 rg(time(NULL));
std::uniform_int_distribution<int> rnd(0,9);

inline int cbb::score(int player) {
	uint32_t pb = player ? cb.b : cb.w;
	uint32_t ob = player ? cb.w : cb.b;
	uint32_t nrp = numBits(pb^cb.k);
	uint32_t nkp = numBits(pb&cb.k);
	uint32_t nro = numBits(ob^cb.k);
	uint32_t nko = numBits(ob&cb.k);
	//std::cerr << pb << " " << ob << "\n";
	if (numBits(ob) == 0)
		return 100000000+rnd(rg);
	if (numBits(pb) == 0)
		return -100000000+rnd(rg);

	return
		10000000*( // Piece/King counts
				5*nkp+3*nrp-5*nko-3*nro
				)
		+ 000000*( // Piece Placement
				player
				? 3*numBits((pb^cb.k)&0x000F0000)-3*numBits((ob^cb.k)&0x0000F000)
				+ 5*numBits((pb^cb.k)&0x00F00000)-5*numBits((ob^cb.k)&0x00000F00)
				+ 7*numBits((pb^cb.k)&0x0F000000)-7*numBits((ob^cb.k)&0x000000F0)
				: 3*numBits((pb^cb.k)&0x0000F000)-3*numBits((ob^cb.k)&0x000F0000)
				+ 5*numBits((pb^cb.k)&0x00000F00)-5*numBits((ob^cb.k)&0x00F00000)
				+ 7*numBits((pb^cb.k)&0x000000F0)-7*numBits((ob^cb.k)&0x0F000000)
				)
		+   0000*( // Trade influencer
				(3*nrp+5*nkp) > (3*nro+5*nko)
				? 24-(nrp+nkp+nro+nko)
				: (nrp+nkp+nro+nko)
				)
		+     00*( // King Placement
				  7*numBits((pb&cb.k)&0x00066000)-7*numBits((ob&cb.k)&0x00066000)
				+ 5*numBits((pb&cb.k)&0x00600600)-5*numBits((ob&cb.k)&0x00600600)
				+ 3*numBits((pb&cb.k)&0x06000060)-3*numBits((ob&cb.k)&0x06000060)
				+ 1*numBits((pb&cb.k)&0xF000000F)-1*numBits((ob&cb.k)&0xF000000F)
				)
		+ rnd(rg); // Random twiddle


}

inline int cbb::negalphabeta(cbb *node, uint32_t d, uint32_t maxd, int alpha, int beta) {
	int score, bscore, i;

	if (d == maxd)
		return node->score(!node->p);
	node->genLegalMoves(&node[1]);
	if (nlm == 0)
		return node->score(!node->p);
	bscore = std::numeric_limits<int>::min();
	for (i = nlm; i > 0; i--) {
		if ((score = -negalphabeta(&node[i], d+1, maxd, -beta, -alpha)) > bscore)
			bscore = score;
		if (bscore <= alpha)
			alpha = bscore;
		if (alpha >= beta) {
			bscore++;
			break;
		}
	}
	return bscore;
}

using namespace std::chrono;
int *cbb::aiPickMove(int timeLimit) {
	int score, bscore, nlmm1=0, tpick=0, i=0;
	uint32_t maxd = 0;
	int *pick = new int[4]{-1};

	time_point<system_clock, system_clock::duration> end, start = system_clock::now();
	genLegalMoves();
	if (nlm == 0) {        // computer player lost
		return pick;
	} else if (nlm == 1) { // only one valid move
		pick[0] = 0;
	} else {               // do search
		nlmm1 = nlm-1;
		while (++maxd<sizeof(stack)/sizeof(lms)) { // iterative deepening
			bscore = std::numeric_limits<int>::min();
			for (i = nlmm1; i >= 0; i--) {         // start search at each legal move
				if (duration_cast<milliseconds>(system_clock::now()-start).count() > timeLimit/2)
					goto BREAK;
				stack[0] = lms[i];
				if ((score = -negalphabeta(stack, 0, maxd, std::numeric_limits<int>::max(), std::numeric_limits<int>::min())) > bscore) {
					if (score == std::numeric_limits<int>::max()) {
						pick[0] = i;
						/* std::cerr << i << ": " << score << "\n"; */
						goto BREAK;
					} else {
						bscore = score;
						tpick = i;
					}
				}
				/* std::cerr << i << ": " << score << "\n"; */
			}
			pick[0] = tpick;
		}
	}
	BREAK:
	*this = lms[pick[0]];
	end = system_clock::now();
	pick[1] = duration_cast<milliseconds>(end-start).count();
	pick[2] = maxd;
	pick[3] = i != nlmm1;
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

			if (cb.w>>ind & 1) std::cout << "\033[1;31;47m";

			if (cb.k>>ind & 1) std::cout << " ⟆";
			else std::cout << "  ";

			if (cb.w>>ind & 1) std::cout << "⊂⊃";
			else if (cb.b>>ind & 1) std::cout << "⋐⋑";
			else std::cout << "  ";

			if (cb.k>>ind & 1) std::cout << "⟅ ";
			else std::cout << "  ";

			if (cb.w>>ind & 1) std::cout << "\033[1;30;47m";
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
	for (int i = 0; i < MAX_CJUMPS; i++)
		for (int j = 1; j < MAX_LMS; j++)
			jumps[i][j] = {0,0,0};

	genLegalMoves();

	for (int i = 0; i < MAX_CJUMPS; i++) {
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
			while (jumps[k][c] != lms[c].cb && k < MAX_CJUMPS-1) {
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
