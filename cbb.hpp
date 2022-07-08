/*
 * Daniel Nakhimovich
 *
 * cbb.h: (checkerboard bit board class)
 *   - specifies checkerboard layout
 *   - defines/implements functions for finding valid moves
 *   - defines functions for human/AI game play and score
 *   - checkerboard bit board indexed as below
 *
 *     00  01  02  03
 *   04  05  06  07
 *     08  09  10  11
 *   12  13  14  15
 *     16  17  18  19
 *   20  21  22  23
 *     24  25  26  27
 *   28  29  30  31
 *
 */
#ifndef _CBB_H
#define _CBB_H

#include <limits>
#include <cstdlib>
#include "utils.hpp"
//#include <iostream>

class cbb {
private:
	typedef struct Board {
		uint32_t w;
		uint32_t b;
		uint32_t k;
		inline bool operator==(Board const &x)
		{
			return w == x.w && b == x.b && k == x.k;
		}
		inline bool operator!=(Board const &x)
		{
			return w != x.w || b != x.b || k != x.k;
		}
		inline bool nonempty()
		{
			return b || w;
		}
	} board;

	board cb = {}; // current checker board data
	int p; // current player

	/*** Utility Functions ***/

#define MAX_LMS (32)
#define MAX_CJUMPS (16)
#define STACK_SIZE (4096)

	static cbb lms[MAX_LMS]; // array for holding legal moves
	static int nlm; // variable to hold # of legal moves
	static cbb stack[STACK_SIZE]; // stack for alphabeta search
	static board jumps[MAX_CJUMPS][MAX_LMS]; // traceback array for showing jumps
	static const uint32_t
	        coords[]; // array of coordinates; checker of index i has coordinate coords[i]

	// clang-format off
#define UR3 (1<<24 | 1<<25 | 1<<26 | 1<<16 | 1<<17 | 1<<18 | 1<<8  | 1<<9  | 1<<10)
#define UL5 (1<<29 | 1<<30 | 1<<31 | 1<<21 | 1<<22 | 1<<23 | 1<<13 | 1<<14 | 1<<15 | 1<<05 | 1<<06 | 1<<07)
#define DR5 (1<<0  | 1<<1  | 1<<2  | 1<<8  | 1<<9  | 1<<10 | 1<<16 | 1<<17 | 1<<18 | 1<<24 | 1<<25 | 1<<26)
#define DL3 (1<<5  | 1<<6  | 1<<7  | 1<<13 | 1<<14 | 1<<15 | 1<<21 | 1<<22 | 1<<23)
	// clang-format on

	inline uint32_t distance(uint32_t p1, uint32_t p2)
	{
		int c1 = coords[pow2log2(p1)];
		int c2 = coords[pow2log2(p2)];
		return abs((c1 % 10) - (c2 % 10)) + abs((c1 / 10) - (c2 / 10));
	}

	inline void flipboard(board &bbc)
	{
		uint32_t tmp = bbc.w;
		bbc.w = reverseBits(bbc.b);
		bbc.b = reverseBits(tmp);
		bbc.k = reverseBits(bbc.k);
	}

	inline uint32_t getWmovers()
	{
		uint32_t es = ~(cb.b | cb.w);
		uint32_t mvs = cb.w & ((es << 4) | ((es & DR5) << 5) | ((es & DL3) << 3));
		uint32_t wk = cb.w & cb.k;
		if (wk != 0)
			mvs |= wk & ((es >> 4) | ((es & UL5) >> 5) | ((es & UR3) >> 3));
		return mvs;
	}

	inline uint32_t getWjumpers()
	{
		uint32_t es = ~(cb.b | cb.w);
		uint32_t tmp = cb.b & (es << 4);
		uint32_t jps = tmp ? cb.w & (((tmp & DR5) << 5) | ((tmp & DL3) << 3)) : 0;
		tmp = cb.b & (((es & DR5) << 5) | ((es & DL3) << 3));
		jps |= tmp ? cb.w & (tmp << 4) : 0;
		uint32_t wk = cb.w & cb.k;
		if (wk != 0) {
			tmp = cb.b & (es >> 4);
			jps |= tmp ? wk & (((tmp & UL5) >> 5) | ((tmp & UR3) >> 3)) : 0;
			tmp = cb.b & (((es & UL5) >> 5) | ((es & UR3) >> 3));
			jps |= tmp ? wk & (tmp >> 4) : 0;
		}
		return jps;
	}

	inline void buildWmove(board &bs, board &be, uint32_t start, uint32_t end,
	                       uint32_t taken = 0)
	{
		be.w = (bs.w ^ start) | end;
		be.b = bs.b ^ taken;
		be.k = (bs.k & start ? end : 0) | (bs.k & (be.b | be.w));
	}

