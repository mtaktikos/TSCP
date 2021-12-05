/*
 *	DATA.H
 *	Tom Kerrigan's Simple Chess Program (TSCP)
 *
 *	Copyright 1997 Tom Kerrigan
 */


/* this is basically a copy of data.c that's included by most
   of the source files so they can use the data.c variables */

extern int color[SqLen];
extern int piece[SqLen];
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
extern int history[SqLen][SqLen];
extern hist_t hist_dat[HIST_STACK];
extern int max_time;
extern int max_depth;
extern int start_time;
extern int stop_time;
extern int nodes;
extern move pv[MAX_PLY][MAX_PLY];
extern int pv_length[MAX_PLY];
extern BOOL follow_pv;
extern int hash_piece[ColorLen][PieceLen][SqLen];
extern int hash_side;
extern int hash_ep[SqLen];
extern int mailbox[MailboxLen];
extern int mailbox64[SqLen];
extern BOOL slide[PieceLen];
extern int offsets[PieceLen];
extern int offset[PieceLen][OffsetLen];
extern int castle_mask[SqLen];
extern char piece_char[PieceLen];
extern int init_color[SqLen];
extern int init_piece[SqLen];
