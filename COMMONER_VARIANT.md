# Commoner Chess Variant

This is a modified version of TSCP (Tom Kerrigan's Simple Chess Program) that implements a fairy chess variant with the following rules:

## Changes from Standard Chess

### 1. Kings Replaced with Commoners
- The King piece has been replaced with a "Commoner"
- Commoners move like Kings (one square in any direction - orthogonal or diagonal)
- Commoners are displayed as 'C' (white) and 'c' (black) on the board

### 2. No Check or Checkmate
- There is no concept of "check" in this variant
- The in_check() function always returns FALSE
- Castling is not possible (removed since there are no kings)

### 3. Optional Gating Mechanic
- When a Commoner moves, the player can CHOOSE whether to gate or not
- **Gating move** (append 'g' to move notation): Creates a new Commoner on the departure square
  - Example: `e1e2g` - Commoner moves from e1 to e2, leaving a Commoner on e1
- **Non-gating move** (normal move notation): Leaves the departure square empty
  - Example: `e1e2` - Commoner moves from e1 to e2, e1 becomes empty
- This allows strategic decisions about whether to multiply Commoners or keep mobility

### 4. Win Condition
- The game is won by capturing **ANY** of the opponent's Commoners (not all of them)
- As soon as a Commoner is captured, the game ends immediately
- The capturing side wins

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

After white plays e2-e4 and e1-e2g (commoner move with gating):
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

After black plays e7-e5 and e8-e7 (commoner move WITHOUT gating):
```
8  r n b q . b n r
7  p p p p c p p p
6  . . . . . . . .
5  . . . . p . . .
4  . . . . P . . .
3  . . . . . . . .
2  P P P P C P P P
1  R N B Q C B N R
```

Note: Black still has only ONE commoner - it moved from e8 to e7, leaving e8 empty.

## Move Notation

- Standard moves: `e2e4` (piece moves from e2 to e4)
- Gating Commoner moves: `e1e2g` (Commoner moves from e1 to e2, gates on e1)
- Non-gating Commoner moves: `e1e2` (Commoner moves from e1 to e2, no gating)
- Captures work the same way - append 'g' to gate or omit it to not gate

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
