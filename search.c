/*
 *	SEARCH.C
 *	Tom Kerrigan's Simple Chess Program (TSCP)
 *
 *	Copyright 1997 Tom Kerrigan
 */


#include <stdio.h>
#include <string.h>
#include "defs.h"
#include "data.h"
#include "protos.h"


/* see the beginning of think() */
#include <setjmp.h>
jmp_buf env;
BOOL stop_search;


/* think() calls search() iteratively. Search statistics
   are printed depending on the value of output:
   0 = no output
   1 = normal output
   2 = xboard format output */

void think(int output)
{
	int iteration, score, plyindex;

	/* try the opening book first */
	pv[0][0].u = book_move();
	if (pv[0][0].u != MvNil)
		return;

	/* some code that lets us longjmp back here and return
	   from think() when our time is up */
	stop_search = FALSE;
	setjmp(env);
	if (stop_search) {

		/* make sure to take back the line we were searching */
		while (ply)
			takeback();
		return;
	}

	start_time = get_ms();
	stop_time = start_time + max_time;

	ply = 0;
	nodes = 0;

	memset(pv, 0, sizeof(pv));
	memset(history, 0, sizeof(history));
	if (output == PmConsole)
		printf("ply      nodes  score  pv\n");
	for (iteration = 1; iteration <= max_depth; ++iteration) {
		follow_pv = TRUE;
		score = search(SvCheckmated, SvMateIn0, iteration);
		if (output == PmConsole)
			printf("%3d  %9d  %5d ", iteration, nodes, score);
		else if (output == PmXBoard)
			printf("%d %d %d %d",
					iteration, score, (get_ms() - start_time) / 10, nodes);
		if (output != PmNone) {
			for (plyindex = 0; plyindex < pv_length[0]; ++plyindex)
				printf(" %s", move_str(pv[0][plyindex].b));
			printf("\n");
			fflush(stdout);
		}
		if (score > SvSlowMate || score < SvSlowLose)
			break;
	}
}


/* search() does just that, in negamax fashion */

int search(int alpha, int beta, int depth)
{
	int genindex, score, plyindex;
	BOOL checked, atleast1move;

	/* we're as deep as we want to be; call quiesce() to get
	   a reasonable score and return it. */
	if (!depth)
		return quiesce(alpha, beta);
	++nodes;

	/* do some housekeeping every 1024 nodes */
	if ((nodes & MX(10)) == 0)
		checkup();

	pv_length[ply] = ply;

	/* if this isn't the root of the search tree (where we have
	   to pick a move and can't simply return 0) then check to
	   see if the position is a repeat. if so, we can assume that
	   this line is a draw and return 0. */
	if (ply && reps())
		return SvEven;

	/* are we too deep? */
	if (ply >= MAX_PLY - 1)
		return eval();
	if (hply >= HIST_STACK - 1)
		return eval();

	/* are we in check? if so, we want to search deeper */
	checked = in_check(side);
	if (checked)
		++depth;
	gen();
	if (follow_pv)  /* are we following the PV? */
		sort_pv();
	atleast1move = FALSE;

	/* loop through the moves */
	for (genindex = first_move[ply]; genindex < first_move[ply + 1]; ++genindex) {
		sort(genindex);
		if (!makemove(gen_dat[genindex].m.b))
			continue;
		atleast1move = TRUE;
		score = -search(-beta, -alpha, depth - 1);
		takeback();
		if (score > alpha) {

			/* this move caused a cutoff, so increase the history
			   value so it gets ordered high next time we can
			   search it */
			history[(int)gen_dat[genindex].m.b.from][(int)gen_dat[genindex].m.b.to] += depth;
			if (score >= beta)
				return beta;
			alpha = score;

			/* update the PV */
			pv[ply][ply] = gen_dat[genindex].m;
			for (plyindex = ply + 1; plyindex < pv_length[ply + 1]; ++plyindex)
				pv[ply][plyindex] = pv[ply + 1][plyindex];
			pv_length[ply] = pv_length[ply + 1];
		}
	}

	/* no legal moves? then we're in checkmate or stalemate */
	if (!atleast1move) {
		if (checked)
			return SvCheckmated + ply;
		else
			return SvEven;
	}

	/* fifty move draw rule */
	if (fifty >= 100)
		return SvEven;
	return alpha;
}


