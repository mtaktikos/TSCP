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
	int i;

	for (i = 0; i < 80; ++i) {
		color[i] = init_color[i];
		piece[i] = init_piece[i];
	}
	side = LIGHT;
	xside = DARK;
	castle = 15;
	ep = -1;
	fifty = 0;
	ply = 0;
	hply = 0;
	set_hash();  /* init_hash() must be called before this function */
	first_move[0] = 0;
}


/* init_hash() initializes the random numbers used by set_hash(). */

void init_hash()
{
	int i, j, k;

	srand(0);
	for (i = 0; i < 2; ++i)
		for (j = 0; j < 8; ++j)
			for (k = 0; k < 80; ++k)
				hash_piece[i][j][k] = hash_rand();
	hash_side = hash_rand();
	for (i = 0; i < 80; ++i)
		hash_ep[i] = hash_rand();
}


/* hash_rand() XORs some shifted random numbers together to make sure
   we have good coverage of all 32 bits. (rand() returns 16-bit numbers
   on some systems.) */

int hash_rand()
{
	int i;
	int r = 0;

	for (i = 0; i < 32; ++i)
		r ^= rand() << i;
	return r;
}


/* set_hash() uses the Zobrist method of generating a unique number (hash)
   for the current chess position. Of course, there are many more chess
   positions than there are 32 bit numbers, so the numbers generated are
   not really unique, but they're unique enough for our purposes (to detect
   repetitions of the position).
   The way it works is to XOR random numbers that correspond to features of
   the position, e.g., if there's a black knight on B8, hash is XORed with
   hash_piece[BLACK][KNIGHT][B8]. All of the pieces are XORed together,
   hash_side is XORed if it's black's move, and the en passant square is
   XORed if there is one. (A chess technicality is that one position can't
   be a repetition of another if the en passant state is different.) */

void set_hash()
{
	int i;

	hash = 0;
	for (i = 0; i < 80; ++i)
		if (color[i] != EMPTY)
			hash ^= hash_piece[color[i]][piece[i]][i];
	if (side == DARK)
		hash ^= hash_side;
	if (ep != -1)
		hash ^= hash_ep[ep];
}


/* in_check() returns TRUE if side s is in check and FALSE
   otherwise. It checks if the castling Commoner (at F1 for LIGHT,
   F8 for DARK) is being attacked. Also returns TRUE if the castling
   Commoner is missing (captured), which means the side has lost. */

BOOL in_check(int s)
{
	int king_sq;
	
	/* The castling Commoner is at F1 for LIGHT, F8 for DARK */
	if (s == LIGHT)
		king_sq = F1;
	else
		king_sq = F8;
	
	/* Check if the castling Commoner exists */
	if (piece[king_sq] != COMMONER || color[king_sq] != s)
		return TRUE;  /* Commoner is missing or captured - side has lost */
	
	/* Check if the Commoner is being attacked */
	return attack(king_sq, s ^ 1);
}


/* is_transparent() returns TRUE if square sq is transparent for side s.
   A square is transparent if it has a friendly Amazon (W) on it, or if
   there's a friendly Amazon adjacent to it (8 directions). */

BOOL is_transparent(int sq, int s)
{
	int i, j;
	
	/* Check if the square itself has a friendly Amazon */
	if (color[sq] == s && piece[sq] == AMAZON)
		return TRUE;
	
	/* Check all 8 adjacent squares for a friendly Amazon */
	int adjacent_offsets[8] = { -13, -12, -11, -1, 1, 11, 12, 13 };
	for (i = 0; i < 8; ++i) {
		int n = mailbox[mailbox64[sq] + adjacent_offsets[i]];
		if (n != -1 && color[n] == s && piece[n] == AMAZON)
			return TRUE;
	}
	
	return FALSE;
}


/* attack() returns TRUE if square sq is being attacked by side
   s and FALSE otherwise. */

BOOL attack(int sq, int s)
{
	int i, j, n;

	for (i = 0; i < 80; ++i)
	{
		if (color[i] == s) {
			if (piece[i] == PAWN) {
				if (s == LIGHT) {
					if (COL(i) != 0 && i - 11 == sq)
						return TRUE;
					if (COL(i) != 9 && i - 9 == sq)
						return TRUE;
				}
				else {
					if (COL(i) != 0 && i + 9 == sq)
						return TRUE;
					if (COL(i) != 9 && i + 11 == sq)
						return TRUE;
				}
			}
			else if (piece[i] == AMAZON)
			{
				/* Amazon cannot capture, so it does not attack any square */
			}
			else
			{
				for (j = 0; j < offsets[piece[i]]; ++j)
				{
					for (n = i;;) {
						n = mailbox[mailbox64[n] + offset[piece[i]][j]];
						if (n == -1)
							break;
						if (n == sq)
							return TRUE;
						if (color[n] != EMPTY)
							break;
						if (!slide[piece[i]])
							break;
					}
				}
			}
		}
	}
	return FALSE;
}


