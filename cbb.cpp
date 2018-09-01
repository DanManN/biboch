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
	if (numBits(ob) == 0)
		return std::numeric_limits<int>::max();
	return numBits(pb)-numBits(ob);
}

inline int cbb::alphabeta(cbb *node, uint32_t d, int alpha, int beta, bool mP) {
	int score, bscore, i;

	if (d == 0)
		return node->score(p);
	if (mP) {
		bscore = std::numeric_limits<int>::min();
		genLegalMoves(node);
		if (nlm == 0)
			return node->score(node->getPlayer());
		for (i = nlm-1; i >= 0; i--) {
			if ((score = alphabeta(node+i, d - 1, alpha, beta, false)) > bscore)
				bscore = score;
			if (bscore > alpha)
				alpha = bscore;
			if (alpha >= beta)
				break;
		}
		return bscore;
	} else {
		bscore = std::numeric_limits<int>::max();
		genLegalMoves(node);
		if (nlm == 0)
			return node->score(node->getPlayer());
		for (i = nlm-1; i > 0; i--) {
			if ((score = alphabeta(node+i, d - 1, alpha, beta, true)) < bscore)
				bscore = score;
			if (bscore < beta)
				beta = bscore;
			if (alpha >= beta)
				break;
		}
		return bscore;
	}
}

using namespace std::chrono;
int *cbb::aiPickMove(int timeLimit) {
	int i, score, bscore = std::numeric_limits<int>::min();
	uint32_t maxd = 0;
	int *pick = new int[3]{-1,0,0};

	time_point<system_clock, system_clock::duration> end, start = system_clock::now();
	genLegalMoves();
	if (nlm == 0) {        // computer player lost
		return pick;
	} else if (nlm == 1) { // only one valid move
		pick[0] = 0;
	} else {               // do search
		while (++maxd<sizeof(stack)/sizeof(lms)) { // iterative deepening
			for (i = nlm-1; i >= 0; i--) {         // start search at each legal move
				if (duration_cast<milliseconds>(system_clock::now()-start).count() > timeLimit*3/4)
					goto BREAK;
				stack[0] = lms[i];
				if ((score = alphabeta(stack, maxd, std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), false)) > bscore) {
					bscore = score;
					pick[0] = i;
				}
				if (bscore == std::numeric_limits<int>::max())
					goto BREAK;
			}
		}
	}
	BREAK:
	*this = lms[pick[0]];
	end = system_clock::now();
	pick[1] = duration_cast<milliseconds>(end-start).count();
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
