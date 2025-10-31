# Transparent Squares Implementation Test

This document demonstrates the transparent squares feature for AMAZON (W) pieces.

## Feature Description

- Squares adjacent to a White AMAZON and the AMAZON itself are "whitetransparent"
- Squares adjacent to a Black AMAZON and the AMAZON itself are "blacktransparent"
- White sliders (BISHOP, ROOK, QUEEN) can move through whitetransparent squares
- Black sliders (BISHOP, ROOK, QUEEN) can move through blacktransparent squares
- Sliders can land only on empty squares or enemy pieces (capturing them)

## Initial Position

```
8  r g n b k q b n w r
7  p p p p p p p p p p
6  . . . . . . . . . .
5  . . . . . . . . . .
4  . . . . . . . . . .
3  . . . . . . . . . .
2  P P P P P P P P P P
1  R W N B Q K B N G R
```

White AMAZON at b1, Black AMAZON at i8.

## Transparent Squares

### White Transparent Squares (around b1):
- a1, b1, c1 (rank 1)
- a2, b2, c2 (rank 2)

### Black Transparent Squares (around i8):
- h8, i8, j8 (rank 8)
- h7, i7, j7 (rank 7)

## Example: ROOK Movement Through Transparent Squares

If we set up a position with:
- White ROOK at e2
- White AMAZON at e4
- Black PAWN at e7

The White ROOK can move to:
- e3 (empty square)
- e5 (empty square, passed through AMAZON at e4)
- e6 (empty square)
- e7 (capture Black PAWN)

The ROOK passes through the AMAZON at e4 because e4 is a whitetransparent square.

## Implementation Details

The implementation adds:
1. `whitetransparent[80]` and `blacktransparent[80]` arrays
2. `compute_transparent_squares()` function called after board changes
3. Modified `genPiece()` to allow sliders to pass through transparent squares
4. Modified `gen_caps()` with the same logic

## Testing

Build and run:
```bash
make clean
make
./tscp
```

The program works correctly with the transparent squares feature enabled.
