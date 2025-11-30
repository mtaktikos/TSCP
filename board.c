/*
 *	BOARD.C
 *	Tom Kerrigan's Simple Chess Program (TSCP)
 *
 *	Copyright 1997 Tom Kerrigan
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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
	compute_transparent_squares();  /* compute transparent squares based on initial position */
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
   otherwise. It just scans the board to find side s's king
   and calls attack() to see if it's being attacked. 
   NOTE: This function is no longer used for move validation since
   capturing the King ends the game immediately. */

BOOL in_check(int s)
{
	int i;

	for (i = 0; i < 80; ++i)
		if (piece[i] == KING && color[i] == s)
			return attack(i, s ^ 1);
	return FALSE;  /* King not found - should not happen in normal game */
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
						if (color[n] != EMPTY) {
							/* For sliders, check if this square is transparent */
							if (slide[piece[i]] && piece[i] != AMAZON &&
							    ((s == LIGHT && whitetransparent[n]) || 
							     (s == DARK && blacktransparent[n]))) {
								/* Can pass through transparent square, continue sliding */
								continue;
							}
							else {
								/* Cannot pass through non-transparent square */
								break;
							}
						}
						if (!slide[piece[i]])
							break;
					}
				}
			}
		}
	}
	return FALSE;
}


/* compute_transparent_squares() computes which squares are transparent for each side
   based on AMAZON (W) piece positions. The square containing an AMAZON and the 8 adjacent
   squares are transparent for that side's sliders. */

void compute_transparent_squares()
{
	int i, j;
	
	/* Initialize all squares as non-transparent */
	for (i = 0; i < 80; ++i) {
		whitetransparent[i] = FALSE;
		blacktransparent[i] = FALSE;
	}
	
	/* Find AMAZONs and mark transparent squares */
	for (i = 0; i < 80; ++i) {
		if (piece[i] == AMAZON) {
			/* The AMAZON's square itself is transparent */
			if (color[i] == LIGHT)
				whitetransparent[i] = TRUE;
			else
				blacktransparent[i] = TRUE;
			
			/* Mark the 8 adjacent squares as transparent */
			int adjacent_offsets[8] = { -13, -12, -11, -1, 1, 11, 12, 13 };
			for (j = 0; j < 8; ++j) {
				int n = mailbox[mailbox64[i] + adjacent_offsets[j]];
				if (n != -1) {
					if (color[i] == LIGHT)
						whitetransparent[n] = TRUE;
					else
						blacktransparent[n] = TRUE;
				}
			}
		}
	}
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
			if (i >= 60 && color[i - 20] == EMPTY)
				gen_push(i, i - 20, 24);
		}
		/* Allow double move even if square in front is whitetransparent */
		else if (i >= 60 && whitetransparent[i - 10] && color[i - 20] == EMPTY) {
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
			if (i <= 19 && color[i + 20] == EMPTY)
				gen_push(i, i + 20, 24);
		}
		/* Allow double move even if square in front is blacktransparent */
		else if (i <= 19 && blacktransparent[i + 10] && color[i + 20] == EMPTY) {
			gen_push(i, i + 20, 24);
		}
	}
}

