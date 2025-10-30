# Test Case: Transparency Logic for Amazon/Witch Pieces

## Background
The Amazon (Witch, piece 'W') has a special property: it makes adjacent squares "transparent" for friendly pieces. This means sliding pieces can move through squares that are adjacent to a friendly Amazon, regardless of whether those squares contain friendly or enemy pieces.

## Test Scenario
This test reproduces the scenario from the issue where:
1. Black has a Witch on e4
2. White has a Pawn on d4 (adjacent to the Witch)
3. Black Queen on f4 should be able to capture White Queen on c4 by passing through:
   - e4 (has the Black Witch - transparent because it's the Amazon itself)
   - d4 (has White Pawn - transparent because it's adjacent to the Black Witch on e4)

## Move Sequence
```
1. d2-d4    i8-g6   (White Pawn d4, Black Witch g6)
2. e2-e4    h8-i6   (White Pawn e4, Black Knight i6)
3. d1-h5    g6-f6   (White Bishop h5, Black Witch f6)
4. e4-e5    f6-e6   (White Pawn e5, Black Witch e6)
5. g2-g4    g7-g6   (White Pawn g4, Black Pawn g6)
6. h5-i4    g8-c4   (White Bishop i4, Black Bishop c4)
7. c1-d3    f8-f4   (White Knight d3, Black Queen f4)
8. e1-c3    e6-e4   (White Queen c3, Black Witch e4)
9. c3xc4    f4xc4   (White Queen captures Black Bishop c4, Black Queen captures White Queen c4)
```

## Expected Result
Move 9 for Black (f4xc4) should be LEGAL because:
- The Black Witch on e4 makes e4 transparent (the Amazon itself)
- The Black Witch on e4 makes d4 transparent (adjacent square)
- The Black Queen can slide from f4 → e4 (transparent) → d4 (transparent) → c4 (capture)

## How to Test
Run the following commands in tscp:
```
d2d4
i8g6
e2e4
h8i6
d1h5
g6f6
e4e5
f6e6
g2g4
g7g6
h5i4
g8c4
c1d3
f8f4
e1c3
e6e4
c3c4
f4c4
d
```

The move `f4c4` should be accepted and the final board should show the Black Queen on c4.