	inline void dojumps(cbb *mptr, int d, board &b, uint32_t pce, uint32_t es)
	{
		bool cantjump = true;
		uint32_t tmp = b.b & (pce >> 4);
		if (((tmp & UL5) >> 5) & es) {
			cantjump = false;
			buildWmove(b, jumps[d][nlm], pce, tmp >> 5, tmp);
			dojumps(mptr, d + 1, jumps[d][nlm], tmp >> 5,
			        ~(jumps[d][nlm].b | jumps[d][nlm].w));
		}
		if (((tmp & UR3) >> 3) & es) {
			cantjump = false;
			buildWmove(b, jumps[d][nlm], pce, tmp >> 3, tmp);
			dojumps(mptr, d + 1, jumps[d][nlm], tmp >> 3,
			        ~(jumps[d][nlm].b | jumps[d][nlm].w));
		}
		tmp = b.b & (((pce & UL5) >> 5) | ((pce & UR3) >> 3));
		if ((tmp >> 4) & es) {
			cantjump = false;
			buildWmove(b, jumps[d][nlm], pce, tmp >> 4, tmp);
			dojumps(mptr, d + 1, jumps[d][nlm], tmp >> 4,
			        ~(jumps[d][nlm].b | jumps[d][nlm].w));
		}
		if (pce & b.k) {
			tmp = b.b & (pce << 4);
			if (((tmp & DR5) << 5) & es) {
				cantjump = false;
				buildWmove(b, jumps[d][nlm], pce, tmp << 5, tmp);
				dojumps(mptr, d + 1, jumps[d][nlm], tmp << 5,
				        ~(jumps[d][nlm].b | jumps[d][nlm].w));
			}
			if (((tmp & DL3) << 3) & es) {
				cantjump = false;
				buildWmove(b, jumps[d][nlm], pce, tmp << 3, tmp);
				dojumps(mptr, d + 1, jumps[d][nlm], tmp << 3,
				        ~(jumps[d][nlm].b | jumps[d][nlm].w));
			}
			tmp = b.b & (((pce & DR5) << 5) | ((pce & DL3) << 3));
			if ((tmp << 4) & es) {
				cantjump = false;
				buildWmove(b, jumps[d][nlm], pce, tmp << 4, tmp);
				dojumps(mptr, d + 1, jumps[d][nlm], tmp << 4,
				        ~(jumps[d][nlm].b | jumps[d][nlm].w));
			}
		}
		if (cantjump) {
			mptr[nlm++].cb = b;
		}
	}

	inline void genWmoves(cbb *mptr)
	{
		uint32_t es = ~(cb.b | cb.w);
		uint32_t pce;
		nlm = 0;
		uint32_t mvs = getWjumpers();
		if (mvs) {
			while (mvs) {
				pce = mvs - 1;
				mvs &= pce;
				pce = (pce ^ mvs) + 1;

				dojumps(mptr, 0, cb, pce, es);
			}
		} else {
			mvs = getWmovers();
			while (mvs) {
				pce = mvs - 1;
				mvs &= pce;
				pce = (pce ^ mvs) + 1;

				if ((es << 4) & pce) {
					buildWmove(cb, mptr[nlm++].cb, pce, pce >> 4);
				}
				if (((es & DR5) << 5) & pce) {
					buildWmove(cb, mptr[nlm++].cb, pce, pce >> 5);
				}
				if (((es & DL3) << 3) & pce) {
					buildWmove(cb, mptr[nlm++].cb, pce, pce >> 3);
				}
				if (pce & cb.k) {
					if ((es >> 4) & pce) {
						buildWmove(cb, mptr[nlm++].cb, pce, pce << 4);
					}
					if (((es & UL5) >> 5) & pce) {
						buildWmove(cb, mptr[nlm++].cb, pce, pce << 5);
					}
					if (((es & UR3) >> 3) & pce) {
						buildWmove(cb, mptr[nlm++].cb, pce, pce << 3);
					}
				}
			}
		}
	}

	inline void genLegalMoves(cbb *mptr = lms)
	{
		if (p)
			flipboard(cb);
		genWmoves(mptr);
		for (int i = 0; i < nlm; i++) {
			mptr[i].cb.k |= mptr[i].cb.w & 15;
			if (p)
				flipboard(mptr[i].cb);
			mptr[i].p = !p;
		}
		if (p)
			flipboard(cb);
	}

public:
	/** construct board from string **/
	cbb(const char sboard[32], int player);

	/** construct board from board **/
	cbb(board b = {0, 0, 0}, int player = 0)
	{
		cb = b;
		p = player;
	}

	/** return the score of the board for #player **/
	inline int score(int player);

	/** perform an alpha beta search on checker board node and return the score **/
	inline int negalphabeta(cbb *node, int d, int maxd, int alpha, int beta);

	/** update the board with move picked by ai **/
	int *aiPickMove(int timeLimit);
	/* returns {                                               *
	 *          # of move picked (-1 if none available),       *
	 *          the time in milliseconds spent picking a move, *
	 *          the depth searched,                            *
	 *          whether max depth was only partially searched  *
	 *         }                                               */

	/** update the board with move picked by human **/
	bool humanPickMove(int pick);
	/* returns false if pick doesn't point to a move, true otherwise */

	/** return the player whose move it is **/
	int getPlayer()
	{
		return p;
	}

	/** print board to stdout **/
	void printcb();
	/* Preview of UI:                                   *
	 *                                                  *
	 *       WhtChk      KingWC      BlkChk      KingBC *
	 * ██████  00  ██████  01  ██████  02  ██████  03   *
	 * ██████  ⊂⊃  ██████ ⟆⊂⊃⟅ ██████  ⋐⋑  ██████ ⟆⋐⋑ ⟅ *
	 * ██████      ██████      ██████      ██████       */

	/** print legal moves to stdout **/
	int printlms();
	/* return number of legal moves */
};

#endif //_CBB_H
