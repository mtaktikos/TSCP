/*
 *	EVAL.C
 *	Tom Kerrigan's Simple Chess Program (TSCP)
 *
 *	Copyright 1997 Tom Kerrigan
 */


#include <string.h>
#include "defs.h"
#include "data.h"
#include "protos.h"


#define DOUBLED_PAWN_PENALTY		10
#define ISOLATED_PAWN_PENALTY		20
#define BACKWARDS_PAWN_PENALTY		8
#define PASSED_PAWN_BONUS			20
#define ROOK_SEMI_OPEN_FILE_BONUS	10
#define ROOK_OPEN_FILE_BONUS		15
#define ROOK_ON_SEVENTH_BONUS		20
#define KING_WIN_SCORE				3500  /* Score for winning by King capture */


/* the values of the pieces */
int piece_value[8] = {
	100, 300, 380, 550, 980, KING_WIN_SCORE, 250, 530
};

/* The "pcsq" arrays are piece/square tables. They're values
   added to the material value of the piece based on the
   location of the piece. Extended for 10x8 board */

int pawn_pcsq[80] = {
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	  5,  10,  15,  20,  20,  20,  20,  15,  10,   5,
	  4,   8,  12,  16,  16,  16,  16,  12,   8,   4,
	  3,   6,   9,  12,  12,  12,  12,   9,   6,   3,
	  2,   4,   6,   8,   8,   8,   8,   6,   4,   2,
	  1,   2,   3, -10, -10, -10, -10,   3,   2,   1,
	  0,   0,   0, -40, -40, -40, -40,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};
 
int knight_pcsq[80] = {
	-10, -10, -10, -10, -10, -10, -10, -10, -10, -10,
	-10,   0,   0,   0,   0,   0,   0,   0,   0, -10,
	-10,   0,   5,   5,   5,   5,   5,   5,   0, -10,
	-10,   0,   5,  10,  10,  10,  10,   5,   0, -10,
	-10,   0,   5,  10,  10,  10,  10,   5,   0, -10,
	-10,   0,   5,   5,   5,   5,   5,   5,   0, -10,
	-10,   0,   0,   0,   0,   0,   0,   0,   0, -10,
	-10, -30, -10, -10, -10, -10, -10, -10, -30, -10
};

int bishop_pcsq[80] = {
	-10, -10, -10, -10, -10, -10, -10, -10, -10, -10,
	-10,   0,   0,   0,   0,   0,   0,   0,   0, -10,
	-10,   0,   5,   5,   5,   5,   5,   5,   0, -10,
	-10,   0,   5,  10,  10,  10,  10,   5,   0, -10,
	-10,   0,   5,  10,  10,  10,  10,   5,   0, -10,
	-10,   0,   5,   5,   5,   5,   5,   5,   0, -10,
	-10,   0,   0,   0,   0,   0,   0,   0,   0, -10,
	-10, -10, -20, -10, -10, -10, -10, -20, -10, -10
};

int king_pcsq[80] = {
	-50, -50, -50, -50, -50, -50, -50, -50, -50, -50,
	-50, -50, -50, -50, -50, -50, -50, -50, -50, -50,
	-50, -50, -50, -50, -50, -50, -50, -50, -50, -50,
	-50, -50, -50, -50, -50, -50, -50, -50, -50, -50,
	-45, -45, -45, -45, -45, -45, -45, -45, -45, -45,
	-40, -40, -40, -40, -40, -40, -40, -40, -40, -40,
	-20, -20, -20, -20, -20, -20, -20, -20, -20, -20,
	  0,  0,  0, 0,   0,   0, 0,  0,  0,   0
};

int king_endgame_pcsq[80] = {
	  0,  10,  20,  30,  30,  30,  30,  20,  10,   0,
	 10,  20,  30,  40,  40,  40,  40,  30,  20,  10,
	 20,  30,  40,  50,  50,  50,  50,  40,  30,  20,
	 30,  40,  50,  60,  60,  60,  60,  50,  40,  30,
	 30,  40,  50,  60,  60,  60,  60,  50,  40,  30,
	 20,  30,  40,  50,  50,  50,  50,  40,  30,  20,
	 10,  20,  30,  40,  40,  40,  40,  30,  20,  10,
	  0,  10,  20,  30,  30,  30,  30,  20,  10,   0
};

/* The flip array is used to calculate the piece/square
   values for DARK pieces. The piece/square value of a
   LIGHT pawn is pawn_pcsq[sq] and the value of a DARK
   pawn is pawn_pcsq[flip[sq]] */
