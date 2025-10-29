/*
 *	DATA.C
 *	Tom Kerrigan's Simple Chess Program (TSCP)
 *
 *	Copyright 1997 Tom Kerrigan
 */


#include "defs.h"


/* the board representation */
int color[80];  /* LIGHT, DARK, or EMPTY */
int piece[80];  /* PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, COMMONER, AMAZON, or EMPTY */
int side;  /* the side to move */
int xside;  /* the side not to move */
int castle;  /* a bitfield with the castle permissions. if 1 is set,
                white can still castle kingside. 2 is white queenside.
				4 is black kingside. 8 is black queenside. */
int ep;  /* the en passant square. if white moves e2e4, the en passant
            square is set to e3, because that's where a pawn would move
			in an en passant capture */
int fifty;  /* the number of moves since a capture or pawn move, used
               to handle the fifty-move-draw rule */
int hash;  /* a (more or less) unique number that corresponds to the
              position */
int ply;  /* the number of half-moves (ply) since the
             root of the search tree */
int hply;  /* h for history; the number of ply since the beginning
              of the game */

/* gen_dat is some memory for move lists that are created by the move
   generators. The move list for ply n starts at first_move[n] and ends
   at first_move[n + 1]. */
gen_t gen_dat[GEN_STACK];
int first_move[MAX_PLY];

/* the history heuristic array (used for move ordering) */
int history[80][80];

/* we need an array of hist_t's so we can take back the
   moves we make */
hist_t hist_dat[HIST_STACK];

/* the engine will search for max_time milliseconds or until it finishes
   searching max_depth ply. */
int max_time;
int max_depth;

/* the time when the engine starts searching, and when it should stop */
int start_time;
int stop_time;

int nodes;  /* the number of nodes we've searched */

/* a "triangular" PV array; for a good explanation of why a triangular
   array is needed, see "How Computers Play Chess" by Levy and Newborn. */
move pv[MAX_PLY][MAX_PLY];
int pv_length[MAX_PLY];
BOOL follow_pv;

/* random numbers used to compute hash; see set_hash() in board.c */
int hash_piece[2][8][80];  /* indexed by piece [color][type][square] */
int hash_side;
int hash_ep[80];

/* Now we have the mailbox array, so called because it looks like a
   mailbox, at least according to Bob Hyatt. This is useful when we
   need to figure out what pieces can go where. Let's say we have a
   rook on square a4 (32) and we want to know if it can move one
   square to the left. We subtract 1, and we get 31 (h5). The rook
   obviously can't move to h5, but we don't know that without doing
   a lot of annoying work. Sooooo, what we do is figure out a4's
   mailbox number, which is 61. Then we subtract 1 from 61 (60) and
   see what mailbox[60] is. In this case, it's -1, so it's out of
   bounds and we can forget it. You can see how mailbox[] is used
   in attack() in board.c. */

