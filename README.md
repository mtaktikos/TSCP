# TSCP - Commoner Chess Variant

Tom Kerrigan's Simple Chess Program (TSCP) modified to implement a fairy chess variant with Commoners.

## About This Variant

This is a chess variant where Kings are replaced with "Commoners" - pieces that move like Kings but with special gating rules. See [COMMONER_VARIANT.md](COMMONER_VARIANT.md) for complete rules.

### Key Features

1. **Commoner Movement**: Moves one square in any direction (like a King)
2. **Optional Gating**: Choose whether to leave a Commoner behind when moving
   - `e1e2g` - Gate (leave Commoner on e1)
   - `e1e2` - Don't gate (e1 becomes empty)
3. **Win Condition**: Capturing ANY opponent's Commoner wins immediately

## Quick Start

```bash
make
./tscp
```

Example game:
```
tscp> e2e4        # Pawn move
tscp> e7e5        # Black pawn
tscp> e1e2g       # Commoner with gating (creates two commoners)
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
