#include "zobrist.h"
#include <random>

HASH_KEY Zobrist::zero;
HASH_KEY Zobrist::psq[COLOR_PIECE_NB][SQ_NB];
HASH_KEY Zobrist::hand[COLOR_NB][RAW_PIECE_NB];
HASH_KEY Zobrist::occ[COLOR_NB][SQ_NB];

void Zobrist::init() {
    std::mt19937 mt;
    mt.seed(20231008);

    // 盤面の乱数表の初期化
    // 注意！！！psq[PIECE_ZERO][sq] は全てゼロにしておくこと！！！
    for (int sq = SQ_ZERO; sq < SQ_NB; ++sq) {
        for (int pc = COLOR_PIECE_BEGIN; pc < COLOR_PIECE_NB; ++pc) {
            // 盤上sqにpcの駒があるときのZobrist Keyを乱数で初期化
            // 0ビット目は手番の情報が入るのでゼロで固定しておく（ビット演算しても変わらない）
            uint64_t r = (((uint64_t)mt() << 32) | (uint64_t)mt()) & ~1ULL;
            Zobrist::psq[pc][sq] = r;
            // 0ビット目が1になっていないかチェック
            // ASSERT(!(Zobrist::psq[pc][sq] & 0b1), "0 bit is not 0 !!!");
        }
    }

    // 手駒の乱数表の初期化
    for (int c = BLACK; c < COLOR_NB; ++c) {
        for (int pr = RAW_PIECE_BEGIN; pr < RAW_PIECE_NB; ++pr) {
            // c側の手駒prが一枚増えるごとにこれを加算するZobristKey
            uint64_t r = (((uint64_t)mt() << 32) | (uint64_t)mt()) & ~1ULL;
            Zobrist::hand[c][pr] = r;
            // ASSERT(!(Zobrist::hand[c][pr] & 0b1), "0 bit is not 0 !!!");
        }
    }
}