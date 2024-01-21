#include "../common/bitboard.h"
#include "../common/zobrist.h"
#include "usi.h"

int main(int argc, char **argv) {
    Bitboards::init();
    Zobrist::init();
    USI usi = USI();
    // std::cout << "Hello, World!" << std::endl;
    usi.loop();

    return 0;
}