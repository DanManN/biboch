/*
 * Daniel Nakhimovich
 *
 * cbb.h: (checkerboard bit board class)
 *   - specifies checkerboard layout
 *   - defines/implements functions for finding valid moves
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

#include <cstdint>
#include <ctime>
//#include <iostream>

#define UR3 (1<<24 | 1<<25 | 1<<26 | 1<<16 | 1<<17 | 1<<18 | 1<<8 | 1<<9 | 1<<10)
#define UL5 (1<<29 | 1<<30 | 1<<31 | 1<<21 | 1<<22 | 1<<23 | 1<<13 | 1<<14 | 1<<15 | 1<<05 | 1<<06 | 1<<07)
#define DR5 (1<<0 | 1<<1 | 1<<2 | 1<<8 | 1<<9 | 1<<10 | 1<<16 | 1<<17 | 1<<18 | 1<<24 | 1<<25 | 1<<26)
#define DL3 (1<<5 | 1<<6 | 1<<7 | 1<<13 | 1<<14 | 1<<15 | 1<<21 | 1<<22 | 1<<23)

class cbb {

	private:

		typedef struct Board{
			uint32_t w;
			uint32_t b;
			uint32_t k;
			inline bool operator==(Board const &x) {
				return w == x.w \
					&& b == x.b \
					&& k == x.k;
			}
			inline bool operator!=(Board const &x) {
				return w != x.w \
					|| b != x.b \
					|| k != x.k;
			}
			inline bool nonempty() {
				return b || w;
			}
		} board;

		board cb = {};
		int p;
		int tl;

		static cbb lms[64];
		static uint32_t nlm;

	/*** Utility Functions ***/

		inline uint32_t reverseBits(uint32_t b32) {
			// Thanks Sean Eron Anderson!
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

		inline void flipboard(board &bbc) {
			uint32_t tmp = bbc.w;
			bbc.w = reverseBits(bbc.b);
			bbc.b = reverseBits(tmp);
			bbc.k = reverseBits(bbc.k);
		}

		inline uint32_t getWmovers() {
			uint32_t es = ~(cb.b | cb.w);
			uint32_t mvs = cb.w & ((es << 4) | ((es&DR5) << 5) | ((es&DL3) << 3));
			uint32_t wk = cb.w & cb.k;
			if (wk != 0)
				mvs |= wk & ((es >> 4) | ((es&UL5) >> 5) | ((es&UR3) >> 3));
			return mvs;
		}

		inline uint32_t getWjumpers() {
			uint32_t es = ~(cb.b | cb.w);
			uint32_t tmp = cb.b & (es << 4);
			uint32_t jps = tmp ? cb.w & (((tmp&DR5) << 5) | ((tmp&DL3) << 3)) : 0;
			tmp = cb.b & (((es&DR5) << 5) | ((es&DL3) << 3));
			jps |= tmp ? cb.w & (tmp << 4) : 0;
			uint32_t wk = cb.w & cb.k;
			if (wk != 0) {
				tmp = cb.b & (es >> 4);
				jps |= tmp ? wk & (((tmp&UL5) >> 5) | ((tmp&UR3) >> 3)) : 0;
				tmp = cb.b & (((es&UL5) >> 5) | ((es&UR3) >> 3));
				jps |= tmp ? wk & (tmp >> 4) : 0;
			}
			return jps;
		}

		inline void buildWmove(board &bs, board &be, uint32_t start, uint32_t end, uint32_t taken = 0) {
			be.w = (bs.w ^ start) | end;
			be.b = bs.b ^ taken;
			be.k = (bs.k & start ? end : 0) | (bs.k & (be.b | be.w));
		}

		static board jumps[16][64];

		inline void dojumps(int d, board &b, uint32_t pce, uint32_t es) {
			bool cantjump = true;
			uint32_t tmp = b.b & (pce >> 4);
			if (((tmp&UL5) >> 5) & es) {
				cantjump = false;
				buildWmove(b, jumps[d][nlm], pce, tmp >> 5, tmp);
				dojumps(d+1, jumps[d][nlm], tmp >> 5, ~(jumps[d][nlm].b | jumps[d][nlm].w));
			}
			if (((tmp&UR3) >> 3) & es) {
				cantjump = false;
				buildWmove(b, jumps[d][nlm], pce, tmp >> 3, tmp);
				dojumps(d+1, jumps[d][nlm], tmp >> 3, ~(jumps[d][nlm].b | jumps[d][nlm].w));
			}
			tmp = b.b & (((pce&UL5) >> 5) | ((pce&UR3) >> 3));
			if ((tmp >> 4) & es) {
				cantjump = false;
				buildWmove(b, jumps[d][nlm], pce, tmp >> 4, tmp);
				dojumps(d+1, jumps[d][nlm], tmp >> 4, ~(jumps[d][nlm].b | jumps[d][nlm].w));
			}
			if (pce & b.k) {
				tmp = b.b & (pce << 4);
				if (((tmp&DR5) << 5) & es) {
					cantjump = false;
					buildWmove(b, jumps[d][nlm], pce, tmp << 5, tmp);
					dojumps(d+1, jumps[d][nlm], tmp << 5, ~(jumps[d][nlm].b | jumps[d][nlm].w));
				}
				if (((tmp&DL3) << 3) & es) {
					cantjump = false;
					buildWmove(b, jumps[d][nlm], pce, tmp << 3, tmp);
					dojumps(d+1, jumps[d][nlm], tmp << 3, ~(jumps[d][nlm].b | jumps[d][nlm].w));
				}
				tmp = b.b & (((pce&DR5) << 5) | ((pce&DL3) << 3));
				if ((tmp << 4) & es) {
					cantjump = false;
					buildWmove(b, jumps[d][nlm], pce, tmp << 4, tmp);
					dojumps(d+1, jumps[d][nlm], tmp << 4, ~(jumps[d][nlm].b | jumps[d][nlm].w));
				}
			}
			if (cantjump) {
				lms[nlm++].cb = b;
			}
		}

		inline void genWmoves() {
			uint32_t es = ~(cb.b | cb.w);
			uint32_t pce;
			nlm = 0;
			uint32_t mvs = getWjumpers();
			if (mvs) {
				while (mvs) {
					pce = mvs-1;
					mvs &= pce;
					pce = (pce^mvs) + 1;

					dojumps(0, cb, pce, es);
				}
			} else {
				mvs = getWmovers();
				while (mvs) {
					pce = mvs-1;
					mvs &= pce;
					pce = (pce^mvs) + 1;

					if ((es << 4) & pce) {
						buildWmove(cb, lms[nlm++].cb, pce, pce >> 4);
					}
					if (((es&DR5) << 5) & pce) {
						buildWmove(cb, lms[nlm++].cb, pce, pce >> 5);
					}
					if (((es&DL3) << 3) & pce) {
						buildWmove(cb, lms[nlm++].cb, pce, pce >> 3);
					}
					if (pce & cb.k) {
						if ((es >> 4) & pce) {
							buildWmove(cb, lms[nlm++].cb, pce, pce << 4);
						}
						if (((es&UL5) >> 5) & pce) {
							buildWmove(cb, lms[nlm++].cb, pce, pce << 5);
						}
						if (((es&UR3) >> 3) & pce) {
							buildWmove(cb, lms[nlm++].cb, pce, pce << 3);
						}
					}
				}
			}
		}

		inline void genLegalMoves() {
			if (p) flipboard(cb);
			genWmoves();
			for (uint32_t i = 0; i < nlm; i++) {
				lms[i].cb.k |= lms[i].cb.w & 15;
				if (p) flipboard(lms[i].cb);
				lms[i].p = !p;
				lms[i].tl = tl;
			}
			if (p) flipboard(cb);
		}

	public:

		/** construct board from string **/
		cbb(const char sboard[32], int player, int time);

		/** construct board from board **/
		cbb(board b = {0,0,0}, int player = 0, int time = 0) {
			cb = b;
			p = player;
			tl = time;
		}

		/** update the board with move picked by human **/
		bool humanPickMove(uint32_t pick);
		/* returns false if pick doesn't point to a move, true otherwise */

		/** update the board with move picked by human **/
		time_t aiPickMove();
		/* return -1 if AI can't move, otherwise          *
		 * return the time in milliseconds to find a move */

		/** return the player whose move it is **/
		int getPlayer() {
			return p;
		}

		/** print board to stdout **/
		void printcb();
		/* Preview of UI:                                   *
		 *                                                  *
		 *       WhtChk      KingWC      BlkChk      KingBC *
		 * ██████  00  ██████  01  ██████  02  ██████  03   *
		 * ██████  ⊂⊃  ██████ ⟆⊂⊃⟅ ██████  ⋐⋑  ██████ ⟆⋐⋑⟅  *
		 * ██████      ██████      ██████      ██████       */


		/** print legal moves to stdout **/
		uint32_t printlms();
		/* return number of legal moves */

	};

#endif
