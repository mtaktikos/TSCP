#include <stdio.h>
#include "defs.h"
#include "data.h"
#include "protos.h"

int main() {
    init_hash();
    init_board();
    
    // Play the test sequence
    char* moves[] = {"e2e4", "i8g6", "b1c3", "c8d6", "d1h5", "g6e6", "d2d3", "d8f6"};
    int num_moves = 8;
    
    for (int i = 0; i < num_moves; i++) {
        gen();
        int m = parse_move(moves[i]);
        if (m == -1) {
            printf("Failed to parse move: %s\n", moves[i]);
            return 1;
        }
        if (!makemove(gen_dat[m].m.b)) {
            printf("Failed to make move: %s\n", moves[i]);
            return 1;
        }
        printf("Made move: %s\n", moves[i]);
    }
    
    // Display the board
    print_board();
    
    // Generate moves for white
    gen();
    
    printf("\nWhite Witch moves from c3:\n");
    for (int i = first_move[0]; i < first_move[1]; i++) {
        move_bytes m = gen_dat[i].m.b;
        if (m.from == 72 && piece[72] == AMAZON) {  // c3 is square 72
            printf("  %s\n", move_str(m));
        }
    }
    
    return 0;
}
