/*
 *	DEFS.H
 *	Tom Kerrigan's Simple Chess Program (TSCP)
 *
 *	Copyright 1997 Tom Kerrigan
 */

/* Booleans */

typedef int BOOL;

#define TRUE  1
#define FALSE 0

/* Binary exponentiation macros */

#define BX(n) (1 << (n))
#define MX(n) (BX(n) - 1)

/* Limits */

#define GEN_STACK  1120
#define MAX_PLY      32
#define HIST_STACK  400

/* Time and depth limits */

#define MaxLevelDepth 32
#define MaxLevelTime  BX(25)

/* Text line character limit */

#define TextLen 256

/* Chessboard files */

#define FileA 0
#define FileB 1
#define FileC 2
#define FileD 3
#define FileE 4
#define FileF 5
#define FileG 6
#define FileH 7

#define FileLen 8

/* Chessboard ranks */

#define Rank1 7
#define Rank2 6
#define Rank3 5
#define Rank4 4
#define Rank5 3
#define Rank6 2
#define Rank7 1
#define Rank8 0

#define RankLen 8

/* Square generation */

#define MapToSq(file, rank) ((file) + (FileLen * (rank)))

/* Squares */

#define SqNil (-1)

#define SqA1 MapToSq(FileA, Rank1)
#define SqB1 MapToSq(FileB, Rank1)
#define SqC1 MapToSq(FileC, Rank1)
#define SqD1 MapToSq(FileD, Rank1)
#define SqE1 MapToSq(FileE, Rank1)
#define SqF1 MapToSq(FileF, Rank1)
#define SqG1 MapToSq(FileG, Rank1)
#define SqH1 MapToSq(FileH, Rank1)

#define SqA8 MapToSq(FileA, Rank8)
#define SqB8 MapToSq(FileB, Rank8)
#define SqC8 MapToSq(FileC, Rank8)
#define SqD8 MapToSq(FileD, Rank8)
#define SqE8 MapToSq(FileE, Rank8)
#define SqF8 MapToSq(FileF, Rank8)
#define SqG8 MapToSq(FileG, Rank8)
#define SqH8 MapToSq(FileH, Rank8)

#define SqLen (FileLen * RankLen)

/* File and rank extraction */

#define MapToFile(sq) ((sq) & 7)
#define MapToRank(sq) ((sq) >> 3)

/* Colors */

#define LIGHT   0
#define DARK    1
#define NoColor 2

#define ColorLen 2

/* Pieces */

#define PAWN    0
#define KNIGHT  1
#define BISHOP  2
#define ROOK    3
#define QUEEN   4
#define KING    5
#define NoPiece 6

#define PieceLen 6

/* Mailbox */

#define MailboxLen 120
#define OffsetLen    8

/* Directional deltas */

#define DeltaE 1           /*  1 */
#define DeltaN (-FileLen)  /* -8 */
#define DeltaW (-1)        /* -1 */
#define DeltaS FileLen     /*  8 */

#define DeltaNE (DeltaN + DeltaE)  /* -7 */
#define DeltaNW (DeltaN + DeltaW)  /* -9 */
#define DeltaSW (DeltaS + DeltaW)  /*  7 */
#define DeltaSE (DeltaS + DeltaE)  /*  9 */

#define Delta2N (2 * DeltaN)  /* -16 */
#define Delta2S (2 * DeltaS)  /*  16 */

/* Useful score values */

#define SvEven       0
#define SvCheckmated (-10000)
#define SvMateIn0    (-SvCheckmated)
#define SvSlowLose   (-9000)
#define SvSlowMate   (-SvSlowLose)

/* This is the basic description of a move. promote is what
   piece to promote the pawn to, if the move is a pawn
   promotion. bits is a bitfield that describes the move,
   with the following bits:

   1	capture
   2	castle
   4	en passant capture
   8	pushing a pawn 2 squares
   16	pawn move
   32	promote

   It's union'ed with an integer so two moves can easily
   be compared with each other. */

/* Move flag bits */

#define MBcapture   0
#define MBcastle    1
#define MBenpassant 2
#define MBdoubleadv 3
#define MBpawnmove  4
#define MBpromote   5

/* Move flag bit masks */

#define MMcapture   BX(MBcapture)
#define MMcastle    BX(MBcastle)
#define MMenpassant BX(MBenpassant)
#define MMdoubleadv BX(MBdoubleadv)
#define MMpawnmove  BX(MBpawnmove)
#define MMpromote   BX(MBpromote)

/* Move flag bit mask combinations */

#define MCpawncapt (MMpawnmove | MMcapture)
#define MCpawndadv (MMpawnmove | MMdoubleadv)
#define MCpawnep   (MMpawnmove | MMenpassant | MMcapture)

/* Castling flag bits */

#define CBwk 0
#define CBwq 1
#define CBbk 2
#define CBbq 3

/* Castling flag bit masks */

#define CMwk BX(CBwk)
#define CMwq BX(CBwq)
#define CMbk BX(CBbk)
#define CMbq BX(CBbq)

/* Castling flag bit mask combinations */

#define CCemp 0
#define CCall (CMwk | CMwq | CMbk | CMbq)
#define CCiA1 (CMwk | CMbk | CMbq)
#define CCiE1 (CMbk | CMbq)
#define CCiH1 (CMwq | CMbk | CMbq)
#define CCiA8 (CMwk | CMwq | CMbk)
#define CCiE8 (CMwk | CMwq)
#define CCiH8 (CMwk | CMwq | CMbq)

typedef struct {
	char from;
	char to;
	char promote;
	char bits;
} move_bytes;

typedef union {
	move_bytes b;
	int u;
} move;

/* Special move values */

#define MvNil  (-1)
#define MvVoid 0

/* an element of the move stack. it's just a move with a
   score, so it can be sorted by the search functions. */
typedef struct {
	move m;
	int score;
} gen_t;

/* an element of the history stack, with the information
   necessary to take a move back. */
typedef struct {
	move m;
	int capture;
	int castle;
	int ep;
	int fifty;
	int hash;
} hist_t;

/* Posting modes */

#define PmNone    0
#define PmConsole 1
#define PmXBoard  2
