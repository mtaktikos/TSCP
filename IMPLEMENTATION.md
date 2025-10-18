# Implementation Summary: Battlekings Chess Variant

This document summarizes the changes made to implement the Battlekings chess variant, which evolved from the "lemmings" branch.

## Requirements Implemented

### 1. Commoner Movement: King-like (not Queen-like)

**Requirement**: The Commoner doesn't move like a Queen, but like a King.

**Implementation**: 
- The Commoner piece (index 5) already had `slide[5] = FALSE` in data.c
- This means it moves only one square at a time
- It has 8 movement directions defined in `offset[5][8]`: diagonal and orthogonal
- No code changes were needed for this requirement - it was already implemented correctly

**Files Changed**: None (already correct)

**Testing**: Verified that commoner moves only one square in any of 8 directions

---

### 2. Win Condition: Capturing a Single Commoner Wins

**Requirement**: The win condition is not to capture ALL opponent's Commoners, a single captured Commoner is enough to win.

**Implementation**:
- Modified `eval()` in eval.c to check the move history (`hist_dat[]`) for any commoner captures
- When a commoner capture is detected, return ±10000 (win/loss score) immediately
- Modified `print_result()` in main.c to check the history and print the appropriate win message
- The game now ends as soon as ANY commoner is captured

**Files Changed**:
- eval.c: Lines 113-135 (replaced commoner counting logic with history checking)
- main.c: Lines 409-430 (replaced commoner counting logic with history checking)

**Testing**: 
- Tested with various capture scenarios
- Game correctly ended with message: "0-1 {Black wins by capturing a Commoner}"

---

### 3. Automatic Gating for Pieces (Except Commoners)

**Requirement**: In the Battlekings variant:
- Commoners do NOT gate (they make normal moves like Kings)
- Other pieces gate automatically:
  - Pawns leave Knights behind
  - Knights leave Bishops behind
  - Bishops leave Rooks behind
  - Rooks leave Queens behind
  - Queens leave Commoners behind
- No en passant
- No castling

**Implementation**:
- Modified `genPiece()` in board.c to generate moves with bit 64 set for Knights, Bishops, Rooks, and Queens (gating enabled)
- Modified `genPawn()` to generate pawn moves with bit 64 set (gating enabled)
- Modified `gen_caps()` to apply same gating rules for captures
- Updated `makemove()` to handle gating based on piece type:
  - When bit 64 is set, leave behind the appropriate piece type based on what moved
  - Pawn → Knight
  - Knight → Bishop
  - Bishop → Rook
  - Rook → Queen
  - Queen → Commoner
  - Commoner → no gating (normal move)
- Updated `genEnPassant()` to do nothing (no en passant in battlekings)
- Removed en passant square setting in `makemove()`
- Castling was already disabled (no kings in the variant)

**Files Changed**:
- defs.h: Lines 51-64 (bit 64 documentation updated for battlekings)
- board.c: 
  - Lines 175-199 (genPawn - automatic gating for pawns)
  - Lines 201-230 (genPiece - automatic gating for Knights, Bishops, Rooks, Queens)
  - Lines 282-332 (gen_caps - automatic gating for captures)
  - Lines 158-160 (genEnPassant - disabled)
  - Lines 458-462 (makemove - removed en passant square setting)
  - Lines 474-513 (makemove - handle automatic gating based on piece type)
- main.c:
  - Lines 163-195 (parse_move - simplified, no 'g' suffix needed)
  - Lines 233-255 (move_str - simplified, no 'g' suffix shown)

**Move Notation**:
- All moves use standard notation: `e2e4`, `b1c3`, etc.
- Gating is automatic and doesn't require special notation
- Commoners move normally without gating

**Testing**:
- Tested pawn gating: `e2e4` correctly left Knight on e2
- Tested knight gating: `b1c3` correctly left Bishop on b1
- Tested gated piece movement: Knight on e2 moving to d4 left Bishop on e2
- Commoners move normally without gating

---

### 4. No Castling

**Requirement**: Castling is not allowed in Battlekings.

**Implementation**: Already disabled in the lemmings variant (no kings).

**Files Changed**: None (already correct)

---

### 5. No En Passant

**Requirement**: En passant captures are not allowed in Battlekings.

**Implementation**:
- `genEnPassant()` now does nothing
- En passant square is always set to -1 in `makemove()`
- En passant generation in `gen_caps()` is removed

**Files Changed**:
- board.c: Modified genEnPassant, makemove, and gen_caps

---

## Files Modified

1. **defs.h** - Bit 64 documentation updated for automatic gating
2. **board.c** - Updated move generation, makemove for automatic gating based on piece type
3. **eval.c** - Win condition: any commoner captured (already implemented)
4. **main.c** - Simplified move parsing/display (no 'g' suffix needed)
5. **COMMONER_VARIANT.md** - Updated documentation for Battlekings rules
6. **README.md** - Updated for Battlekings variant
7. **IMPLEMENTATION.md** - Updated implementation details

## Testing Summary

All requirements have been tested and verified:

✅ Commoner moves like a King (one square, 8 directions)
✅ Game ends immediately when ANY commoner is captured
✅ Automatic gating: Pawns → Knights, Knights → Bishops, Bishops → Rooks, Rooks → Queens, Queens → Commoners
✅ Commoners don't gate (normal moves)
✅ No castling
✅ No en passant

## Build Instructions

```bash
make clean
make
```

## Usage Examples

See COMMONER_VARIANT.md for detailed examples of:
- Automatic gating for different pieces
- Commoner movement (no gating)
- Win condition (capturing a commoner)