int flip[80] = {
	 70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
	 60,  61,  62,  63,  64,  65,  66,  67,  68,  69,
	 50,  51,  52,  53,  54,  55,  56,  57,  58,  59,
	 40,  41,  42,  43,  44,  45,  46,  47,  48,  49,
	 30,  31,  32,  33,  34,  35,  36,  37,  38,  39,
	 20,  21,  22,  23,  24,  25,  26,  27,  28,  29,
	 10,  11,  12,  13,  14,  15,  16,  17,  18,  19,
	  0,   1,   2,   3,   4,   5,   6,   7,   8,   9
};

/* pawn_rank[x][y] is the rank of the least advanced pawn of color x on file
   y - 1. There are "buffer files" on the left and right to avoid special-case
   logic later. If there's no pawn on a rank, we pretend the pawn is
   impossibly far advanced (0 for LIGHT and 7 for DARK). This makes it easy to
   test for pawns on a rank and it simplifies some pawn evaluation code. */
int pawn_rank[2][12];

int piece_mat[2];  /* the value of a side's pieces */
int pawn_mat[2];  /* the value of a side's pawns */

int eval()
{
	int i;
	int f;  /* file */
	int score[2];  /* each side's score */
	int captured_king;

	/* Check if a King has been captured - this ends the game */
	captured_king = king_captured();
	if (captured_king == LIGHT)
		return (side == DARK) ? KING_WIN_SCORE : -KING_WIN_SCORE;  /* Light King captured, Dark wins */
	if (captured_king == DARK)
		return (side == LIGHT) ? KING_WIN_SCORE : -KING_WIN_SCORE;  /* Dark King captured, Light wins */

	/* Check if either King is under attack (can be captured on the next move) */
	/* If opponent's King is under attack, we can capture it - winning position */
	if (in_check(xside))
		return KING_WIN_SCORE;
	/* If our King is under attack, opponent can capture it - losing position */
	if (in_check(side))
		return -KING_WIN_SCORE;

	/* this is the first pass: set up pawn_rank, piece_mat, and pawn_mat. */
	for (i = 0; i < 12; ++i) {
		pawn_rank[LIGHT][i] = 0;
		pawn_rank[DARK][i] = 7;
	}
	piece_mat[LIGHT] = 0;
	piece_mat[DARK] = 0;
	pawn_mat[LIGHT] = 0;
	pawn_mat[DARK] = 0;
	for (i = 0; i < 80; ++i) {
		if (color[i] == EMPTY)
			continue;
		if (piece[i] == PAWN) {
			pawn_mat[color[i]] += piece_value[PAWN];
			f = COL(i) + 1;  /* add 1 because of the extra file in the array */
			if (color[i] == LIGHT) {
				if (pawn_rank[LIGHT][f] < ROW(i))
					pawn_rank[LIGHT][f] = ROW(i);
			}
			else {
				if (pawn_rank[DARK][f] > ROW(i))
					pawn_rank[DARK][f] = ROW(i);
			}
		}
		else
			piece_mat[color[i]] += piece_value[piece[i]];
	}

	/* this is the second pass: evaluate each piece */
	score[LIGHT] = piece_mat[LIGHT] + pawn_mat[LIGHT];
	score[DARK] = piece_mat[DARK] + pawn_mat[DARK];
	for (i = 0; i < 80; ++i) {
		if (color[i] == EMPTY)
			continue;
		if (color[i] == LIGHT) {
			switch (piece[i]) {
				case PAWN:
					score[LIGHT] += eval_light_pawn(i);
					break;
				case KNIGHT:
					score[LIGHT] += knight_pcsq[i];
					break;
				case BISHOP:
					score[LIGHT] += bishop_pcsq[i];
					break;
				case ROOK:
					if (pawn_rank[LIGHT][COL(i) + 1] == 0) {
						if (pawn_rank[DARK][COL(i) + 1] == 7)
							score[LIGHT] += ROOK_OPEN_FILE_BONUS;
						else
							score[LIGHT] += ROOK_SEMI_OPEN_FILE_BONUS;
					}
					if (ROW(i) == 1)
						score[LIGHT] += ROOK_ON_SEVENTH_BONUS;
					break;
				case KING:
					if (piece_mat[DARK] <= 1200)
						score[LIGHT] += king_endgame_pcsq[i];
					else
						score[LIGHT] += eval_light_king(i);
					break;
				case COMMONER:
					/* Evaluate commoner similarly to king in endgame */
					score[LIGHT] += king_endgame_pcsq[i] / 2;
					break;
				case AMAZON:
					/* Amazon is very powerful, no positional bonus needed */
					break;
			}
		}
		else {
			switch (piece[i]) {
				case PAWN:
					score[DARK] += eval_dark_pawn(i);
					break;
				case KNIGHT:
					score[DARK] += knight_pcsq[flip[i]];
					break;
				case BISHOP:
					score[DARK] += bishop_pcsq[flip[i]];
					break;
				case ROOK:
					if (pawn_rank[DARK][COL(i) + 1] == 7) {
						if (pawn_rank[LIGHT][COL(i) + 1] == 0)
							score[DARK] += ROOK_OPEN_FILE_BONUS;
						else
							score[DARK] += ROOK_SEMI_OPEN_FILE_BONUS;
					}
					if (ROW(i) == 6)
						score[DARK] += ROOK_ON_SEVENTH_BONUS;
					break;
				case KING:
					if (piece_mat[LIGHT] <= 1200)
						score[DARK] += king_endgame_pcsq[flip[i]];
					else
						score[DARK] += eval_dark_king(i);
					break;
				case COMMONER:
					/* Evaluate commoner similarly to king in endgame */
					score[DARK] += king_endgame_pcsq[flip[i]] / 2;
					break;
				case AMAZON:
					/* Amazon is very powerful, no positional bonus needed */
					break;
			}
		}
	}

	/* Add mobility evaluation (doubled as requested) */
	int mobility_score = eval_mobility();

	/* the score[] array is set, now return the score relative
	   to the side to move */
	if (side == LIGHT)
		return score[LIGHT] - score[DARK] + mobility_score;
	return score[DARK] - score[LIGHT] + mobility_score;
}