void genPiece(int i)
{
	int is_amazon = (piece[i] == AMAZON);
	BOOL is_slider = slide[piece[i]];
	
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
				if (!is_slider)
					break;
			}
			else if (color[n] == xside && !is_amazon) {
				/* Amazon cannot capture, other pieces can capture enemy pieces */
				gen_push(i, n, 1);
				/* For sliders, check if this square is transparent */
				if (is_slider && 
				    ((side == LIGHT && whitetransparent[n]) || 
				     (side == DARK && blacktransparent[n]))) {
					/* Can pass through transparent square, continue sliding */
					continue;
				}
				else {
					/* Cannot pass through non-transparent square */
					break;
				}
			}
			else if (color[n] == side) {
				/* Hit a friendly piece */
				/* For sliders, check if this square is transparent */
				/* Amazon cannot slide through transparent squares */
				if (is_slider && !is_amazon &&
				    ((side == LIGHT && whitetransparent[n]) || 
				     (side == DARK && blacktransparent[n]))) {
					/* Can pass through transparent square, continue sliding */
					continue;
				}
				else {
					/* Cannot pass through or capture friendly piece */
					break;
				}
			}
			/* Note: All color cases (EMPTY, xside, side) are handled above */
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
								/* For sliders, check if this square is transparent */
								if (slide[piece[i]] && piece[i] != AMAZON &&
								    ((side == LIGHT && whitetransparent[n]) || 
								     (side == DARK && blacktransparent[n]))) {
									/* Can pass through transparent square, continue sliding */
									continue;
								}
								else {
									/* Cannot pass through non-transparent square */
									break;
								}
							}
							else {
								/* Hit a friendly piece */
								/* For sliders, check if this square is transparent */
								if (slide[piece[i]] && piece[i] != AMAZON &&
								    ((side == LIGHT && whitetransparent[n]) || 
								     (side == DARK && blacktransparent[n]))) {
									/* Can pass through transparent square, continue sliding */
									continue;
								}
								else {
									/* Cannot pass through friendly piece */
									break;
								}
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
   returns TRUE. 
   NOTE: King (Commoner K) check validation has been removed since
   capturing the King ends the game immediately. */

BOOL makemove(move_bytes m)
{

	/* test to see if a castle move is legal and move the rook
	   (the king is moved with the usual move code later) */
	if (m.bits & 2) {
		int from, to;

		switch (m.to) {
		case 78:  /* I1 - white kingside castle: King f1->i1, Rook j1->h1 */
			if (color[G1] != EMPTY || color[H1] != EMPTY || color[I1] != EMPTY)
				return FALSE;
			from = J1;
			to = H1;
			break;
		case 72:  /* C1 - white queenside castle: King f1->c1, Rook a1->d1 */
			if (color[B1] != EMPTY || color[C1] != EMPTY || color[D1] != EMPTY || color[E1] != EMPTY)
				return FALSE;
			from = A1;
			to = D1;
			break;
		case 7:  /* H8 - black queenside castle: King f8->h8, Rook j8->i8 */
			if (color[G8] != EMPTY || color[H8] != EMPTY || color[I8] != EMPTY)
				return FALSE;
			from = J8;
			to = I8;
			break;
		case 1:  /* B8 - black kingside castle: King f8->b8, Rook a8->c8 */
			if (color[B8] != EMPTY || color[C8] != EMPTY || color[D8] != EMPTY || color[E8] != EMPTY)
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

	/* switch sides */
	side ^= 1;
	xside ^= 1;
	set_hash();
	compute_transparent_squares();  /* recompute transparent squares after move */
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
	compute_transparent_squares();  /* recompute transparent squares after undoing move */
}


/* piece_to_fen_char() converts a piece/color to a FEN character */
static char piece_to_fen_char(int p, int c)
{
	char ch;
	switch (p) {
		case PAWN:     ch = 'p'; break;
		case KNIGHT:   ch = 'n'; break;
		case BISHOP:   ch = 'b'; break;
		case ROOK:     ch = 'r'; break;
		case QUEEN:    ch = 'q'; break;
		case KING:     ch = 'k'; break;
		case COMMONER: ch = 'g'; break;
		case AMAZON:   ch = 'w'; break;
		default:       return '?';
	}
	if (c == LIGHT)
		return ch - 'a' + 'A';  /* uppercase for white */
	return ch;  /* lowercase for black */
}


/* fen_char_to_piece() converts a FEN character to piece type and color.
   Returns 1 on success, 0 on failure. */
static int fen_char_to_piece(char ch, int *p, int *c)
{
	if (ch >= 'A' && ch <= 'Z') {
		*c = LIGHT;
		ch = ch - 'A' + 'a';  /* convert to lowercase */
	} else if (ch >= 'a' && ch <= 'z') {
		*c = DARK;
	} else {
		return 0;
	}
	
	switch (ch) {
		case 'p': *p = PAWN; break;
		case 'n': *p = KNIGHT; break;
		case 'b': *p = BISHOP; break;
		case 'r': *p = ROOK; break;
		case 'q': *p = QUEEN; break;
		case 'k': *p = KING; break;
		case 'g': *p = COMMONER; break;
		case 'w': *p = AMAZON; break;
		default: return 0;
	}
	return 1;
}


/* set_fen() parses a FEN string and sets up the board position.
   Returns 1 on success, 0 on failure.
   FEN format for 10x8 board:
   <piece_placement> <side_to_move> <castling> <en_passant> <halfmove_clock> <fullmove_number>
   Example: rgnbkqbnwr/pppppppppp/10/10/10/10/PPPPPPPPPP/RWNBQKBNGR w KQkq - 0 1 */

int set_fen(const char *fen)
{
	int i, sq, p_val, c_val;
	const char *ptr;
	char ch;
	int empty_count;
	
	/* Use temporary storage to validate before applying */
	int temp_color[80];
	int temp_piece[80];
	int temp_side, temp_xside, temp_castle, temp_ep, temp_fifty;
	
	/* Clear the temporary board */
	for (i = 0; i < 80; ++i) {
		temp_color[i] = EMPTY;
		temp_piece[i] = EMPTY;
	}
	
	ptr = fen;
	sq = 0;  /* Start from a8 (index 0) */
	
	/* Parse piece placement */
	while (*ptr && *ptr != ' ') {
		ch = *ptr++;
		
		if (ch == '/') {
			/* Move to next rank - check if current rank is complete (10 squares) */
			if (sq % 10 != 0) {
				return 0;  /* Invalid: rank not complete */
			}
			continue;
		}
		
		if (ch >= '1' && ch <= '9') {
			/* Empty squares: single digit 1-9 */
			empty_count = ch - '0';
			/* Check for two-digit number (10) */
			if (*ptr == '0' && empty_count == 1) {
				empty_count = 10;
				ptr++;
			}
			sq += empty_count;
			if (sq > 80) {
				return 0;  /* Too many squares */
			}
			continue;
		}
		
		/* Piece character */
		if (!fen_char_to_piece(ch, &p_val, &c_val)) {
			return 0;  /* Invalid piece character */
		}
		if (sq >= 80) {
			return 0;  /* Too many squares */
		}
		temp_piece[sq] = p_val;
		temp_color[sq] = c_val;
		sq++;
	}
	
	if (sq != 80) {
		return 0;  /* Board not fully specified */
	}
	
	/* Skip space and parse side to move */
	if (*ptr != ' ') {
		return 0;
	}
	ptr++;
	
	if (*ptr == 'w' || *ptr == 'W') {
		temp_side = LIGHT;
		temp_xside = DARK;
	} else if (*ptr == 'b' || *ptr == 'B') {
		temp_side = DARK;
		temp_xside = LIGHT;
	} else {
		return 0;  /* Invalid side to move */
	}
	ptr++;
	
	/* Skip space and parse castling rights */
	if (*ptr != ' ') {
		return 0;
	}
	ptr++;
	
	temp_castle = 0;
	if (*ptr == '-') {
		ptr++;
	} else {
		while (*ptr && *ptr != ' ') {
			switch (*ptr) {
				case 'K': temp_castle |= 1; break;  /* White kingside */
				case 'Q': temp_castle |= 2; break;  /* White queenside */
				case 'k': temp_castle |= 4; break;  /* Black kingside */
				case 'q': temp_castle |= 8; break;  /* Black queenside */
				default: break;  /* Ignore invalid characters */
			}
			ptr++;
		}
	}
	
	/* Skip space and parse en passant square */
	if (*ptr != ' ') {
		return 0;
	}
	ptr++;
	
	if (*ptr == '-') {
		temp_ep = -1;
		ptr++;
	} else {
		/* Parse en passant square (e.g., "e3" or "e6") */
		if (*ptr < 'a' || *ptr > 'j') {
			return 0;  /* Invalid file */
		}
		int file = *ptr - 'a';
		ptr++;
		if (*ptr < '1' || *ptr > '8') {
			return 0;  /* Invalid rank */
		}
		int rank = 8 - (*ptr - '0');  /* Convert '1'-'8' to 0-7 (inverted for our representation) */
		ptr++;
		temp_ep = rank * 10 + file;
	}
	
	/* Parse halfmove clock (fifty move rule counter) - optional */
	if (*ptr == ' ') {
		ptr++;
		temp_fifty = 0;
		while (*ptr >= '0' && *ptr <= '9') {
			temp_fifty = temp_fifty * 10 + (*ptr - '0');
			ptr++;
		}
	} else {
		temp_fifty = 0;
	}
	
	/* Skip fullmove number (not used internally) */
	
	/* All validation passed - now apply the changes */
	for (i = 0; i < 80; ++i) {
		color[i] = temp_color[i];
		piece[i] = temp_piece[i];
	}
	side = temp_side;
	xside = temp_xside;
	castle = temp_castle;
	ep = temp_ep;
	fifty = temp_fifty;
	
	/* Reset game state */
	ply = 0;
	hply = 0;
	set_hash();
	compute_transparent_squares();
	first_move[0] = 0;
	
	return 1;  /* Success */
}


/* get_fen() generates a FEN string from the current position.
   The caller must provide a buffer of at least 128 characters. */

void get_fen(char *fen)
{
	int rank, file, sq, empty;
	char *ptr = fen;
	
	/* Generate piece placement */
	for (rank = 0; rank < 8; ++rank) {
		empty = 0;
		for (file = 0; file < 10; ++file) {
			sq = rank * 10 + file;
			if (color[sq] == EMPTY) {
				empty++;
			} else {
				if (empty > 0) {
					if (empty == 10) {
						*ptr++ = '1';
						*ptr++ = '0';
					} else {
						*ptr++ = '0' + empty;
					}
					empty = 0;
				}
				*ptr++ = piece_to_fen_char(piece[sq], color[sq]);
			}
		}
		if (empty > 0) {
			if (empty == 10) {
				*ptr++ = '1';
				*ptr++ = '0';
			} else {
				*ptr++ = '0' + empty;
			}
		}
		if (rank < 7) {
			*ptr++ = '/';
		}
	}
	
	/* Side to move */
	*ptr++ = ' ';
	*ptr++ = (side == LIGHT) ? 'w' : 'b';
	
	/* Castling rights */
	*ptr++ = ' ';
	if (castle == 0) {
		*ptr++ = '-';
	} else {
		if (castle & 1) *ptr++ = 'K';
		if (castle & 2) *ptr++ = 'Q';
		if (castle & 4) *ptr++ = 'k';
		if (castle & 8) *ptr++ = 'q';
	}
	
	/* En passant square */
	*ptr++ = ' ';
	if (ep == -1) {
		*ptr++ = '-';
	} else {
		*ptr++ = 'a' + COL(ep);
		*ptr++ = '0' + (8 - ROW(ep));
	}
	
	/* Halfmove clock and fullmove number */
	sprintf(ptr, " %d %d", fifty, (hply / 2) + 1);
}

