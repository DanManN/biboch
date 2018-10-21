/*
 * Daniel Nakhimovich
 *
 * cbb.cpp: (checkerboard bit board class)
 *   - implements constructor and display functions
 *   - implements functions for finding valid moves
 *   - implements functions for updating board with chosen moves
 *   - implements the the AI's search stategy and evaluation functions
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

const uint32_t cbb::coords[] = {
	  46, 55, 64, 73,
	36, 45, 54, 63,
	  35, 44, 53, 62,
	25, 34, 43, 52,
	  24, 33, 42, 51,
	14, 23, 32, 41,
	  13, 22, 31, 40,
	03, 12, 21, 30
};

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
std::uniform_int_distribution<int> rnd(-5,5);
#define INF       (100000000)
#define PRTY_HIGH (10000)
#define PRTY_MED  (100)
#define PRTY_LOW  (10)

inline int cbb::score(int player) {
	int score = 0;
	uint32_t pb = player ? cb.b : cb.w;
	uint32_t ob = player ? cb.w : cb.b;

	// Piece/King counts
	score = PRTY_HIGH * (
		  (5*numBits(pb&cb.k))+(3*numBits(pb^(pb&cb.k)))
		- (5*numBits(ob&cb.k))+(3*numBits(ob^(ob&cb.k)))
	);

	// King Placement (end game only)
	if (numBits(pb^ob) <= 12 && numBits(cb.k) > numBits(pb^ob)/2) {
		uint32_t ppcs = pb|ob;
		uint32_t ppce, opce, opcs, dist, maxdist=0, mindist=7;
		while (ppcs) {
			ppce = ppcs-1;
			ppcs &= ppce;
			ppce = (ppce^ppcs) + 1;

			opcs = ob|pb;
			while (opcs) {
				opce = opcs-1;
				opcs &= opce;
				opce = (opce^opcs) + 1;
				dist = distance(ppce,opce);
				if (dist > maxdist) maxdist = dist;
				if (dist < mindist) mindist = dist;
			}
		}
		score += (score > 0 ? -1 : 1)*PRTY_LOW*(maxdist+mindist);

		ppcs = pb;
		maxdist=0;
		mindist=7;
		while (ppcs) {
			ppce = ppcs-1;
			ppcs &= ppce;
			ppce = (ppce^ppcs) + 1;

			opcs = ob;
			while (opcs) {
				opce = opcs-1;
				opcs &= opce;
				opce = (opce^opcs) + 1;
				dist = distance(ppce,opce);
				if (dist > maxdist) maxdist = dist;
				if (dist < mindist) mindist = dist;
			}
		}
		score += (score > 0 ? -1 : 1)*PRTY_MED*(maxdist+mindist);
	}

	// Trade Influencer
	/* if (score > 0) { */
	/* 	score -= 1000000*(numBits(pb)+numBits(ob)); */
	/* } else { */
	/* 	score += 1000000*(numBits(pb)+numBits(ob)); */
	/* } */

	// Piece Placement
	/* score +=  10000*( */
	/* 				  3*numBits((pb^(pb&cb.k))&0x000F0000)-3*numBits((ob^(ob&cb.k))&0x0000F000) */
	/* 				+ 4*numBits((pb^(pb&cb.k))&0x00F00000)-4*numBits((ob^(ob&cb.k))&0x00000F00) */
	/* 				+ 5*numBits((pb^(pb&cb.k))&0x0F000000)-5*numBits((ob^(ob&cb.k))&0x000000F0) */
	/* 				+ 7*numBits((pb^(pb&cb.k))&0x0000000F)-7*numBits((ob^(ob&cb.k))&0xF0000000) */
	/* 				); */

	return score + rnd(rg);
}

inline int cbb::negalphabeta(cbb *node, int d, int maxd, int alpha, int beta) {
	int score, bscore;

	node->genLegalMoves(&node[1]);
	if (nlm == 0)  return d-(INF/10);
	if (d == maxd) return node->score(node->p);

	bscore = -INF;
	for (int i = nlm; i > 0; i--) {
		score = -negalphabeta(&node[i], d+1, maxd, -beta, -alpha);
		if (score > bscore) bscore = score;
		if (score > alpha) alpha = bscore;
		if (alpha >= beta) {
			bscore++;
			break;
		}
	}
	return bscore;
}

using namespace std::chrono;
int *cbb::aiPickMove(int timeLimit) {
	int score, bscore, i, nlmm1, tpick=0, maxd=0;
	int *pick = new int[4]{-1};

	time_point<system_clock, system_clock::duration> end, start = system_clock::now();
	genLegalMoves();
	if (nlm == 0) {                 // computer player lost
		return pick;
	} else if (nlm == 1) {          // only one valid move
		pick[0] = 0;
	} else {                        // do search
		nlmm1 = nlm-1;
		while (++maxd<STACK_SIZE) { // iterative deepening
			int alpha = -INF;
			int beta = INF;
			bscore = -INF;
			for (i = nlmm1; i >= 0; i--) { // start search at each legal move
				if (duration_cast<milliseconds>(system_clock::now()-start).count() > timeLimit/3) goto BREAK;
				stack[0] = lms[i];
				score = -negalphabeta(stack, 0, maxd, -beta, -alpha);
				if (score > bscore) {
					bscore = score;
					tpick = i;
				}
				if (score > alpha) alpha = bscore;
			}
			pick[0] = tpick;
			/* std::cout << bscore << "\n"; */
			pick[2] = (INF/10)-abs(bscore);
			if (bscore >= (INF/10)-maxd) break;
		}
	}
	i = nlmm1;
	BREAK:
	*this = lms[pick[0]];
	end = system_clock::now();
	pick[1] = duration_cast<milliseconds>(end-start).count();
	if (maxd < pick[2]) pick[2] = maxd;
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

int cbb::printlms() {
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
