# Test Cases for Commoner Variant

This file contains test cases that demonstrate the three main features of the Commoner variant.

## Test 1: Commoner Movement (King-like, one square)

The commoner moves like a king - one square in any of 8 directions (orthogonal and diagonal).

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

The white commoner on e1 can move to: d1, d2, e2, f2, f1 (5 squares adjacent to e1).

## Test 2: Optional Gating

Players can choose whether to gate (leave a commoner behind) or not when moving a commoner.

### Test 2a: Gating Move (with 'g' suffix)

Input sequence:
```
e2e4
e7e5
e1e2g
```

Result after `e1e2g`:
```
8  r n b q c b n r
7  p p p p . p p p
6  . . . . . . . .
5  . . . . p . . .
4  . . . . P . . .
3  . . . . . . . .
2  P P P P C P P P
1  R N B Q C B N R
```

Note: White commoner is on BOTH e1 (gated) and e2 (moved to).

### Test 2b: Non-gating Move (without 'g' suffix)

Continuing from above, black plays:
```
e8e7
```

Result:
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

Note: Black commoner moved from e8 to e7, e8 is now EMPTY (no gating).

## Test 3: Win Condition (Capturing ANY commoner wins)

When ANY commoner is captured, the game ends immediately and the capturing side wins.

Input sequence (Fool's Mate variant):
```
f2f3
e7e5
g2g4
d8h4
e1f2
h4f2
```

Result:
```
0-1 {Black wins by capturing a Commoner}

8  r n b . c b n r
7  p p p p . p p p
6  . . . . . . . .
5  . . . . p . . .
4  . . . . . . P .
3  . . . . . P . .
2  P P P P P q . P
1  R N B Q . B N R
```

Note: The game ended immediately when black's queen captured white's commoner on f2. Black wins!

## Running Tests

To run these tests manually:

```bash
./tscp
# Then enter the moves as shown above
```

To verify the implementation:

```bash
# Test gating
echo -e "e2e4\ne7e5\ne1e2g\nd\nbye" | ./tscp

# Test non-gating
echo -e "e2e4\ne7e5\ne1e2g\ne8e7\nd\nbye" | ./tscp

# Test win condition
echo -e "f2f3\ne7e5\ng2g4\nd8h4\ne1f2\nh4f2\nd\nbye" | ./tscp
```
