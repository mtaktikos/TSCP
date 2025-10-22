# Gravity Chess Variant

This is a custom chess variant implemented in TSCP with the following rules:

## Starting Position
The board is rotated 90 degrees from standard chess:
```
FEN: rp4PR/np4PN/bp4PB/kp4PK/qp4PQ/bp4PB/np4PN/rp4PR w - - 0 1

8  r p . . . . P R
7  n p . . . . P N
6  b p . . . . P B
5  k p . . . . P K
4  q p . . . . P Q
3  b p . . . . P B
2  n p . . . . P N
1  r p . . . . P R

   a b c d e f g h
```

- Black pieces are on columns a and b
- White pieces are on columns g and h
- Columns c-f are initially empty

## Pawn Movement
- **White pawns** move **west** (left, toward column a)
- **Black pawns** move **east** (right, toward column h)
- Pawns can move 1 or 2 squares from their starting position
- Pawns capture diagonally (one square in their movement direction and one rank up or down)

## Promotion
- **White pawns** promote when reaching **column a** (a1-a8)
- **Black pawns** promote when reaching **column h** (h1-h8)

## Gravity Mechanics

### Moving Piece Gravity
When a piece moves to an empty square (not a capture):
1. The piece falls down toward rank 1 due to gravity
2. The piece continues falling through empty squares
3. The piece stops when it reaches:
   - Another piece below it, or
   - Rank 1 (the bottom of the board)

When a piece captures:
- The capturing piece does NOT fall
- It remains at the capture square

### Column Gravity
After a piece moves from a square (column n, rank m):
1. If there is a piece at column n, rank m+1 (the square directly above), it falls down to rank m
2. If there is a piece at column n, rank m+2, it falls down to rank m+1
3. This chain reaction continues upward through all pieces in the column up to rank 8

Example:
```
Before white knight moves from g7:
8  . . . . . . P R
7  . . . . . . P N  <- knight moves from here
6  . . . . . . P B

After g7e8 (knight to e8, then falls):
8  . . . . . . P .  <- rook fell from here
7  . . . . . . P R  <- to here
6  . . . . . . P B
...
1  . . . . N . . .  <- knight landed here after gravity
```

## Special Rules
- **No castling** - castling is disabled
- **No en passant** - en passant captures are disabled

## Example Game
```
Initial position:
8  r p . . . . P R
7  n p . . . . P N
...

After g8f8 (white pawn moves west):
8  r p . . . . . R  (pawn left g8)
...
1  r p . . . P P R  (pawn fell to f1)

After b8c8 (black pawn moves east):
8  r . . . . . . R  (pawn left b8)
...
1  r p p . . P P R  (pawn fell to c1)
```

## Implementation Details
- Based on Tom Kerrigan's Simple Chess Program (TSCP)
- Gravity is applied automatically after each move
- The move history tracks gravity adjustments for proper takeback
- All standard chess pieces move normally (knights, bishops, rooks, queens, kings)
