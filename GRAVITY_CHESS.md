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
When a piece moves to an empty square (not a capture):
1. The piece falls down toward rank 1 due to gravity
2. The piece continues falling through empty squares
3. The piece stops when it reaches:
   - Another piece below it, or
   - Rank 1 (the bottom of the board)

When a piece captures:
- The capturing piece does NOT fall
- It remains at the capture square

### Cascade Effect
When a piece moves away from a square, all pieces above it in the same column fall down:
- Each piece falls down by one rank to fill the gap
- This creates a "cascade" or "compression" effect in the column
- The cascade continues until there are no more pieces above

Example:
```
Initial:         After h2f3:
h8  R            h8  .
h7  N            h7  R
h6  B            h6  N
h5  K            h5  B
h4  Q            h4  K
h3  B            h3  Q
h2  N ← moves    h2  B
h1  R            h1  R
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
