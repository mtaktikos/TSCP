# TSCP - Battlekings Chess Variant

Tom Kerrigan's Simple Chess Program (TSCP) modified to implement the "Battlekings" fairy chess variant.

## About This Variant

This is a chess variant where Kings are replaced with "Commoners" and pieces automatically gate (leave behind other pieces) when they move. See [COMMONER_VARIANT.md](COMMONER_VARIANT.md) for complete rules.

### Key Features

1. **Commoner Movement**: Moves one square in any direction (like a King)
2. **Automatic Gating**: Pieces automatically leave behind other pieces when moving:
   - Pawns leave Knights
   - Knights leave Bishops
   - Bishops leave Rooks
   - Rooks leave Queens
   - Queens leave Commoners
   - Commoners don't gate (normal moves)
3. **Win Condition**: Capturing ANY opponent's Commoner wins immediately
4. **No Castling**: Castling is not allowed
5. **No En Passant**: En passant captures are not allowed

## Quick Start

```bash
make
./tscp
```

Example game:
```
tscp> e2e4        # Pawn move - automatically leaves Knight on e2
tscp> e7e5        # Black pawn - automatically leaves Knight on e7
tscp> b1c3        # Knight move - automatically leaves Bishop on b1
tscp> d           # Display board
```

## Documentation

- [COMMONER_VARIANT.md](COMMONER_VARIANT.md) - Complete rules and examples
- [TEST_CASES.md](TEST_CASES.md) - Test cases demonstrating all features
- [IMPLEMENTATION.md](IMPLEMENTATION.md) - Technical implementation details

## Build Requirements

- GCC or compatible C compiler
- Make

## Original TSCP

This is based on Tom Kerrigan's Simple Chess Program (TSCP) version 1.81b.
Visual Studio 2015 C compatible.