int eval_light_pawn(int sq)
{
	int r;  /* the value to return */
	int f;  /* the pawn's file */

	r = 0;
	f = COL(sq) + 1;

	r += pawn_pcsq[sq];

	/* if there's a pawn behind this one, it's doubled */
	if (pawn_rank[LIGHT][f] > ROW(sq))
		r -= DOUBLED_PAWN_PENALTY;

	/* if there aren't any friendly pawns on either side of
	   this one, it's isolated */
	if ((pawn_rank[LIGHT][f - 1] == 0) &&
			(pawn_rank[LIGHT][f + 1] == 0))
		r -= ISOLATED_PAWN_PENALTY;

	/* if it's not isolated, it might be backwards */
	else if ((pawn_rank[LIGHT][f - 1] < ROW(sq)) &&
			(pawn_rank[LIGHT][f + 1] < ROW(sq)))
		r -= BACKWARDS_PAWN_PENALTY;

	/* add a bonus if the pawn is passed */
	if ((pawn_rank[DARK][f - 1] >= ROW(sq)) &&
			(pawn_rank[DARK][f] >= ROW(sq)) &&
			(pawn_rank[DARK][f + 1] >= ROW(sq)))
		r += (7 - ROW(sq)) * PASSED_PAWN_BONUS;

	return r;
}

int eval_dark_pawn(int sq)
{
	int r;  /* the value to return */
	int f;  /* the pawn's file */

	r = 0;
	f = COL(sq) + 1;

	r += pawn_pcsq[flip[sq]];

	/* if there's a pawn behind this one, it's doubled */
	if (pawn_rank[DARK][f] < ROW(sq))
		r -= DOUBLED_PAWN_PENALTY;

	/* if there aren't any friendly pawns on either side of
	   this one, it's isolated */
	if ((pawn_rank[DARK][f - 1] == 7) &&
			(pawn_rank[DARK][f + 1] == 7))
		r -= ISOLATED_PAWN_PENALTY;

	/* if it's not isolated, it might be backwards */
	else if ((pawn_rank[DARK][f - 1] > ROW(sq)) &&
			(pawn_rank[DARK][f + 1] > ROW(sq)))
		r -= BACKWARDS_PAWN_PENALTY;

	/* add a bonus if the pawn is passed */
	if ((pawn_rank[LIGHT][f - 1] <= ROW(sq)) &&
			(pawn_rank[LIGHT][f] <= ROW(sq)) &&
			(pawn_rank[LIGHT][f + 1] <= ROW(sq)))
		r += ROW(sq) * PASSED_PAWN_BONUS;

	return r;
}

