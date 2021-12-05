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


#define DOUBLED_PAWN_PENALTY      10
#define ISOLATED_PAWN_PENALTY     20
#define BACKWARDS_PAWN_PENALTY     8
#define PASSED_PAWN_BONUS         20
#define ROOK_SEMI_OPEN_FILE_BONUS 10
#define ROOK_OPEN_FILE_BONUS      15
#define ROOK_ON_SEVENTH_BONUS     20


/* the values of the pieces */
int piece_value[PieceLen] = {
	900, 850, 250, 60, -150, -2700  
};

/* The "pcsq" arrays are piece/square tables. They're values
   added to the material value of the piece based on the
   location of the piece. */

int pawn_pcsq[SqLen] = {
	  0,   0,   0,   0,   0,   0,   0,   0,
	  5,  10,  15,  20,  20,  15,  10,   5,
	  4,   8,  12,  16,  16,  12,   8,   4,
	  3,   6,   9,  12,  12,   9,   6,   3,
	  2,   4,   6,   8,   8,   6,   4,   2,
	  1,   2,   3, -10, -10,   3,   2,   1,
	  0,   0,   0, -40, -40,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0
};

int knight_pcsq[SqLen] = {
	-10, -10, -10, -10, -10, -10, -10, -10,
	-10,   0,   0,   0,   0,   0,   0, -10,
	-10,   0,   5,   5,   5,   5,   0, -10,
	-10,   0,   5,  10,  10,   5,   0, -10,
	-10,   0,   5,  10,  10,   5,   0, -10,
	-10,   0,   5,   5,   5,   5,   0, -10,
	-10,   0,   0,   0,   0,   0,   0, -10,
	-10, -30, -10, -10, -10, -10, -30, -10
};

int bishop_pcsq[SqLen] = {
	-10, -10, -10, -10, -10, -10, -10, -10,
	-10,   0,   0,   0,   0,   0,   0, -10,
	-10,   0,   5,   5,   5,   5,   0, -10,
	-10,   0,   5,  10,  10,   5,   0, -10,
	-10,   0,   5,  10,  10,   5,   0, -10,
	-10,   0,   5,   5,   5,   5,   0, -10,
	-10,   0,   0,   0,   0,   0,   0, -10,
	-10, -10, -20, -10, -10, -20, -10, -10
};

int king_pcsq[SqLen] = {
	-10, -10, -10, -10, -10, -10, -10, -10,
	-10, -20, -20, -20, -20, -20, -20, -10,
	-10, -20, -40, -40, -40, -40, -20, -10,
	-10, -20, -40, -70, -70, -40, -20, -10,
	-10, -20, -40, -70, -70, -40, -20, -10,
	-10, -20, -40, -40, -40, -40, -20, -10,
	-10, -20, -20, -20, -20, -20, -20, -10,
	 -10,  -10,  -10, -10,  -10, -10,  -10,  -10
};

int king_endgame_pcsq[SqLen] = {
-10, -10, -10, -10, -10, -10, -10, -10,
	-10, -20, -20, -20, -20, -20, -20, -10,
	-10, -20, -40, -40, -40, -40, -20, -10,
	-10, -20, -40, -70, -70, -40, -20, -10,
	-10, -20, -40, -70, -70, -40, -20, -10,
	-10, -20, -40, -40, -40, -40, -20, -10,
	-10, -20, -20, -20, -20, -20, -20, -10,
	 -10,  -10,  -10, -10,  -10, -10,  -10,  -10
};

/* The flip array is used to calculate the piece/square
   values for DARK pieces. The piece/square value of a
   LIGHT pawn is pawn_pcsq[sq] and the value of a DARK
   pawn is pawn_pcsq[flip[sq]] */
int flip[SqLen] = {
	 56,  57,  58,  59,  60,  61,  62,  63,
	 48,  49,  50,  51,  52,  53,  54,  55,
	 40,  41,  42,  43,  44,  45,  46,  47,
	 32,  33,  34,  35,  36,  37,  38,  39,
	 24,  25,  26,  27,  28,  29,  30,  31,
	 16,  17,  18,  19,  20,  21,  22,  23,
	  8,   9,  10,  11,  12,  13,  14,  15,
	  0,   1,   2,   3,   4,   5,   6,   7
};

