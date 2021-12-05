/*
 *	BOARD.C
 *	Tom Kerrigan's Simple Chess Program (TSCP)
 *
 *	Copyright 1997 Tom Kerrigan
 */


#include <stdlib.h>
#include "defs.h"
#include "data.h"
#include "protos.h"


/* init_board() sets the board to the initial game state. */

void init_board()
{
	int sq;

	for (sq = 0; sq < SqLen; ++sq) {
		color[sq] = init_color[sq];
		piece[sq] = init_piece[sq];
	}
	side = LIGHT;
	xside = DARK;
	castle = CCall;
	ep = SqNil;
	fifty = 0;
	ply = 0;
	hply = 0;
	set_hash();  /* init_hash() must be called before this function */
	first_move[0] = 0;
}


/* init_hash() initializes the random numbers used by set_hash(). */

void init_hash()
{
	int colorkind, piecekind, sq;

	srand(0);
	for (colorkind = 0; colorkind < ColorLen; ++colorkind)
		for (piecekind = 0; piecekind < PieceLen; ++piecekind)
			for (sq = 0; sq < SqLen; ++sq)
				hash_piece[colorkind][piecekind][sq] = hash_rand();
	hash_side = hash_rand();
	for (sq = 0; sq < SqLen; ++sq)
		hash_ep[sq] = hash_rand();
}


/* hash_rand() XORs some shifted random numbers together to make sure
   we have good coverage of all 32 bits. (rand() returns 16-bit numbers
   on some systems.) */

int hash_rand()
{
	int i;
	int result = 0;

	for (i = 0; i < 32; ++i)
		result ^= rand() << i;
	return result;
}


/* set_hash() uses the Zobrist method of generating a unique number (hash)
   for the current chess position. Of course, there are many more chess
   positions than there are 32 bit numbers, so the numbers generated are
   not really unique, but they're unique enough for our purposes (to detect
   repetitions of the position).
   The way it works is to XOR random numbers that correspond to features of
   the position, e.g., if there's a black knight on SqB8, hash is XORed with
   hash_piece[BLACK][KNIGHT][SqB8]. All of the pieces are XORed together,
   hash_side is XORed if it's black's move, and the en passant square is
   XORed if there is one. (A chess technicality is that one position can't
   be a repetition of another if the en passant state is different.) */

void set_hash()
{
	int sq;

	hash = 0;
	for (sq = 0; sq < SqLen; ++sq)
		if (color[sq] != NoColor)
			hash ^= hash_piece[color[sq]][piece[sq]][sq];
	if (side == DARK)
		hash ^= hash_side;
	if (ep != SqNil)
		hash ^= hash_ep[ep];
}


/* in_check() returns TRUE if side s is in check and FALSE
   otherwise. It just scans the board to find side s's king
   and calls attack() to see if it's being attacked. */

BOOL in_check(int s)
{
	int sq;

	for (sq = 0; sq < SqLen; ++sq)
		if (piece[sq] == KING && color[sq] == s)
			return attack(sq, s ^ 1);
	return FALSE;  /* shouldn't get here */
}


/* attack() returns TRUE if square sq is being attacked by side
   s and FALSE otherwise. */

BOOL attack(int sq, int s)
{
	int frsq, tosq, stride;

	for (frsq = 0; frsq < SqLen; ++frsq)
		if (color[frsq] == s) {
			if (piece[frsq] == PAWN) {
				if (s == LIGHT) {
					if (MapToFile(frsq) != FileA && frsq + DeltaNW == sq)
						return TRUE;
					if (MapToFile(frsq) != FileH && frsq + DeltaNE == sq)
						return TRUE;
				}
				else {
					if (MapToFile(frsq) != FileA && frsq + DeltaSW == sq)
						return TRUE;
					if (MapToFile(frsq) != FileH && frsq + DeltaSE == sq)
						return TRUE;
				}
			}
			else
				for (stride = 0; stride < offsets[piece[frsq]]; ++stride)
					for (tosq = frsq;;) {
						tosq = mailbox[mailbox64[tosq] + offset[piece[frsq]][stride]];
						if (tosq == SqNil)
							break;
						if (tosq == sq)
							return TRUE;
						if (color[tosq] != NoColor)
							break;
						if (!slide[piece[frsq]])
							break;
					}
		}
	return FALSE;
}


/* gen() generates pseudo-legal moves for the current position.
   It scans the board to find friendly pieces and then determines
   what squares they attack. When it finds a piece/square
   combination, it calls gen_push to put the move on the "move
   stack." */