/* quiesce() is a recursive minimax search function with
   alpha-beta cutoffs. In other words, negamax. It basically
   only searches capture sequences and allows the evaluation
   function to cut the search off (and set alpha). The idea
   is to find a position where there isn't a lot going on
   so the static evaluation function will work. */

int quiesce(int alpha, int beta)
{
	int genindex, score, plyindex;

	++nodes;

	/* do some housekeeping every 1024 nodes */
	if ((nodes & MX(10)) == 0)
		checkup();

	pv_length[ply] = ply;

	/* are we too deep? */
	if (ply >= MAX_PLY - 1)
		return eval();
	if (hply >= HIST_STACK - 1)
		return eval();

	/* check with the evaluation function */
	score = eval();
	if (score >= beta)
		return beta;
	if (score > alpha)
		alpha = score;

	gen_caps();
	if (follow_pv)  /* are we following the PV? */
		sort_pv();

	/* loop through the moves */
	for (genindex = first_move[ply]; genindex < first_move[ply + 1]; ++genindex) {
		sort(genindex);
		if (!makemove(gen_dat[genindex].m.b))
			continue;
		score = -quiesce(-beta, -alpha);
		takeback();
		if (score > alpha) {
			if (score >= beta)
				return beta;
			alpha = score;

			/* update the PV */
			pv[ply][ply] = gen_dat[genindex].m;
			for (plyindex = ply + 1; plyindex < pv_length[ply + 1]; ++plyindex)
				pv[ply][plyindex] = pv[ply + 1][plyindex];
			pv_length[ply] = pv_length[ply + 1];
		}
	}
	return alpha;
}


/* reps() returns the number of times the current position
   has been repeated. It compares the current value of hash
   to previous values. */

int reps()
{
	int histindex;
	int r = 0;

	for (histindex = hply - fifty; histindex < hply; ++histindex)
		if (hist_dat[histindex].hash == hash)
			++r;
	return r;
}


/* sort_pv() is called when the search function is following
   the PV (Principal Variation). It looks through the current
   ply's move list to see if the PV move is there. If so,
   it adds 10,000,000 to the move's score so it's played first
   by the search function. If not, follow_pv remains FALSE and
   search() stops calling sort_pv(). */

void sort_pv()
{
	int genindex;

	follow_pv = FALSE;
	for(genindex = first_move[ply]; genindex < first_move[ply + 1]; ++genindex)
		if (gen_dat[genindex].m.u == pv[0][ply].u) {
			follow_pv = TRUE;
			gen_dat[genindex].score += 10000000;
			return;
		}
}


/* sort() searches the current ply's move list from 'from'
   to the end to find the move with the highest score. Then it
   swaps that move and the 'from' move so the move with the
   highest score gets searched next, and hopefully produces
   a cutoff. */

void sort(int from)
{
	int genindex;
	int bs;  /* best score */
	int bi;  /* best i */
	gen_t g;

	bs = -1;
	bi = from;
	for (genindex = from; genindex < first_move[ply + 1]; ++genindex)
		if (gen_dat[genindex].score > bs) {
			bs = gen_dat[genindex].score;
			bi = genindex;
		}
	g = gen_dat[from];
	gen_dat[from] = gen_dat[bi];
	gen_dat[bi] = g;
}


/* checkup() is called once in a while during the search. */

void checkup()
{
	/* is the engine's time up? if so, longjmp back to the
	   beginning of think() */
	if (get_ms() >= stop_time) {
		stop_search = TRUE;
		longjmp(env, 0);
	}
}