void genCastles()
{
	if (side == LIGHT) {
		if (castle & 1)
			gen_push(F1, I1, 2);  /* white kingside: F1 to I1 */
		if (castle & 2)
			gen_push(F1, C1, 2);  /* white queenside: F1 to C1 */
	}
	else {
		if (castle & 4)
			gen_push(F8, H8, 2);  /* black queenside: F8 to H8 */
		if (castle & 8)
			gen_push(F8, B8, 2);  /* black kingside: F8 to B8 */
	}
}

void genEnPassant()
{
	if (ep != -1) {
		if (side == LIGHT) {
			if (COL(ep) != 0 && color[ep + 9] == LIGHT && piece[ep + 9] == PAWN)
				gen_push(ep + 9, ep, 21);
			if (COL(ep) != 9 && color[ep + 11] == LIGHT && piece[ep + 11] == PAWN)
				gen_push(ep + 11, ep, 21);
		}
		else {
			if (COL(ep) != 0 && color[ep - 11] == DARK && piece[ep - 11] == PAWN)
				gen_push(ep - 11, ep, 21);
			if (COL(ep) != 9 && color[ep - 9] == DARK && piece[ep - 9] == PAWN)
				gen_push(ep - 9, ep, 21);
		}
	}
}

void genPawn(int i)
{
	if (side == LIGHT) {
		if (COL(i) != 0 && color[i - 11] == DARK)
			gen_push(i, i - 11, 17);
		if (COL(i) != 9 && color[i - 9] == DARK)
			gen_push(i, i - 9, 17);
		if (color[i - 10] == EMPTY) {
			gen_push(i, i - 10, 16);
			/* Standard double move when intermediate square is empty */
			if (i >= 60 && color[i - 20] == EMPTY)
				gen_push(i, i - 20, 24);
		}
		/* Allow double move through transparent square when intermediate is occupied */
		else if (i >= 60 && is_transparent(i - 10, side) && color[i - 20] == EMPTY) {
			gen_push(i, i - 20, 24);
		}
	}
	else {
		if (COL(i) != 0 && color[i + 9] == LIGHT)
			gen_push(i, i + 9, 17);
		if (COL(i) != 9 && color[i + 11] == LIGHT)
			gen_push(i, i + 11, 17);
		if (color[i + 10] == EMPTY) {
			gen_push(i, i + 10, 16);
			/* Standard double move when intermediate square is empty */
			if (i <= 19 && color[i + 20] == EMPTY)
				gen_push(i, i + 20, 24);
		}
		/* Allow double move through transparent square when intermediate is occupied */
		else if (i <= 19 && is_transparent(i + 10, side) && color[i + 20] == EMPTY) {
			gen_push(i, i + 20, 24);
		}
	}
}

void genPiece(int i)
{
	int is_amazon = (piece[i] == AMAZON);
	
	/* Generate queen-like moves */
	for (int j = 0; j < offsets[piece[i]]; ++j)
	{
		int n = i;
		while (1) {
			n = mailbox[mailbox64[n] + offset[piece[i]][j]];
			if (n == -1)
				break;
			if (color[n] == EMPTY) {
				gen_push(i, n, 0);
				if (!slide[piece[i]])
					break;
			}
			else if (color[n] == side) {
				/* For slider pieces (not Amazon), check if we can pass through transparent squares */
				if (slide[piece[i]] && !is_amazon && is_transparent(n, side)) {
					/* Can pass through transparent square, continue sliding */
					continue;
				}
				else {
					/* Cannot move here - blocked by friendly piece */
					break;
				}
			}
			else {
				/* Enemy piece */
				if (!is_amazon)  /* Amazon cannot capture */
					gen_push(i, n, 1);
				break;
			}
		}
	}
	
	/* For Amazon, also generate knight moves (cannot capture) */
	if (is_amazon) {
		int knight_offsets[8] = { -25, -23, -14, -10, 10, 14, 23, 25 };
		for (int j = 0; j < 8; ++j) {
			int n = mailbox[mailbox64[i] + knight_offsets[j]];
			if (n != -1 && color[n] == EMPTY) {
				gen_push(i, n, 0);
			}
		}
		
		/* Add Dabbaba moves (2-square orthogonal jumps) */
		int dabbaba_offsets[4] = { -24, -2, 2, 24 };  /* 2 rows up/down, 2 cols left/right */
		for (int j = 0; j < 4; ++j) {
			int n = mailbox[mailbox64[i] + dabbaba_offsets[j]];
			if (n != -1 && color[n] == EMPTY) {
				gen_push(i, n, 0);
			}
		}
		
		/* Add Alfil moves (2-square diagonal jumps) */
		int alfil_offsets[4] = { -26, -22, 22, 26 };  /* 2 squares diagonally */
		for (int j = 0; j < 4; ++j) {
			int n = mailbox[mailbox64[i] + alfil_offsets[j]];
			if (n != -1 && color[n] == EMPTY) {
				gen_push(i, n, 0);
			}
		}
	}
}