/* pawn_rank[x][y] is the rank of the least advanced pawn of color x on file
   y - 1. There are "buffer files" on the left and right to avoid special-case
   logic later. If there's no pawn on a rank, we pretend the pawn is
   impossibly far advanced (0 for LIGHT and 7 for DARK). This makes it easy to
   test for pawns on a rank and it simplifies some pawn evaluation code. */
int pawn_rank[ColorLen][10];

int piece_mat[ColorLen];  /* the value of a side's pieces */
int pawn_mat[ColorLen];  /* the value of a side's pawns */

int eval()
{
	int sq;
	int file;  /* file */
	int score[ColorLen];  /* each side's score */

	/* this is the first pass: set up pawn_rank, piece_mat, and pawn_mat. */
	for (sq = 0; sq < 10; ++sq) {
		pawn_rank[LIGHT][sq] = 0;
		pawn_rank[DARK][sq] = 7;
	}
	piece_mat[LIGHT] = 0;
	piece_mat[DARK] = 0;
	pawn_mat[LIGHT] = 0;
	pawn_mat[DARK] = 0;
	for (sq = 0; sq < SqLen; ++sq) {
		if (color[sq] == NoColor)
			continue;
		if (piece[sq] == PAWN) {
			pawn_mat[color[sq]] += piece_value[PAWN];
			file = MapToFile(sq) + 1;  /* add 1 because of the extra file in the array */
			if (color[sq] == LIGHT) {
				if (pawn_rank[LIGHT][file] < MapToRank(sq))
					pawn_rank[LIGHT][file] = MapToRank(sq);
			}
			else {
				if (pawn_rank[DARK][file] > MapToRank(sq))
					pawn_rank[DARK][file] = MapToRank(sq);
			}
		}
		else
			piece_mat[color[sq]] += piece_value[piece[sq]];
	}

	/* this is the second pass: evaluate each piece */
	score[LIGHT] = piece_mat[LIGHT] + pawn_mat[LIGHT];
	score[DARK] = piece_mat[DARK] + pawn_mat[DARK];
	for (sq = 0; sq < SqLen; ++sq) {
		if (color[sq] == NoColor)
			continue;
		if (color[sq] == LIGHT) {
			switch (piece[sq]) {
				case PAWN:
					score[LIGHT] += eval_light_pawn(sq);
					break;
				case KNIGHT:
					score[LIGHT] += knight_pcsq[sq];
					break;
				case BISHOP:
					score[LIGHT] += bishop_pcsq[sq];
					break;
				case ROOK:
					if (pawn_rank[LIGHT][MapToFile(sq) + 1] == 0) {
						if (pawn_rank[DARK][MapToFile(sq) + 1] == 7)
							score[LIGHT] += ROOK_OPEN_FILE_BONUS;
						else
							score[LIGHT] += ROOK_SEMI_OPEN_FILE_BONUS;
					}
					if (MapToRank(sq) == Rank7)
						score[LIGHT] += ROOK_ON_SEVENTH_BONUS;
					break;
				case KING:
					if (piece_mat[DARK] <= 1200)
						score[LIGHT] += king_endgame_pcsq[sq];
					else
						score[LIGHT] += eval_light_king(sq);
					break;
			}
		}
		else {
			switch (piece[sq]) {
				case PAWN:
					score[DARK] += eval_dark_pawn(sq);
					break;
				case KNIGHT:
					score[DARK] += knight_pcsq[flip[sq]];
					break;
				case BISHOP:
					score[DARK] += bishop_pcsq[flip[sq]];
					break;
				case ROOK:
					if (pawn_rank[DARK][MapToFile(sq) + 1] == 7) {
						if (pawn_rank[LIGHT][MapToFile(sq) + 1] == 0)
							score[DARK] += ROOK_OPEN_FILE_BONUS;
						else
							score[DARK] += ROOK_SEMI_OPEN_FILE_BONUS;
					}
					if (MapToRank(sq) == Rank2)
						score[DARK] += ROOK_ON_SEVENTH_BONUS;
					break;
				case KING:
					if (piece_mat[LIGHT] <= 1200)
						score[DARK] += king_endgame_pcsq[flip[sq]];
					else
						score[DARK] += eval_dark_king(sq);
					break;
			}
		}
	}

	/* the score[] array is set, now return the score relative
	   to the side to move */
	if (side == LIGHT)
		return score[LIGHT] - score[DARK];
	return score[DARK] - score[LIGHT];
}

