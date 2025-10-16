# Commoner Chess Variant

This is a modified version of TSCP (Tom Kerrigan's Simple Chess Program) that implements a fairy chess variant with the following rules:

## Changes from Standard Chess

### 1. Kings Replaced with Commoners
- The King piece has been replaced with a "Commoner"
- Commoners move like Queens (can move any number of squares in any direction)
- Commoners are displayed as 'C' (white) and 'c' (black) on the board

### 2. No Check or Checkmate
- There is no concept of "check" in this variant
- The in_check() function always returns FALSE
- Castling is not possible (removed since there are no kings)

### 3. Gating Mechanic
- When a Commoner moves, it doesn't leave an empty square behind
- Instead, it creates a new Commoner on the square it departed from
- This means Commoners can multiply as they move
- Each time a Commoner moves, there will be one more Commoner for that side

### 4. Win Condition
- The game is won by capturing ALL of the opponent's Commoners
- When a side has no Commoners remaining (count reaches 0), they lose
- With the gating mechanic, this means you must capture every Commoner that has been created through moves

### 5. Stalemate
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

After white plays e2-e4 and e1-e2 (commoner move):
```
8  r n b q c b n r
7  p p p p p p p p
6  . . . . . . . .
5  . . . . . . . .
4  . . . . P . . .
3  . . . . . . . .
2  P P P P C P P P
1  R N B Q C B N R
```

Note: There are now TWO white Commoners - one on e1 (gated) and one on e2 (moved to).

## Building

```bash
gcc -O3 -c board.c data.c eval.c search.c main.c book.c
gcc -O3 -o tscp board.o data.o eval.o search.o main.o book.o
```

## Running

```bash
./tscp
```

Use standard chess notation for moves (e.g., e2e4, e1e2, etc.)
