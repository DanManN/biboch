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
	/* if (numBits(ob) == 0) */
	/* 	return std::numeric_limits<int>::max(); */
	/* if (numBits(pb) == 0) */
	/* 	return std::numeric_limits<int>::min(); */
	return
		10000000*( // Piece/King counts
				6*(nkp-nko)+4*(nrp-nro)
				)
		+ 100000*( // Piece Placement
				player
				? 5*numBits((pb^cb.k)&0x0FF0000F)-3*numBits((ob^cb.k)&0xF0000FF0)
				: 5*numBits((pb^cb.k)&0xF0000FF0)-3*numBits((ob^cb.k)&0x0FF0000F)
				)
		+   1000*( // Trade influencer
				(nrp+nkp) > (nro+nko)
				? 24-(nrp+nkp+nro+nko)
				: (nrp+nkp+nro+nko)
				)
		+     10*( // King Placement
				  7*numBits((pb&cb.k)&0x00066000)-4*numBits((ob&cb.k)&0x00066000)
				+ 6*numBits((pb&cb.k)&0x00600600)-4*numBits((ob&cb.k)&0x00600600)
				+ 5*numBits((pb&cb.k)&0x06000060)-4*numBits((ob&cb.k)&0x06000060)
				)
		+ rnd(rg);                                                                // Random twiddle


}

inline int cbb::alphabeta(cbb *node, uint32_t d, int alpha, int beta, bool mP) {
	int score, bscore, i;

	if (d == 0)
		return node->score(p);
	node->genLegalMoves(&node[1]);
	if (nlm == 0)
		return node->score(p);
	if (mP) {
		bscore = std::numeric_limits<int>::min();
		for (i = nlm; i > 0; i--) {
			if ((score = alphabeta(&node[i], d - 1, alpha, beta, false)) > bscore)
				bscore = score;
			if (bscore > alpha)
				alpha = bscore;
			if (alpha >= beta)
				break;
		}
	} else {
		bscore = std::numeric_limits<int>::max();
		for (i = nlm; i > 0; i--) {
			if ((score = alphabeta(&node[i], d - 1, alpha, beta, true)) < bscore)
				bscore = score;
			if (bscore < beta)
				beta = bscore;
			if (alpha >= beta)
				break;
		}
	}
	return bscore;
}

using namespace std::chrono;
int *cbb::aiPickMove(int timeLimit) {
	int score, nlmm1, i=0, bscore = std::numeric_limits<int>::min();
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
			for (i = nlmm1; i >= 0; i--) {         // start search at each legal move
				stack[0] = lms[i];
				if ((score = alphabeta(stack, maxd, std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), false)) > bscore) {
					bscore = score;
					pick[0] = i;
				}
				/* if (bscore == std::numeric_limits<int>::max()) */
				/* 	goto BREAK; */
				if (duration_cast<milliseconds>(system_clock::now()-start).count() > timeLimit*(nlmm1-1)/(nlmm1+2))
					goto BREAK;
			}
		}
	}
	BREAK:
	*this = lms[pick[0]];
	end = system_clock::now();
	pick[1] = duration_cast<milliseconds>(end-start).count();
	pick[2] = maxd;
	pick[3] = i;
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
	for (int i = 0; i < MAX_CJUMPS; i++)
		for (int j = 1; j < nlm; j++)
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
