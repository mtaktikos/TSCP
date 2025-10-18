# Battlekings Chess Variant

This is a modified version of TSCP (Tom Kerrigan's Simple Chess Program) that implements a fairy chess variant called "Battlekings" with the following rules:

## Changes from Standard Chess

### 1. Kings Replaced with Commoners
- The King piece has been replaced with a "Commoner"
- Commoners move like Kings (one square in any direction - orthogonal or diagonal)
- Commoners are displayed as 'C' (white) and 'c' (black) on the board

### 2. No Check or Checkmate
- There is no concept of "check" in this variant
- The in_check() function always returns FALSE
- Castling is not possible (removed since there are no kings)

### 3. Automatic Gating Mechanic
- When pieces move (except Commoners), they automatically leave behind a specific piece:
  - **Pawns** leave **Knights** behind
  - **Knights** leave **Bishops** behind
  - **Bishops** leave **Rooks** behind
  - **Rooks** leave **Queens** behind
  - **Queens** leave **Commoners** behind
  - **Commoners** do NOT gate (they make normal moves like Kings)
- Gating is automatic and mandatory for all pieces except Commoners
- Example: `e2e4` - Pawn moves from e2 to e4, automatically leaving a Knight on e2

### 4. No En Passant
- En passant captures are not allowed in this variant

### 5. Win Condition
- The game is won by capturing **ANY** of the opponent's Commoners (not all of them)
- As soon as a Commoner is captured, the game ends immediately
- The capturing side wins

### 6. Stalemate
- If a player has no legal moves but still has Commoners, it's a stalemate (draw)
- Draw by repetition and fifty-move rule still apply

## Example Game

Starting position:
```
8  r n b q c b n r
7  p p p p p p p p
6  . . . . . . . .
5  . . . . . . . .
4  . . . . . . . .
3  . . . . . . . .
2  P P P P P P P P
1  R N B Q C B N R
```

After white plays e2-e4 (pawn move with automatic gating):
```
8  r n b q c b n r
7  p p p p p p p p
6  . . . . . . . .
5  . . . . . . . .
4  . . . . P . . .
3  . . . . . . . .
2  P P P P N P P P
1  R N B Q C B N R
```

Note: The pawn moved to e4 and automatically left a Knight on e2.

After black plays c7-c5 (pawn move with automatic gating):
```
8  r n b q c b n r
7  p p n p p p p p
6  . . . . . . . .
5  . . p . . . . .
4  . . . . P . . .
3  . . . . . . . .
2  P P P P N P P P
1  R N B Q C B N R
```

Note: Black's pawn moved to c5 and automatically left a Knight on c7.

After white plays b1-c3 (knight move with automatic gating):
```
8  r n b q c b n r
7  p p n p p p p p
6  . . . . . . . .
5  . . p . . . . .
4  . . . . P . . .
3  . . N . . . . .
2  P P P P N P P P
1  R B B Q C B N R
```

Note: The Knight moved to c3 and automatically left a Bishop on b1.

## Move Notation

- Standard moves: `e2e4` (piece moves from e2 to e4, automatically gating if applicable)
- No special notation needed - gating is automatic for Pawns, Knights, Bishops, Rooks, and Queens
- Commoner moves: `e1e2` (Commoner moves from e1 to e2, no gating)
- Captures work the same way

## Building

```bash
make
```

Or manually:
```bash
gcc -O3 -c board.c data.c eval.c search.c main.c book.c
gcc -O3 -o tscp board.o data.o eval.o search.o main.o book.o
```

## Running

```bash
./tscp
```

Use standard chess notation for moves (e.g., e2e4, e1e2g, etc.)