int eval_light_pawn(int sq)
{
	int r;  /* the value to return */
	int f;  /* the pawn's file */

	r = 0;
	f = MapToFile(sq) + 1;

	r += pawn_pcsq[sq];

	/* if there's a pawn behind this one, it's doubled */
	if (pawn_rank[LIGHT][f] > MapToRank(sq))
		r -= DOUBLED_PAWN_PENALTY;

	/* if there aren't any friendly pawns on either side of
	   this one, it's isolated */
	if ((pawn_rank[LIGHT][f - 1] == 0) &&
			(pawn_rank[LIGHT][f + 1] == 0))
		r -= ISOLATED_PAWN_PENALTY;

	/* if it's not isolated, it might be backwards */
	else if ((pawn_rank[LIGHT][f - 1] < MapToRank(sq)) &&
			(pawn_rank[LIGHT][f + 1] < MapToRank(sq)))
		r -= BACKWARDS_PAWN_PENALTY;

	/* add a bonus if the pawn is passed */
	if ((pawn_rank[DARK][f - 1] >= MapToRank(sq)) &&
			(pawn_rank[DARK][f] >= MapToRank(sq)) &&
			(pawn_rank[DARK][f + 1] >= MapToRank(sq)))
		r += (Rank1 - MapToRank(sq)) * PASSED_PAWN_BONUS;

	return r;
}

int eval_dark_pawn(int sq)
{
	int r;  /* the value to return */
	int f;  /* the pawn's file */

	r = 0;
	f = MapToFile(sq) + 1;

	r += pawn_pcsq[flip[sq]];

	/* if there's a pawn behind this one, it's doubled */
	if (pawn_rank[DARK][f] < MapToRank(sq))
		r -= DOUBLED_PAWN_PENALTY;

	/* if there aren't any friendly pawns on either side of
	   this one, it's isolated */
	if ((pawn_rank[DARK][f - 1] == 7) &&
			(pawn_rank[DARK][f + 1] == 7))
		r -= ISOLATED_PAWN_PENALTY;

	/* if it's not isolated, it might be backwards */
	else if ((pawn_rank[DARK][f - 1] > MapToRank(sq)) &&
			(pawn_rank[DARK][f + 1] > MapToRank(sq)))
		r -= BACKWARDS_PAWN_PENALTY;

	/* add a bonus if the pawn is passed */
	if ((pawn_rank[LIGHT][f - 1] <= MapToRank(sq)) &&
			(pawn_rank[LIGHT][f] <= MapToRank(sq)) &&
			(pawn_rank[LIGHT][f + 1] <= MapToRank(sq)))
		r += MapToRank(sq) * PASSED_PAWN_BONUS;

	return r;
}

int eval_light_king(int sq)
{
	int r;  /* the value to return */
	int i;

	r = king_pcsq[sq];

	/* if the king is castled, use a special function to evaluate the
	   pawns on the appropriate side */
	if (MapToFile(sq) < 3) {
		r += eval_lkp(1);
		r += eval_lkp(2);
		r += eval_lkp(3) / 2;  /* problems with pawns on the c & f files
								  are not as severe */
	}
	else if (MapToFile(sq) > 4) {
		r += eval_lkp(8);
		r += eval_lkp(7);
		r += eval_lkp(6) / 2;
	}

	/* otherwise, just assess a penalty if there are open files near
	   the king */
	else {
		for (i = MapToFile(sq); i <= MapToFile(sq) + 2; ++i)
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
	if (MapToFile(sq) < 3) {
		r += eval_dkp(1);
		r += eval_dkp(2);
		r += eval_dkp(3) / 2;
	}
	else if (MapToFile(sq) > 4) {
		r += eval_dkp(8);
		r += eval_dkp(7);
		r += eval_dkp(6) / 2;
	}
	else {
		for (i = MapToFile(sq); i <= MapToFile(sq) + 2; ++i)
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