int eval_light_king(int sq)
{
	int r;  /* the value to return */
	int i;

	r = king_pcsq[sq];

	/* if the king is castled, use a special function to evaluate the
	   pawns on the appropriate side */
	if (COL(sq) < 3) {
		r += eval_lkp(1);
		r += eval_lkp(2);
		r += eval_lkp(3) / 2;  /* problems with pawns on the c file
								  are not as severe */
	}
	else if (COL(sq) > 6) {
		r += eval_lkp(10);
		r += eval_lkp(9);
		r += eval_lkp(8) / 2;
	}

	/* otherwise, just assess a penalty if there are open files near
	   the king */
	else {
		for (i = COL(sq); i <= COL(sq) + 2; ++i)
			if ((pawn_rank[LIGHT][i] == 0) &&
					(pawn_rank[DARK][i] == 7))
				r -= 10;
	}

	/* scale the king safety value according to the opponent's material;
	   the premise is that your king safety can only be bad if the
	   opponent has enough pieces to attack you */
	r *= piece_mat[DARK];
	r /= 3100;

	return r;
}

/* eval_lkp(f) evaluates the Light King Pawn on file f */

int eval_lkp(int f)
{
	int r = 0;

	if (pawn_rank[LIGHT][f] == 6);  /* pawn hasn't moved */
	else if (pawn_rank[LIGHT][f] == 5)
		r -= 10;  /* pawn moved one square */
	else if (pawn_rank[LIGHT][f] != 0)
		r -= 20;  /* pawn moved more than one square */
	else
		r -= 25;  /* no pawn on this file */

	if (pawn_rank[DARK][f] == 7)
		r -= 15;  /* no enemy pawn */
	else if (pawn_rank[DARK][f] == 5)
		r -= 10;  /* enemy pawn on the 3rd rank */
	else if (pawn_rank[DARK][f] == 4)
		r -= 5;   /* enemy pawn on the 4th rank */

	return r;
}

int eval_dark_king(int sq)
{
	int r;
	int i;

	r = king_pcsq[flip[sq]];
	if (COL(sq) < 3) {
		r += eval_dkp(1);
		r += eval_dkp(2);
		r += eval_dkp(3) / 2;
	}
	else if (COL(sq) > 6) {
		r += eval_dkp(10);
		r += eval_dkp(9);
		r += eval_dkp(8) / 2;
	}
	else {
		for (i = COL(sq); i <= COL(sq) + 2; ++i)
			if ((pawn_rank[LIGHT][i] == 0) &&
					(pawn_rank[DARK][i] == 7))
				r -= 10;
	}
	r *= piece_mat[LIGHT];
	r /= 3100;
	return r;
}

int eval_dkp(int f)
{
	int r = 0;

	if (pawn_rank[DARK][f] == 1);
	else if (pawn_rank[DARK][f] == 2)
		r -= 10;
	else if (pawn_rank[DARK][f] != 7)
		r -= 20;
	else
		r -= 25;

	if (pawn_rank[LIGHT][f] == 0)
		r -= 15;
	else if (pawn_rank[LIGHT][f] == 2)
		r -= 10;
	else if (pawn_rank[LIGHT][f] == 3)
		r -= 5;

	return r;
}

/* eval_mobility() counts the number of legal moves for the current side
   and returns the mobility score */
int eval_mobility()
{
	int i;
	int mobility[2];
	int saved_first_move;
	int saved_ply, saved_hply;
	int old_side, old_xside;
	int current_ply;
	
	mobility[LIGHT] = 0;
	mobility[DARK] = 0;
	
	/* Save current state */
	current_ply = ply;
	saved_first_move = first_move[current_ply];
	saved_ply = ply;
	saved_hply = hply;
	old_side = side;
	old_xside = xside;
	
	/* Count mobility for LIGHT */
	side = LIGHT;
	xside = DARK;
	ply = current_ply;  /* ensure ply is at saved value */
	first_move[ply + 1] = first_move[ply];
	gen();
	for (i = first_move[current_ply]; i < first_move[current_ply + 1]; ++i) {
		if (makemove(gen_dat[i].m.b)) {
			mobility[LIGHT]++;
			takeback();
		}
	}
	
	/* Count mobility for DARK */
	side = DARK;
	xside = LIGHT;
	ply = current_ply;  /* ensure ply is at saved value */
	first_move[ply + 1] = first_move[ply];
	gen();
	for (i = first_move[current_ply]; i < first_move[current_ply + 1]; ++i) {
		if (makemove(gen_dat[i].m.b)) {
			mobility[DARK]++;
			takeback();
		}
	}
	
	/* Restore state */
	side = old_side;
	xside = old_xside;
	first_move[current_ply] = saved_first_move;
	ply = saved_ply;
	hply = saved_hply;
	
	/* Return mobility difference doubled, relative to side to move */
	if (old_side == LIGHT)
		return (mobility[LIGHT] - mobility[DARK]) * 2;
	return (mobility[DARK] - mobility[LIGHT]) * 2;
}