void gen()
{
	int frsq, tosq, stride;

	/* so far, we have no moves for the current ply */
	first_move[ply + 1] = first_move[ply];

	for (frsq = 0; frsq < SqLen; ++frsq)
		if (color[frsq] == side) {
			if (piece[frsq] == PAWN) {
				if (side == LIGHT) {
					if (MapToFile(frsq) != FileA && color[frsq + DeltaNW] == DARK)
						gen_push(frsq, frsq + DeltaNW, MCpawncapt);
					if (MapToFile(frsq) != FileH && color[frsq + DeltaNE] == DARK)
						gen_push(frsq, frsq + DeltaNE, MCpawncapt);
					if (color[frsq + DeltaN] == NoColor) {
						gen_push(frsq, frsq + DeltaN, MMpawnmove);
						if (MapToRank(frsq) == Rank2 && color[frsq + Delta2N] == NoColor)
							gen_push(frsq, frsq + Delta2N, MCpawndadv);
					}
				}
				else {
					if (MapToFile(frsq) != FileA && color[frsq + DeltaSW] == LIGHT)
						gen_push(frsq, frsq + DeltaSW, MCpawncapt);
					if (MapToFile(frsq) != FileH && color[frsq + DeltaSE] == LIGHT)
						gen_push(frsq, frsq + DeltaSE, MCpawncapt);
					if (color[frsq + DeltaS] == NoColor) {
						gen_push(frsq, frsq + DeltaS, MMpawnmove);
						if (MapToRank(frsq) == Rank7 && color[frsq + Delta2S] == NoColor)
							gen_push(frsq, frsq + Delta2S, MCpawndadv);
					}
				}
			}
			else
				for (stride = 0; stride < offsets[piece[frsq]]; ++stride)
					for (tosq = frsq;;) {
						tosq = mailbox[mailbox64[tosq] + offset[piece[frsq]][stride]];
						if (tosq == SqNil)
							break;
						if (color[tosq] != NoColor) {
							if (color[tosq] == xside)
								gen_push(frsq, tosq, MMcapture);
							break;
						}
						gen_push(frsq, tosq, 0);
						if (!slide[piece[frsq]])
							break;
					}
		}

	/* generate castle moves */
	if (side == LIGHT) {
		if (castle & CMwk)
			gen_push(SqE1, SqG1, MMcastle);
		if (castle & CMwq)
			gen_push(SqE1, SqC1, MMcastle);
	}
	else {
		if (castle & CMbk)
			gen_push(SqE8, SqG8, MMcastle);
		if (castle & CMbq)
			gen_push(SqE8, SqC8, MMcastle);
	}

	/* generate en passant moves */
	if (ep != SqNil) {
		if (side == LIGHT) {
			if (MapToFile(ep) != FileA && color[ep + DeltaSW] == LIGHT && piece[ep + DeltaSW] == PAWN)
				gen_push(ep + DeltaSW, ep, MCpawnep);
			if (MapToFile(ep) != FileH && color[ep + DeltaSE] == LIGHT && piece[ep + DeltaSE] == PAWN)
				gen_push(ep + DeltaSE, ep, MCpawnep);
		}
		else {
			if (MapToFile(ep) != FileA && color[ep + DeltaNW] == DARK && piece[ep + DeltaNW] == PAWN)
				gen_push(ep + DeltaNW, ep, MCpawnep);
			if (MapToFile(ep) != FileH && color[ep + DeltaNE] == DARK && piece[ep + DeltaNE] == PAWN)
				gen_push(ep + DeltaNE, ep, MCpawnep);
		}
	}
}


/* gen_caps() is basically a copy of gen() that's modified to
   only generate capture and promote moves. It's used by the
   quiescence search. */

