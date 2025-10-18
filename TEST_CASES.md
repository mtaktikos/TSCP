# Test Cases for Battlekings Variant

This file contains test cases that demonstrate the main features of the Battlekings variant.

## Test 1: Commoner Movement (King-like, one square, NO gating)

The commoner moves like a king - one square in any of 8 directions (orthogonal and diagonal).
Commoners do NOT gate when they move (they leave the departure square empty).

Initial position:
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

The white commoner on e1 can move to adjacent squares when they are empty or contain enemy pieces. In the initial position, the Commoner can move to d2, e2, or f2 (3 available squares), since d1, e1, and f1 are occupied by friendly pieces.

## Test 2: Automatic Gating for Pieces

In Battlekings, pieces automatically gate (leave behind another piece) when they move:
- Pawns leave Knights
- Knights leave Bishops
- Bishops leave Rooks
- Rooks leave Queens
- Queens leave Commoners
- Commoners do NOT gate

### Test 2a: Pawn Gates Knight

Input sequence:
```
e2e4
```

Result after `e2e4`:
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

Note: Pawn moved from e2 to e4, and automatically left a Knight on e2.

### Test 2b: Knight Gates Bishop

Continuing from above:
```
b1c3
```

Result:
```
8  r n b q c b n r
7  p p p p p p p p
6  . . . . . . . .
5  . . . . . . . .
4  . . . . P . . .
3  . . N . . . . .
2  P P P P N P P P
1  R B B Q C B N R
```

Note: Knight moved from b1 to c3, and automatically left a Bishop on b1.

### Test 2c: Gated Knight Also Gates

The gated Knight on e2 can move and will gate a Bishop:
```
e2d4
```

Result:
```
8  r n b q c b n r
7  p p p p p p p p
6  . . . . . . . .
5  . . . . . . . .
4  . . . N P . . .
3  . . N . . . . .
2  P P P P B P P P
1  R B B Q C B N R
```

Note: The gated Knight moved from e2 to d4 and left a Bishop on e2.

## Test 3: Win Condition (Capturing ANY commoner wins)

When ANY commoner is captured, the game ends immediately and the capturing side wins.

The exact scenario will depend on the game progression, but the principle is:
- As soon as any Commoner is captured, the game ends
- The side that captured the Commoner wins

## Test 4: Commoner Does NOT Gate

When a Commoner moves, it does NOT leave behind another piece.

Input sequence:
```
g1f3
e7e6
e1g1
```

After these moves, the Commoner will have moved from e1 to g1, and e1 will be empty (no gating).

## Running Tests

To run these tests manually:

```bash
./tscp
# Then enter the moves as shown above
```

To verify the implementation:

```bash
# Test pawn gating knight
echo -e "e2e4\nd\nbye" | ./tscp

# Test knight gating bishop
echo -e "b1c3\nd\nbye" | ./tscp

# Test gated piece also gates (after letting computer make a move for black)
echo -e "e2e4\non\noff\ne2d4\nd\nbye" | ./tscp

# Let computer play to see full variant in action
echo -e "on\non\non\non\nd\nbye" | ./tscp
```
