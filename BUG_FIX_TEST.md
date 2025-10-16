# Bug Fix: Gating Logic Issue

## Problem Statement
After a Commoner moves with gating, and then a non-Commoner moves, the Commoners suddenly disappear from the board.

## Root Cause
The `takeback()` function in `board.c` had incorrect logic for handling gated moves. When taking back a gated move, it was clearing the `from` square instead of keeping the original piece there.

### Before Fix (Lines 553-558)
```c
/* For gating: if this was a Commoner move with gating, clear the from square.
   Otherwise the from square was already set with the piece above. */
if (piece[(int)m.to] == COMMONER && (m.bits & 64) && !(m.bits & 2)) {
    color[(int)m.from] = EMPTY;
    piece[(int)m.from] = EMPTY;
}
```

This code incorrectly cleared the `from` square for gated moves, which was backwards logic.

### After Fix
The problematic lines were removed entirely, as the earlier code (lines 547-551) already correctly restores the piece at the `from` square.

## Test Cases

### Test 1: Undo After Gating Move
```bash
echo -e "e2e4\ne7e5\ne1e2g\nd\nundo\nd\nbye" | ./tscp
```

**Before Fix:** The commoner at e1 disappeared after undo.
**After Fix:** The commoner at e1 is correctly restored after undo.

### Test 2: Non-Commoner Move After Gating
```bash
echo -e "e2e4\ne7e5\ne1e2g\ne8e7g\nd\nb1c3\nd\nb8c6\nd\nbye" | ./tscp
```

**Before Fix:** Commoners disappeared from the board after knight moves.
**After Fix:** All commoners (white at e1 & e2, black at e8 & e7) remain on the board.

### Test 3: Computer Search With Gating
```bash
echo -e "e2e4\ne7e5\ne1e2g\non\nd\nbye" | ./tscp
```

**Before Fix:** The computer couldn't properly evaluate moves because `takeback()` corrupted the board state during search.
**After Fix:** The computer successfully finds and plays moves (e.g., e8e7g) and all commoners remain on the board.

## Files Changed
- `board.c`: Removed incorrect gating logic from `takeback()` function (lines 553-558)

## Verification
All test cases from TEST_CASES.md still pass:
- ✅ Commoner movement (one square, 8 directions)
- ✅ Gating moves (with 'g' suffix)
- ✅ Non-gating moves (without 'g' suffix)
- ✅ Undo/takeback of gating moves
- ✅ Win condition (capturing a commoner)
- ✅ Computer search with gating moves
