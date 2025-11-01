/*
 *	DATA.H
 *	Tom Kerrigan's Simple Chess Program (TSCP)
 *
 *	Copyright 1997 Tom Kerrigan
 */


/* this is basically a copy of data.c that's included by most
   of the source files so they can use the data.c variables */

extern int color[80];
extern int piece[80];
extern int side;
extern int xside;
extern int castle;
extern int ep;
extern int fifty;
extern int hash;
extern int ply;
extern int hply;
extern gen_t gen_dat[GEN_STACK];
extern int first_move[MAX_PLY];
extern int history[80][80];
extern hist_t hist_dat[HIST_STACK];
extern int max_time;
extern int max_depth;
extern int start_time;
extern int stop_time;
extern int nodes;
extern move pv[MAX_PLY][MAX_PLY];
extern int pv_length[MAX_PLY];
extern BOOL follow_pv;
extern int hash_piece[2][8][80];
extern int hash_side;
extern int hash_ep[80];
extern int mailbox[156];
extern int mailbox64[80];
extern BOOL slide[8];
extern int offsets[8];
extern int offset[8][8];
extern int castle_mask[80];
extern char piece_char[8];
extern int init_color[80];
extern int init_piece[80];
extern BOOL whitetransparent[80];
extern BOOL blacktransparent[80];
