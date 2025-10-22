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

	for (i = 0; i < 64; ++i) {
		color[i] = init_color[i];
		piece[i] = init_piece[i];
	}
	side = LIGHT;
	xside = DARK;
	castle = 0;  /* no castling in gravity chess */
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
		for (j = 0; j < 6; ++j)
			for (k = 0; k < 64; ++k)
				hash_piece[i][j][k] = hash_rand();
	hash_side = hash_rand();
	for (i = 0; i < 64; ++i)
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
	for (i = 0; i < 64; ++i)
		if (color[i] != EMPTY)
			hash ^= hash_piece[color[i]][piece[i]][i];
	if (side == DARK)
		hash ^= hash_side;
	if (ep != -1)
		hash ^= hash_ep[ep];
}


/* in_check() returns TRUE if side s is in check and FALSE
   otherwise. It just scans the board to find side s's king
   and calls attack() to see if it's being attacked. */

BOOL in_check(int s)
{
	int i;

	for (i = 0; i < 64; ++i)
		if (piece[i] == KING && color[i] == s)
			return attack(i, s ^ 1);
	return TRUE;  /* shouldn't get here */
}


/* attack() returns TRUE if square sq is being attacked by side
   s and FALSE otherwise. */

BOOL attack(int sq, int s)
{
	int i, j, n;

	for (i = 0; i < 64; ++i)
	{
		if (color[i] == s) {
			if (piece[i] == PAWN) {
				int col = COL(i);
				int row = ROW(i);
				if (s == LIGHT) {
					/* White pawns attack diagonally to the west (left) */
					if (col > 0) {
						/* Up-left diagonal: -1 column, -1 rank = i - 1 - 8 = i - 9 */
						if (row > 0 && i - 9 == sq)
							return TRUE;
						/* Down-left diagonal: -1 column, +1 rank = i - 1 + 8 = i + 7 */
						if (row < 7 && i + 7 == sq)
							return TRUE;
					}
				}
				else {
					/* Black pawns attack diagonally to the east (right) */
					if (col < 7) {
						/* Up-right diagonal: +1 column, -1 rank = i + 1 - 8 = i - 7 */
						if (row > 0 && i - 7 == sq)
							return TRUE;
						/* Down-right diagonal: +1 column, +1 rank = i + 1 + 8 = i + 9 */
						if (row < 7 && i + 9 == sq)
							return TRUE;
					}
				}
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
			gen_push(E1, G1, 2);
		if (castle & 2)
			gen_push(E1, C1, 2);
	}
	else {
		if (castle & 4)
			gen_push(E8, G8, 2);
		if (castle & 8)
			gen_push(E8, C8, 2);
	}
}

void genEnPassant()
{
	if (ep != -1) {
		if (side == LIGHT) {
			if (COL(ep) != 0 && color[ep + 7] == LIGHT && piece[ep + 7] == PAWN)
				gen_push(ep + 7, ep, 21);
			if (COL(ep) != 7 && color[ep + 9] == LIGHT && piece[ep + 9] == PAWN)
				gen_push(ep + 9, ep, 21);
		}
		else {
			if (COL(ep) != 0 && color[ep - 9] == DARK && piece[ep - 9] == PAWN)
				gen_push(ep - 9, ep, 21);
			if (COL(ep) != 7 && color[ep - 7] == DARK && piece[ep - 7] == PAWN)
				gen_push(ep - 7, ep, 21);
		}
	}
}

void genPawn(int i)
{
	int col = COL(i);
	int row = ROW(i);
	
	if (side == LIGHT) {
		/* White pawns move west (left, -1 column) */
		if (col > 0) {  /* can move left */
			int target = i - 1;  /* one square to the left */
			
			/* Diagonal captures - to the left and up/down one rank */
			/* Note: moving to adjacent column and up one rank means -1 col, -8 square = i - 1 - 8 = i - 9 */
			/* But since target = i - 1, we check target +/- 8 */
			if (row > 0 && color[i - 1 - 8] == DARK)  /* left and up */
				gen_push(i, i - 1 - 8, 17);
			if (row < 7 && color[i - 1 + 8] == DARK)  /* left and down */
				gen_push(i, i - 1 + 8, 17);
			
			/* Forward move (just left) */
			if (color[target] == EMPTY) {
				gen_push(i, target, 16);
				/* Double move from starting column (g = column 6) */
				if (col == 6 && color[i - 2] == EMPTY)
					gen_push(i, i - 2, 24);
			}
		}
	}
	else {
		/* Black pawns move east (right, +1 column) */
		if (col < 7) {  /* can move right */
			int target = i + 1;  /* one square to the right */
			
			/* Diagonal captures - to the right and up/down one rank */
			if (row > 0 && color[i + 1 - 8] == LIGHT)  /* right and up */
				gen_push(i, i + 1 - 8, 17);
			if (row < 7 && color[i + 1 + 8] == LIGHT)  /* right and down */
				gen_push(i, i + 1 + 8, 17);
			
			/* Forward move (just right) */
			if (color[target] == EMPTY) {
				gen_push(i, target, 16);
				/* Double move from starting column (b = column 1) */
				if (col == 1 && color[i + 2] == EMPTY)
					gen_push(i, i + 2, 24);
			}
		}
	}
}

void genPiece(int i)
{
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
			else {
				if (color[n] == xside)
					gen_push(i, n, 1);
				break;
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

	for (int i = 0; i < 64; ++i)
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

	/* No castling or en passant in gravity chess */
}


/* gen_caps() is basically a copy of gen() that's modified to
   only generate capture and promote moves. It's used by the
   quiescence search. */

void gen_caps()
{
	int i, j, n;

	first_move[ply + 1] = first_move[ply];
	for (i = 0; i < 64; ++i)
		if (color[i] == side) {
			if (piece[i] == PAWN) {
				int col = COL(i);
				int row = ROW(i);
				if (side == LIGHT) {
					/* White pawns capture diagonally to the west */
					if (col > 0) {
						/* Up-left and down-left captures */
						if (row > 0 && color[i - 9] == DARK)
							gen_push(i, i - 9, 17);
						if (row < 7 && color[i + 7] == DARK)
							gen_push(i, i + 7, 17);
						/* Include promotions (one square before column a) */
						if (col == 1 && color[i - 1] == EMPTY)
							gen_push(i, i - 1, 16);
					}
				}
				if (side == DARK) {
					/* Black pawns capture diagonally to the east */
					if (col < 7) {
						/* Up-right and down-right captures */
						if (row > 0 && color[i - 7] == LIGHT)
							gen_push(i, i - 7, 17);
						if (row < 7 && color[i + 9] == LIGHT)
							gen_push(i, i + 9, 17);
						/* Include promotions (one square before column h) */
						if (col == 6 && color[i + 1] == EMPTY)
							gen_push(i, i + 1, 16);
					}
				}
			}
			else
				for (j = 0; j < offsets[piece[i]]; ++j)
					for (n = i;;) {
						n = mailbox[mailbox64[n] + offset[piece[i]][j]];
						if (n == -1)
							break;
						if (color[n] != EMPTY) {
							if (color[n] == xside)
								gen_push(i, n, 1);
							break;
						}
						if (!slide[piece[i]])
							break;
					}
		}
	/* No en passant in gravity chess */
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

	/* Check for pawn promotion in gravity chess */
	if (bits & 16) {
		if (side == LIGHT) {
			/* White promotes on column a (COL = 0) */
			if (COL(to) == 0) {
				gen_promote(from, to, bits);
				return;
			}
		}
		else {
			/* Black promotes on column h (COL = 7) */
			if (COL(to) == 7) {
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
		case 62:
			if (color[F1] != EMPTY || color[G1] != EMPTY ||
				attack(F1, xside) || attack(G1, xside))
				return FALSE;
			from = H1;
			to = F1;
			break;
		case 58:
			if (color[B1] != EMPTY || color[C1] != EMPTY || color[D1] != EMPTY ||
				attack(C1, xside) || attack(D1, xside))
				return FALSE;
			from = A1;
			to = D1;
			break;
		case 6:
			if (color[F8] != EMPTY || color[G8] != EMPTY ||
				attack(F8, xside) || attack(G8, xside))
				return FALSE;
			from = H8;
			to = F8;
			break;
		case 2:
			if (color[B8] != EMPTY || color[C8] != EMPTY || color[D8] != EMPTY ||
				attack(C8, xside) || attack(D8, xside))
				return FALSE;
			from = A8;
			to = D8;
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
	hist_dat[hply].gravity_to = -1;  /* will be updated if gravity applies */
	hist_dat[hply].fall_from = -1;   /* will be updated if piece falls from above */
	hist_dat[hply].fall_to = -1;     /* will be updated if piece falls from above */
	
	/* Check if this is a capture (before we overwrite the destination) */
	BOOL is_capture = (hist_dat[hply].capture != EMPTY);
	
	++ply;
	++hply;

	/* update the castle, en passant, and
	   fifty-move-draw variables */
	castle &= castle_mask[(int)m.from] & castle_mask[(int)m.to];
	/* No en passant in gravity chess */
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
			color[m.to + 8] = EMPTY;
			piece[m.to + 8] = EMPTY;
		}
		else {
			color[m.to - 8] = EMPTY;
			piece[m.to - 8] = EMPTY;
		}
	}

	/* Apply gravity if the piece landed on an empty square (not a capture) */
	if (!is_capture) {
		int gravity_sq = apply_gravity(m.to);
		if (gravity_sq != m.to) {
			/* Piece fell to a different square */
			hist_dat[hply - 1].gravity_to = gravity_sq;
			color[gravity_sq] = color[(int)m.to];
			piece[gravity_sq] = piece[(int)m.to];
			color[(int)m.to] = EMPTY;
			piece[(int)m.to] = EMPTY;
		}
	}

	/* Check if a piece from above should fall to the starting square */
	/* m.from is now empty, check if rank < 8 (row > 0) and if there's a piece above */
	int from_row = ROW(m.from);
	if (from_row > 0) {  /* rank < 8 */
		int above_sq = m.from - 8;  /* one rank higher (rank n+1) */
		if (color[above_sq] != EMPTY) {
			/* There's a piece above, make it fall to m.from */
			hist_dat[hply - 1].fall_from = above_sq;
			
			/* DEBUG */
			/* printf("DEBUG: Piece at sq %d falling to sq %d\n", above_sq, m.from); */
			
			/* Move the piece to m.from first */
			color[(int)m.from] = color[above_sq];
			piece[(int)m.from] = piece[above_sq];
			color[above_sq] = EMPTY;
			piece[above_sq] = EMPTY;
			
			/* Then apply gravity to make it fall further if possible */
			int fall_sq = apply_gravity(m.from);
			hist_dat[hply - 1].fall_to = fall_sq;
			
			/* DEBUG */
			/* printf("DEBUG: After gravity, piece at sq %d\n", fall_sq); */
			
			if (fall_sq != m.from) {
				/* Piece fell further */
				color[fall_sq] = color[(int)m.from];
				piece[fall_sq] = piece[(int)m.from];
				color[(int)m.from] = EMPTY;
				piece[(int)m.from] = EMPTY;
			}
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
	int actual_to;

	side ^= 1;
	xside ^= 1;
	--ply;
	--hply;
	m = hist_dat[hply].m.b;
	castle = hist_dat[hply].castle;
	ep = hist_dat[hply].ep;
	fifty = hist_dat[hply].fifty;
	hash = hist_dat[hply].hash;
	
	/* First, undo any piece that fell from above */
	if (hist_dat[hply].fall_from != -1) {
		int fall_from = hist_dat[hply].fall_from;
		int fall_to = hist_dat[hply].fall_to;
		
		/* The piece is currently at fall_to, restore it to fall_from */
		color[fall_from] = color[fall_to];
		piece[fall_from] = piece[fall_to];
		color[fall_to] = EMPTY;
		piece[fall_to] = EMPTY;
	}
	
	/* If gravity was applied, the piece is actually at gravity_to, not m.to */
	actual_to = (hist_dat[hply].gravity_to != -1) ? hist_dat[hply].gravity_to : m.to;
	
	color[(int)m.from] = side;
	if (m.bits & 32)
		piece[(int)m.from] = PAWN;
	else
		piece[(int)m.from] = piece[actual_to];
	
	/* Clear the actual destination square */
	color[actual_to] = EMPTY;
	piece[actual_to] = EMPTY;
	
	/* Restore the captured piece at the original destination (not gravity destination) */
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
		case 62:
			from = F1;
			to = H1;
			break;
		case 58:
			from = D1;
			to = A1;
			break;
		case 6:
			from = F8;
			to = H8;
			break;
		case 2:
			from = D8;
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
			color[m.to + 8] = xside;
			piece[m.to + 8] = PAWN;
		}
		else {
			color[m.to - 8] = xside;
			piece[m.to - 8] = PAWN;
		}
	}
}


/* apply_gravity() makes a piece fall down due to gravity.
   Returns the final square after falling. If a piece lands on an
   empty square, it continues to fall until it hits another piece
   or reaches rank 1.
   In the coordinate system: rank 8 is row 0 (squares 0-7), rank 1 is row 7 (squares 56-63)
   Gravity pulls pieces toward rank 1, which means increasing row number. */

int apply_gravity(int sq)
{
	int current = sq;
	
	/* Keep falling toward rank 1 (row 7) */
	while (ROW(current) < 7) {
		int below = current + 8;  /* next rank down (toward rank 1) */
		if (color[below] != EMPTY)
			break;  /* hit another piece */
		current = below;
	}
	
	return current;
}