void gen_caps()
{
	int frsq, tosq, stride;

	first_move[ply + 1] = first_move[ply];
	for (frsq = 0; frsq < SqLen; ++frsq)
		if (color[frsq] == side) {
			if (piece[frsq]==PAWN) {
				if (side == LIGHT) {
					if (MapToFile(frsq) != FileA && color[frsq + DeltaNW] == DARK)
						gen_push(frsq, frsq + DeltaNW, MCpawncapt);
					if (MapToFile(frsq) != FileH && color[frsq + DeltaNE] == DARK)
						gen_push(frsq, frsq + DeltaNE, MCpawncapt);
					if (MapToRank(frsq) == Rank7 && color[frsq + DeltaN] == NoColor)
						gen_push(frsq, frsq + DeltaN, MMpawnmove);
				}
				if (side == DARK) {
					if (MapToFile(frsq) != FileA && color[frsq + DeltaSW] == LIGHT)
						gen_push(frsq, frsq + DeltaSW, MCpawncapt);
					if (MapToFile(frsq) != FileH && color[frsq + DeltaSE] == LIGHT)
						gen_push(frsq, frsq + DeltaSE, MCpawncapt);
					if (MapToRank(frsq) == Rank2 && color[frsq + DeltaS] == NoColor)
						gen_push(frsq, frsq + DeltaS, MMpawnmove);
				}
			}
			else
				for (stride = 0; stride < offsets[piece[frsq]]; ++stride)
					for (tosq = frsq;;) {
						tosq = mailbox[mailbox64[tosq] + offset[piece[frsq]][stride]];
						if (tosq == SqNil)
							break;
						if (color[tosq] != NoColor) {
							if (color[tosq] == xside)
								gen_push(frsq, tosq, MMcapture);
							break;
						}
						if (!slide[piece[frsq]])
							break;
					}
		}
	if (ep != SqNil) {
		if (side == LIGHT) {
			if (MapToFile(ep) != FileA && color[ep + DeltaSW] == LIGHT && piece[ep + DeltaSW] == PAWN)
				gen_push(ep + DeltaSW, ep, MCpawnep);
			if (MapToFile(ep) != FileH && color[ep + DeltaSE] == LIGHT && piece[ep + DeltaSE] == PAWN)
				gen_push(ep + DeltaSE, ep, MCpawnep);
		}
		else {
			if (MapToFile(ep) != FileA && color[ep + DeltaNW] == DARK && piece[ep + DeltaNW] == PAWN)
				gen_push(ep + DeltaNW, ep, MCpawnep);
			if (MapToFile(ep) != FileH && color[ep + DeltaNE] == DARK && piece[ep + DeltaNE] == PAWN)
				gen_push(ep + DeltaNE, ep, MCpawnep);
		}
	}
}


/* gen_push() puts a move on the move stack, unless it's a
   pawn promotion that needs to be handled by gen_promote().
   It also assigns a score to the move for alpha-beta move
   ordering. If the move is a capture, it uses MVV/LVA
   (Most Valuable Victim/Least Valuable Attacker). Otherwise,
   it uses the move's history heuristic value. Note that
   1,000,000 is added to a capture move's score, so it
   always gets ordered above a "normal" move. */

void gen_push(int from, int to, int bits)
{
	gen_t *g;

	if (bits & MMpawnmove) {
		if (side == LIGHT) {
			if (MapToRank(to) == Rank8) {
				gen_promote(from, to, bits);
				return;
			}
		}
		else {
			if (MapToRank(to) == Rank1) {
				gen_promote(from, to, bits);
				return;
			}
		}
	}
	g = &gen_dat[first_move[ply + 1]++];
	g->m.b.from = (char)from;
	g->m.b.to = (char)to;
	g->m.b.promote = 0;
	g->m.b.bits = (char)bits;
	if (color[to] != NoColor)
		g->score = 1000000 + (piece[to] * 10) - piece[from];
	else
		g->score = history[from][to];
}


/* gen_promote() is just like gen_push(), only it puts 4 moves
   on the move stack, one for each possible promotion piece */

void gen_promote(int from, int to, int bits)
{
	int piecekind;
	gen_t *g;

	for (piecekind = KNIGHT; piecekind <= QUEEN; ++piecekind) {
		g = &gen_dat[first_move[ply + 1]++];
		g->m.b.from = (char)from;
		g->m.b.to = (char)to;
		g->m.b.promote = (char)piecekind;
		g->m.b.bits = (char)(bits | MMpromote);
		g->score = 1000000 + (piecekind * 10);
	}
}


/* makemove() makes a move. If the move is illegal, it
   undoes whatever it did and returns FALSE. Otherwise, it
   returns TRUE. */