void genSidePiece(int i)
{
	if (piece[i] == PAWN) {
		genPawn(i);

	}
	else
	{
		genPiece(i);

	}

}

void genMoves()
{

	for (int i = 0; i < 80; ++i)
	{
		if (color[i] == side) {
			genSidePiece(i);
		}

	}
}

/* gen() generates pseudo-legal moves for the current position.
   It scans the board to find friendly pieces and then determines
   what squares they attack. When it finds a piece/square
   combination, it calls gen_push to put the move on the "move
   stack." */

void gen()
{
	/* so far, we have no moves for the current ply */
	first_move[ply + 1] = first_move[ply];
	genMoves();

	/* generate castle moves */
	genCastles();
	/* generate en passant moves */
	genEnPassant();

}


/* gen_caps() is basically a copy of gen() that's modified to
   only generate capture and promote moves. It's used by the
   quiescence search. */

void gen_caps()
{
	int i, j, n;

	first_move[ply + 1] = first_move[ply];
	for (i = 0; i < 80; ++i)
		if (color[i] == side) {
			if (piece[i] == PAWN) {
				if (side == LIGHT) {
					if (COL(i) != 0 && color[i - 11] == DARK)
						gen_push(i, i - 11, 17);
					if (COL(i) != 9 && color[i - 9] == DARK)
						gen_push(i, i - 9, 17);
					if (i <= 19 && color[i - 10] == EMPTY)
						gen_push(i, i - 10, 16);
				}
				if (side == DARK) {
					if (COL(i) != 0 && color[i + 9] == LIGHT)
						gen_push(i, i + 9, 17);
					if (COL(i) != 9 && color[i + 11] == LIGHT)
						gen_push(i, i + 11, 17);
					if (i >= 60 && color[i + 10] == EMPTY)
						gen_push(i, i + 10, 16);
				}
			}
			else if (piece[i] == AMAZON) {
				/* Amazon cannot capture, so no moves are generated in gen_caps() */
				/* This is intentional - gen_caps() only generates capturing moves */
			}
			else
				for (j = 0; j < offsets[piece[i]]; ++j)
					for (n = i;;) {
						n = mailbox[mailbox64[n] + offset[piece[i]][j]];
						if (n == -1)
							break;
						if (color[n] != EMPTY) {
							if (color[n] == xside) {
								gen_push(i, n, 1);
								break;
							}
							else if (slide[piece[i]] && is_transparent(n, side)) {
								/* Can pass through transparent square, continue */
								continue;
							}
							else {
								/* Blocked by friendly piece */
								break;
							}
						}
						if (!slide[piece[i]])
							break;
					}
		}
	if (ep != -1) {
		if (side == LIGHT) {
			if (COL(ep) != 0 && color[ep + 9] == LIGHT && piece[ep + 9] == PAWN)
				gen_push(ep + 9, ep, 21);
			if (COL(ep) != 9 && color[ep + 11] == LIGHT && piece[ep + 11] == PAWN)
				gen_push(ep + 11, ep, 21);
		}
		else {
			if (COL(ep) != 0 && color[ep - 11] == DARK && piece[ep - 11] == PAWN)
				gen_push(ep - 11, ep, 21);
			if (COL(ep) != 9 && color[ep - 9] == DARK && piece[ep - 9] == PAWN)
				gen_push(ep - 9, ep, 21);
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

	if (bits & 16) {
		if (side == LIGHT) {
			if (to <= J8) {  /* promotion to rank 8 */
				gen_promote(from, to, bits);
				return;
			}
		}
		else {
			if (to >= A1) {  /* promotion to rank 1 */
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
	if (color[to] != EMPTY)
		g->score = 1000000 + (piece[to] * 10) - piece[from];
	else
		g->score = history[from][to];
}


/* gen_promote() is just like gen_push(), only it puts 4 moves
   on the move stack, one for each possible promotion piece */

void gen_promote(int from, int to, int bits)
{
	int i;
	gen_t *g;

	for (i = KNIGHT; i <= QUEEN; ++i) {
		g = &gen_dat[first_move[ply + 1]++];
		g->m.b.from = (char)from;
		g->m.b.to = (char)to;
		g->m.b.promote = (char)i;
		g->m.b.bits = (char)(bits | 32);
		g->score = 1000000 + (i * 10);
	}
}


/* makemove() makes a move. If the move is illegal, it
   undoes whatever it did and returns FALSE. Otherwise, it
   returns TRUE. */

BOOL makemove(move_bytes m)
{

	/* test to see if a castle move is legal and move the rook
	   (the king is moved with the usual move code later) */
	if (m.bits & 2) {
		int from, to;

		if (in_check(side))
			return FALSE;
		switch (m.to) {
		case 78:  /* I1 - white kingside castle: King f1->i1, Rook j1->h1 */
			if (color[G1] != EMPTY || color[H1] != EMPTY || color[I1] != EMPTY ||
				attack(G1, xside) || attack(H1, xside) || attack(I1, xside))
				return FALSE;
			from = J1;
			to = H1;
			break;
		case 72:  /* C1 - white queenside castle: King f1->c1, Rook a1->d1 */
			if (color[B1] != EMPTY || color[C1] != EMPTY || color[D1] != EMPTY || color[E1] != EMPTY ||
				attack(C1, xside) || attack(D1, xside) || attack(E1, xside))
				return FALSE;
			from = A1;
			to = D1;
			break;
		case 7:  /* H8 - black queenside castle: King f8->h8, Rook j8->i8 */
			if (color[G8] != EMPTY || color[H8] != EMPTY || color[I8] != EMPTY ||
				attack(G8, xside) || attack(H8, xside) || attack(I8, xside))
				return FALSE;
			from = J8;
			to = I8;
			break;
		case 1:  /* B8 - black kingside castle: King f8->b8, Rook a8->c8 */
			if (color[B8] != EMPTY || color[C8] != EMPTY || color[D8] != EMPTY || color[E8] != EMPTY ||
				attack(B8, xside) || attack(C8, xside) || attack(D8, xside))
				return FALSE;
			from = A8;
			to = C8;
			break;
		default:  /* shouldn't get here */
			from = -1;
			to = -1;
			break;
		}
		color[to] = color[from];
		piece[to] = piece[from];
		color[from] = EMPTY;
		piece[from] = EMPTY;
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
	if (m.bits & 8) {
		if (side == LIGHT)
			ep = m.to + 10;
		else
			ep = m.to - 10;
	}
	else
		ep = -1;
	if (m.bits & 17)
		fifty = 0;
	else
		++fifty;

	/* move the piece */
	color[(int)m.to] = side;
	if (m.bits & 32)
		piece[(int)m.to] = m.promote;
	else
		piece[(int)m.to] = piece[(int)m.from];
	color[(int)m.from] = EMPTY;
	piece[(int)m.from] = EMPTY;

	/* erase the pawn if this is an en passant move */
	if (m.bits & 4) {
		if (side == LIGHT) {
			color[m.to + 10] = EMPTY;
			piece[m.to + 10] = EMPTY;
		}
		else {
			color[m.to - 10] = EMPTY;
			piece[m.to - 10] = EMPTY;
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
	if (m.bits & 32)
		piece[(int)m.from] = PAWN;
	else
		piece[(int)m.from] = piece[(int)m.to];
	if (hist_dat[hply].capture == EMPTY) {
		color[(int)m.to] = EMPTY;
		piece[(int)m.to] = EMPTY;
	}
	else {
		color[(int)m.to] = xside;
		piece[(int)m.to] = hist_dat[hply].capture;
	}
	if (m.bits & 2) {
		int from, to;

		switch (m.to) {
		case 78:  /* I1 - white kingside castle */
			from = H1;
			to = J1;
			break;
		case 72:  /* C1 - white queenside castle */
			from = D1;
			to = A1;
			break;
		case 7:  /* H8 - black queenside castle */
			from = I8;
			to = J8;
			break;
		case 1:  /* B8 - black kingside castle */
			from = C8;
			to = A8;
			break;
		default:  /* shouldn't get here */
			from = -1;
			to = -1;
			break;
		}
		color[to] = side;
		piece[to] = ROOK;
		color[from] = EMPTY;
		piece[from] = EMPTY;
	}
	if (m.bits & 4) {
		if (side == LIGHT) {
			color[m.to + 10] = xside;
			piece[m.to + 10] = PAWN;
		}
		else {
			color[m.to - 10] = xside;
			piece[m.to - 10] = PAWN;
		}
	}
}