int mailbox[156] = {
	 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	 -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1,
	 -1, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, -1,
	 -1, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, -1,
	 -1, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, -1,
	 -1, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, -1,
	 -1, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, -1,
	 -1, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, -1,
	 -1, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, -1,
	 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

int mailbox64[80] = {
	25, 26, 27, 28, 29, 30, 31, 32, 33, 34,
	37, 38, 39, 40, 41, 42, 43, 44, 45, 46,
	49, 50, 51, 52, 53, 54, 55, 56, 57, 58,
	61, 62, 63, 64, 65, 66, 67, 68, 69, 70,
	73, 74, 75, 76, 77, 78, 79, 80, 81, 82,
	85, 86, 87, 88, 89, 90, 91, 92, 93, 94,
	97, 98, 99, 100, 101, 102, 103, 104, 105, 106,
	109, 110, 111, 112, 113, 114, 115, 116, 117, 118
};


/* slide, offsets, and offset are basically the vectors that
   pieces can move in. If slide for the piece is FALSE, it can
   only move one square in any one direction. offsets is the
   number of directions it can move in, and offset is an array
   of the actual directions. For 10x8 board: row offset = 12 (mailbox width)
   Knight: row±1,col±2 or row±2,col±1
   Bishop: diagonal = ±11, ±13
   Rook: horizontal/vertical = ±1, ±12
   Queen/King: all 8 directions = ±1, ±11, ±12, ±13
   Commoner: like King
   Amazon: Queen + Knight (slides like Queen, jumps like Knight) */

BOOL slide[8] = {
	FALSE, FALSE, TRUE, TRUE, TRUE, FALSE, FALSE, TRUE
};

int offsets[8] = {
	0, 8, 4, 4, 8, 8, 8, 8
};

int offset[8][8] = {
	{ 0, 0, 0, 0, 0, 0, 0, 0 },           /* PAWN */
	{ -25, -23, -14, -10, 10, 14, 23, 25 }, /* KNIGHT */
	{ -13, -11, 11, 13, 0, 0, 0, 0 },     /* BISHOP */
	{ -12, -1, 1, 12, 0, 0, 0, 0 },       /* ROOK */
	{ -13, -12, -11, -1, 1, 11, 12, 13 }, /* QUEEN */
	{ -13, -12, -11, -1, 1, 11, 12, 13 }, /* KING */
	{ -13, -12, -11, -1, 1, 11, 12, 13 }, /* COMMONER */
	{ -13, -12, -11, -1, 1, 11, 12, 13 }  /* AMAZON (queen moves, knight handled separately) */
};


/* This is the castle_mask array. We can use it to determine
   the castling permissions after a move. What we do is
   logical-AND the castle bits with the castle_mask bits for
   both of the move's squares. Let's say castle is 1, meaning
   that white can still castle kingside. Now we play a move
   where the rook on j1 gets captured. We AND castle with
   castle_mask[79], so we have 1&14, and castle becomes 0 and
   white can't castle kingside anymore. 
   For 10x8 board: King on f-file: A1=70, F1=75 (king), J1=79, A8=0, F8=5 (king), J8=9
   Castle bits: 1=white kingside, 2=white queenside, 4=black kingside, 8=black queenside */

int castle_mask[80] = {
	 7, 15, 15, 15, 15,  3, 15, 15, 15, 11,  /* rank 8: a8=7, f8=3, j8=11 */
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15,  /* rank 7 */
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15,  /* rank 6 */
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15,  /* rank 5 */
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15,  /* rank 4 */
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15,  /* rank 3 */
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15,  /* rank 2 */
	13, 15, 15, 15, 15, 12, 15, 15, 15, 14   /* rank 1: a1=13, f1=12, j1=14 */
};


/* the piece letters, for print_board() */
char piece_char[8] = {
	'P', 'N', 'B', 'R', 'Q', 'K', 'G', 'W'
};


/* the initial board state 
   FEN: rgnbkqbnwr/pppppppppp/10/10/10/10/PPPPPPPPPP/RWNBQKBNGR w KQkq - 0 1
   Rank 8 (0-9): r g n b k q b n w r
   Rank 7 (10-19): p p p p p p p p p p
   Ranks 6-3 (20-59): empty
   Rank 2 (60-69): P P P P P P P P P P
   Rank 1 (70-79): R W N B Q K B N G R */

int init_color[80] = {
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  /* rank 8 */
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  /* rank 7 */
	8, 8, 8, 8, 8, 8, 8, 8, 8, 8,  /* rank 6 */
	8, 8, 8, 8, 8, 8, 8, 8, 8, 8,  /* rank 5 */
	8, 8, 8, 8, 8, 8, 8, 8, 8, 8,  /* rank 4 */
	8, 8, 8, 8, 8, 8, 8, 8, 8, 8,  /* rank 3 */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* rank 2 */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0   /* rank 1 */
};

int init_piece[80] = {
	3, 6, 1, 2, 5, 4, 2, 1, 7, 3,  /* rank 8: r g n b k q b n w r */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* rank 7: pawns */
	8, 8, 8, 8, 8, 8, 8, 8, 8, 8,  /* rank 6 */
	8, 8, 8, 8, 8, 8, 8, 8, 8, 8,  /* rank 5 */
	8, 8, 8, 8, 8, 8, 8, 8, 8, 8,  /* rank 4 */
	8, 8, 8, 8, 8, 8, 8, 8, 8, 8,  /* rank 3 */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* rank 2: pawns */
	3, 7, 1, 2, 4, 5, 2, 1, 6, 3   /* rank 1: R W N B Q K B N G R */
};

/* transparent array marks squares that are temporarily transparent 
   (own W pieces and adjacent squares) */
BOOL transparent[80];