BOOL makemove(move_bytes m)
{

	/* test to see if a castle move is legal and move the rook
	   (the king is moved with the usual move code later) */
	if (m.bits & MMcastle) {
		int from, to;

		if (in_check(side))
			return FALSE;
		switch (m.to) {
			case SqG1:
				if (color[SqF1] != NoColor || color[SqG1] != NoColor ||
						attack(SqF1, xside) || attack(SqG1, xside))
					return FALSE;
				from = SqH1;
				to = SqF1;
				break;
			case SqC1:
				if (color[SqB1] != NoColor || color[SqC1] != NoColor || color[SqD1] != NoColor ||
						attack(SqC1, xside) || attack(SqD1, xside))
					return FALSE;
				from = SqA1;
				to = SqD1;
				break;
			case SqG8:
				if (color[SqF8] != NoColor || color[SqG8] != NoColor ||
						attack(SqF8, xside) || attack(SqG8, xside))
					return FALSE;
				from = SqH8;
				to = SqF8;
				break;
			case SqC8:
				if (color[SqB8] != NoColor || color[SqC8] != NoColor || color[SqD8] != NoColor ||
						attack(SqC8, xside) || attack(SqD8, xside))
					return FALSE;
				from = SqA8;
				to = SqD8;
				break;
			default:  /* shouldn't get here */
				from = SqNil;
				to = SqNil;
				break;
		}
		color[to] = color[from];
		piece[to] = piece[from];
		//color[from] = NoColor;
		//piece[from] = NoPiece;
		piece[from] = piece[from] + 1;
		
	}

	/* back up information so we can take the move back later. */
	hist_dat[hply].m.b = m;
	hist_dat[hply].capture = piece[(int)m.to];
	hist_dat[hply].castle = castle;
	hist_dat[hply].ep = ep;
	hist_dat[hply].fifty = fifty;
	hist_dat[hply].hash = hash;
	++ply;
	++hply;

	/* update the castle, en passant, and
	   fifty-move-draw variables */
	castle &= castle_mask[(int)m.from] & castle_mask[(int)m.to];
	if (m.bits & MMdoubleadv) {
		if (side == LIGHT)
			ep = m.to + DeltaS;
		else
			ep = m.to + DeltaN;
	}
	else
		ep = SqNil;
	if (m.bits & MCpawncapt)
		fifty = 0;
	else
		++fifty;

	/* move the piece */
	color[(int)m.to] = side;
	if (m.bits & MMpromote)
		piece[(int)m.to] = m.promote;
	else
		piece[(int)m.to] = piece[(int)m.from];
	//color[(int)m.from] = NoColor;
	piece[(int)m.from] = piece[(int)m.to] + 1;

	/* erase the pawn if this is an en passant move */
	if (m.bits & MMenpassant) {
		if (side == LIGHT) {
			color[m.to + DeltaS] = NoColor;
			piece[m.to + DeltaS] = NoPiece;
		}
		else {
			color[m.to + DeltaN] = NoColor;
			piece[m.to + DeltaN] = NoPiece;
		}
	}

	/* switch sides and test for legality (if we can capture
	   the other guy's king, it's an illegal position and
	   we need to take the move back) */
	side ^= 1;
	xside ^= 1;
	if (in_check(xside)) {
		takeback();
		return FALSE;
	}
	set_hash();
	return TRUE;
}


/* takeback() is very similar to makemove(), only backwards :)  */

void takeback()
{
	move_bytes m;

	side ^= 1;
	xside ^= 1;
	--ply;
	--hply;
	m = hist_dat[hply].m.b;
	castle = hist_dat[hply].castle;
	ep = hist_dat[hply].ep;
	fifty = hist_dat[hply].fifty;
	hash = hist_dat[hply].hash;
	color[(int)m.from] = side;
	if (m.bits & MMpromote)
		piece[(int)m.from] = PAWN;
	else
		piece[(int)m.from] = piece[(int)m.to];
	if (hist_dat[hply].capture == NoPiece) {
		color[(int)m.to] = NoColor;
		piece[(int)m.to] = NoPiece;
	}
	else {
		color[(int)m.to] = xside;
		piece[(int)m.to] = hist_dat[hply].capture;
	}
	if (m.bits & MMcastle) {
		int from, to;

		switch(m.to) {
			case SqG1:
				from = SqF1;
				to = SqH1;
				break;
			case SqC1:
				from = SqD1;
				to = SqA1;
				break;
			case SqG8:
				from = SqF8;
				to = SqH8;
				break;
			case SqC8:
				from = SqD8;
				to = SqA8;
				break;
			default:  /* shouldn't get here */
				from = SqNil;
				to = SqNil;
				break;
		}
		color[to] = side;
		piece[to] = ROOK;
		color[from] = NoColor;
		piece[from] = NoPiece;
	}
	if (m.bits & MMenpassant) {
		if (side == LIGHT) {
			color[m.to + DeltaS] = xside;
			piece[m.to + DeltaS] = PAWN;
		}
		else {
			color[m.to + DeltaN] = xside;
			piece[m.to + DeltaN] = PAWN;
		}
	}
}
