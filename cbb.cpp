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
std::uniform_int_distribution<int> rnd(0,9);

inline int64_t cbb::score(int player, int depth) {
	int64_t score = 0;
	//std::cerr << pb << " " << ob << "\n";
	if (numBits(cb.b) == 0)
		score = 10000000000-(depth*10);
	else if (numBits(cb.w) == 0)
		score = (depth*10)-10000000000;
	else {
		// Piece/King counts
		score = 1000000*(5*numBits(cb.w&cb.k)+3*numBits(cb.w^(cb.w&cb.k))-5*numBits(cb.b&cb.k)-3*numBits(cb.b^(cb.b&cb.k)));
		
		// King Placement
		/* if (numBits(cb.w^cb.b) <= 12 && numBits(cb.k) > numBits(cb.w^cb.b)/2) { */
		/* 	uint32_t ppcs = cb.w; */
		/* 	uint32_t ppce, opce, opcs; */
		/* 	while (ppcs) { */
		/* 		ppce = ppcs-1; */
		/* 		ppcs &= ppce; */
		/* 		ppce = (ppce^ppcs) + 1; */

		/* 		opcs = cb.b; */
		/* 		while (opcs) { */
		/* 			opce = opcs-1; */
		/* 			opcs &= opce; */
		/* 			opce = (opce^opcs) + 1; */

		/* 			if (score > 0) { */
		/* 				score -= 100*distance(ppce,opce); */
		/* 			} else { */
		/* 				score += 100*distance(ppce,opce); */
		/* 			} */
		/* 		} */
		/* 	} */
		/* } */

		// Trade Influencer
		/* if (score > 0) { */
		/* 	score -= 10000*(numBits(cb.w)+numBits(cb.b)); */
		/* } else { */
		/* 	score += 10000*(numBits(cb.w)+numBits(cb.b)); */
		/* } */

		// Piece Placement
		/* score +=  10000*( */
		/* 				  3*numBits((cb.w^(cb.w&cb.k))&0x000F0000)-3*numBits((cb.b^(cb.b&cb.k))&0x0000F000) */
		/* 				+ 4*numBits((cb.w^(cb.w&cb.k))&0x00F00000)-4*numBits((cb.b^(cb.b&cb.k))&0x00000F00) */
		/* 				+ 5*numBits((cb.w^(cb.w&cb.k))&0x0F000000)-5*numBits((cb.b^(cb.b&cb.k))&0x000000F0) */
		/* 				+ 7*numBits((cb.w^(cb.w&cb.k))&0x0000000F)-7*numBits((cb.b^(cb.b&cb.k))&0xF0000000) */
		/* 				); */


		score += rnd(rg);
	}
	/* std::cerr << "postscore: " << score << "\n"; */
	return player ? -score : score;
}

inline int cbb::negalphabeta(cbb *node, uint32_t d, uint32_t maxd, int64_t alpha, int64_t beta) {
	int64_t score, bscore;
	int i;

	if (d == maxd)
		return node->score(node->p, d);
	node->genLegalMoves(&node[1]);
	if (nlm == 0)
		return node->score(node->p, d);
	bscore = std::numeric_limits<int64_t>::min();
	for (i = nlm; i > 0; i--) {
		if ((score = -negalphabeta(&node[i], d+1, maxd, -beta, -alpha)) > bscore)
			bscore = score;
		if (bscore >= alpha)
			alpha = bscore;
		if (alpha >= beta) {
			/* bscore++; */
			break;
		}
	}
	return bscore;
}

using namespace std::chrono;
int *cbb::aiPickMove(int timeLimit) {
	int64_t score, bscore;
	int nlmm1=0, tpick=0, i=0;
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
			bscore = std::numeric_limits<int64_t>::min();
			for (i = nlmm1; i >= 0; i--) {         // start search at each legal move
				if (duration_cast<milliseconds>(system_clock::now()-start).count() > timeLimit/2)
					goto BREAK;
				stack[0] = lms[i];
				if ((score = -negalphabeta(stack, 0, maxd, std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::max())) > bscore) {
					/* if (score == std::numeric_limits<int>::max()) { */
					/*	pick[0] = i; */
					/*	goto BREAK; */
					/* } else { */
						bscore = score;
						tpick = i;
					/* } */
				}
			}
			pick[0] = tpick;
			/* std::cerr << "Score: " << bscore << "\n"; */
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
	/* std::cerr << "Score: " << score(p,0) << "\n"; */
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
