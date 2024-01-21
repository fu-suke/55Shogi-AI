#include "bitboard.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <random>
#include <string>

Bitboard CROSS_EFFECT_BB[SQ_NB]; // 十字方向の利き
Bitboard X_EFFECT_BB[SQ_NB];     // X字方向の利き
Bitboard PAWN_EFFECT_BB[SQ_NB][COLOR_NB];
Bitboard GOLD_EFFECT_BB[SQ_NB][COLOR_NB];
Bitboard SILVER_EFFECT_BB[SQ_NB][COLOR_NB];
Bitboard KING_EFFECT_BB[SQ_NB];
Bitboard BISHOP_EFFECT_BB[SQ_NB][DIRECTION_NB];
Bitboard ROOK_EFFECT_BB[SQ_NB][DIRECTION_NB];

Bitboard BETWEEN_BB[BETWEEN_INDEX_SIZE];
uint16_t BETWEEN_INDEX[SQ_NB][SQ_NB];

Bitboard FILE_BB[FILE_NB]; // 各筋の全ビットが1のBitboard
Bitboard PROMOTE_ZONE[COLOR_NB]; // [BLACK]なら、先手が成れる場所のBitboard

HASH_KEY Bitboards::occ[SQ_NB];
HASH_KEY Bitboards::square[SQ_NB];
std::unordered_map<HASH_KEY, Bitboard> Bitboards::hash_rook_effect;
std::unordered_map<HASH_KEY, Bitboard> Bitboards::hash_bishop_effect;

unsigned ctz(unsigned x) {
    if (x == 0)
        return 32;
    unsigned n = 0;
    if ((x & 0x0000FFFF) == 0) {
        n += 16;
        x >>= 16;
    }
    if ((x & 0x000000FF) == 0) {
        n += 8;
        x >>= 8;
    }
    if ((x & 0x0000000F) == 0) {
        n += 4;
        x >>= 4;
    }
    if ((x & 0x00000003) == 0) {
        n += 2;
        x >>= 2;
    }
    return n + (x & 1 ? 0 : 1);
}

std::ostream &operator<<(std::ostream &os, const Bitboard &bb) {
    // bbの25ビット目以上に1が立っている場合は、エラーを出力する
    // ASSERT(!(bb.p & 0xFE000000), "bb.p & 0xFE000000");
    std::cout << "----------" << std::endl;
    for (int rank = 0; rank < 5; ++rank) {
        for (int file = 0; file < 5; ++file) {
            int index = rank + (20 - file * 5);
            bool bit_set = bb.p & (1U << index);
            os << (bit_set ? '1' : '0') << ' ';
            // os << index << ' ';
        }
        os << '\n'; // 各段ごとに改行
    }

    std::cout << "----------" << std::endl;
    return os;
}

// Bitboard関連のテーブルの初期化
void Bitboards::init() {
    // --------------------
    //   利きテーブルの初期化
    // --------------------

    // 歩の利きテーブル
    for (int sq = SQ_ZERO; sq < SQ_NB; ++sq) {
        int file = sq / 5; // 0 ~ 4
        int rank = sq % 5; // 0 ~ 4

        // 初期化
        PAWN_EFFECT_BB[sq][BLACK] = Bitboard(0);
        PAWN_EFFECT_BB[sq][WHITE] = Bitboard(0);

        // 黒の歩の利き
        if (rank > 0) {                 // 上端の段を除く
            int target_sq = sq + SQ_UP; // 今いるマスから上に1つ進んだマス
            PAWN_EFFECT_BB[sq][BLACK] = Bitboard(1 << target_sq);
        }

        // 白の歩の利き
        if (rank < 4) { // 下端の段を除く
            int target_sq = sq + SQ_DOWN;
            PAWN_EFFECT_BB[sq][WHITE] = Bitboard(1 << target_sq);
        }
    }

    // 十字方向の利きテーブル
    // 金や玉の利き合成用に一時的に使う。init関数が終われば解放する。
    for (int sq = SQ_ZERO; sq < SQ_NB; ++sq) {
        int file = sq / 5; // 0 ~ 4
        int rank = sq % 5; // 0 ~ 4

        // 初期化
        CROSS_EFFECT_BB[sq] = Bitboard(0);

        // 上方向の利き
        if (rank > 0) {
            int target_sq = sq + SQ_UP;
            CROSS_EFFECT_BB[sq].p |= (1 << target_sq);
        }

        // 下方向の利き
        if (rank < 4) {
            int target_sq = sq + SQ_DOWN;
            CROSS_EFFECT_BB[sq].p |= (1 << target_sq);
        }

        // 左方向の利き
        if (file < 4) {
            int target_sq = sq + SQ_LEFT;
            CROSS_EFFECT_BB[sq].p |= (1 << target_sq);
        }

        // 右方向の利き
        if (file > 0) {
            int target_sq = sq + SQ_RIGHT;
            CROSS_EFFECT_BB[sq].p |= (1 << target_sq);
        }
    }

    // X字方向の利きテーブル
    for (int sq = SQ_ZERO; sq < SQ_NB; ++sq) {
        int file = sq / 5; // 0 ~ 4
        int rank = sq % 5; // 0 ~ 4

        // 初期化
        X_EFFECT_BB[sq] = Bitboard(0);

        // 右上方向の利き
        if (rank > 0 && file > 0) {
            int target_sq = sq + SQ_UPPER_RIGHT;
            X_EFFECT_BB[sq].p |= (1 << target_sq);
        }

        // 左上方向の利き
        if (rank > 0 && file < 4) {
            int target_sq = sq + SQ_UPPER_LEFT;
            X_EFFECT_BB[sq].p |= (1 << target_sq);
        }

        // 右下方向の利き
        if (rank < 4 && file > 0) {
            int target_sq = sq + SQ_LOWER_RIGHT;
            X_EFFECT_BB[sq].p |= (1 << target_sq);
        }

        // 左下方向の利き
        if (rank < 4 && file < 4) {
            int target_sq = sq + SQ_LOWER_LEFT;
            X_EFFECT_BB[sq].p |= (1 << target_sq);
        }
    }

    // 金の利きテーブル
    for (int sq = SQ_ZERO; sq < SQ_NB; ++sq) {
        int file = sq / 5; // 0 ~ 4
        int rank = sq % 5; // 0 ~ 4

        // 黒番の金の利き
        GOLD_EFFECT_BB[sq][BLACK] = CROSS_EFFECT_BB[sq];
        // 右上方向の利き
        if (rank > 0 && file > 0) {
            int target_sq = sq + SQ_UPPER_RIGHT;
            GOLD_EFFECT_BB[sq][BLACK].p |= (1 << target_sq);
        }
        // 左上方向の利き
        if (rank > 0 && file < 4) {
            int target_sq = sq + SQ_UPPER_LEFT;
            GOLD_EFFECT_BB[sq][BLACK].p |= (1 << target_sq);
        }

        // 白番の金の利き
        GOLD_EFFECT_BB[sq][WHITE] = CROSS_EFFECT_BB[sq];
        // 右下方向の利き
        if (rank < 4 && file > 0) {
            int target_sq = sq + SQ_LOWER_RIGHT;
            GOLD_EFFECT_BB[sq][WHITE].p |= (1 << target_sq);
        }
        // 左下方向の利き
        if (rank < 4 && file < 4) {
            int target_sq = sq + SQ_LOWER_LEFT;
            GOLD_EFFECT_BB[sq][WHITE].p |= (1 << target_sq);
        }
    }

    // 銀の利きテーブル
    for (int sq = SQ_ZERO; sq < SQ_NB; ++sq) {
        // 黒番の銀の利きは、黒番の歩の利きと、X字方向の利きの合成で表現できる
        SILVER_EFFECT_BB[sq][BLACK] =
            X_EFFECT_BB[sq].p | PAWN_EFFECT_BB[sq][BLACK].p;

        // 白番の銀の利き
        SILVER_EFFECT_BB[sq][WHITE] =
            X_EFFECT_BB[sq].p | PAWN_EFFECT_BB[sq][WHITE].p;
    }

    // 玉の利きテーブル
    for (int sq = SQ_ZERO; sq < SQ_NB; ++sq) {
        // 玉の利きは、十字方向の利きとX字方向の利きの合成で表現できる
        KING_EFFECT_BB[sq] = CROSS_EFFECT_BB[sq].p | X_EFFECT_BB[sq].p;
    }

    // 他のコマの存在を一切考慮しない場合の角の利きテーブル
    for (int sq = SQ_ZERO; sq < SQ_NB; ++sq) {
        int file = sq / 5; // 0 ~ 4
        int rank = sq % 5; // 0 ~ 4

        // 右上に進む利き
        int target_sq = sq;
        int tmp_file = file;
        int tmp_rank = rank;
        while (tmp_rank > 0 && tmp_file > 0) {
            target_sq += SQ_UPPER_RIGHT;
            BISHOP_EFFECT_BB[sq][DIRECTION_UPPER_RIGHT].p |= (1 << target_sq);
            tmp_file--;
            tmp_rank--;
        }

        // 右下に進む利き
        target_sq = sq;
        tmp_file = file;
        tmp_rank = rank;
        while (tmp_rank < 4 && tmp_file > 0) {
            target_sq += SQ_LOWER_RIGHT;
            BISHOP_EFFECT_BB[sq][DIRECTION_LOWER_RIGHT].p |= (1 << target_sq);
            tmp_file--;
            tmp_rank++;
        }

        // 左下に進む利き
        target_sq = sq;
        tmp_file = file;
        tmp_rank = rank;
        while (tmp_rank < 4 && tmp_file < 4) {
            target_sq += SQ_LOWER_LEFT;
            BISHOP_EFFECT_BB[sq][DIRECTION_LOWER_LEFT].p |= (1 << target_sq);
            tmp_file++;
            tmp_rank++;
        }

        // 左上に進む利き
        target_sq = sq;
        tmp_file = file;
        tmp_rank = rank;
        while (tmp_rank > 0 && tmp_file < 4) {
            target_sq += SQ_UPPER_LEFT;
            BISHOP_EFFECT_BB[sq][DIRECTION_UPPER_LEFT].p |= (1 << target_sq);
            tmp_file++;
            tmp_rank--;
        }

        BISHOP_EFFECT_BB[sq][DIRECTION_X] =
            BISHOP_EFFECT_BB[sq][DIRECTION_UPPER_RIGHT].p |
            BISHOP_EFFECT_BB[sq][DIRECTION_LOWER_RIGHT].p |
            BISHOP_EFFECT_BB[sq][DIRECTION_LOWER_LEFT].p |
            BISHOP_EFFECT_BB[sq][DIRECTION_UPPER_LEFT].p;
    }

    // 飛車の利きテーブル
    for (int sq = SQ_ZERO; sq < SQ_NB; ++sq) {
        int file = sq / 5; // 0 ~ 4
        int rank = sq % 5; // 0 ~ 4

        // 右に進む利き
        int target_sq = sq;
        int tmp_file = file;
        while (tmp_file > 0) {
            target_sq += SQ_RIGHT;
            ROOK_EFFECT_BB[sq][DIRECTION_RIGHT].p |= (1 << target_sq);
            tmp_file--;
        }

        // 下に進む利き
        target_sq = sq;
        int tmp_rank = rank;
        while (tmp_rank < 4) {
            target_sq += SQ_DOWN;
            ROOK_EFFECT_BB[sq][DIRECTION_DOWN].p |= (1 << target_sq);
            tmp_rank++;
        }

        // 左に進む利き
        target_sq = sq;
        tmp_file = file;
        while (tmp_file < 4) {
            target_sq += SQ_LEFT;
            ROOK_EFFECT_BB[sq][DIRECTION_LEFT].p |= (1 << target_sq);
            tmp_file++;
        }

        // 上に進む利き
        target_sq = sq;
        tmp_rank = rank;
        while (tmp_rank > 0) {
            target_sq += SQ_UP;
            ROOK_EFFECT_BB[sq][DIRECTION_UP].p |= (1 << target_sq);
            tmp_rank--;
        }

        ROOK_EFFECT_BB[sq][DIRECTION_CROSS] =
            ROOK_EFFECT_BB[sq][DIRECTION_RIGHT].p |
            ROOK_EFFECT_BB[sq][DIRECTION_DOWN].p |
            ROOK_EFFECT_BB[sq][DIRECTION_LEFT].p |
            ROOK_EFFECT_BB[sq][DIRECTION_UP].p;
    }

    // 各筋のBitboard
    FILE_BB[FILE_1] = Bitboard(0x1F);
    FILE_BB[FILE_2] = Bitboard(0x3E0);
    FILE_BB[FILE_3] = Bitboard(0x7C00);
    FILE_BB[FILE_4] = Bitboard(0xF8000);
    FILE_BB[FILE_5] = Bitboard(0x1F00000);

    PROMOTE_ZONE[BLACK] = Bitboard(0x108421);  // 黒が成れる場所
    PROMOTE_ZONE[WHITE] = Bitboard(0x1084210); // 白が成れる場所

    // --------------------
    //   BETWEEN_BBの初期化
    // --------------------

    // 2升に挟まれている升を返すためのテーブル(その2升は含まない)
    uint16_t between_index = 1;
    // BETWEEN_BB[0] == ZERO_BBであることを保証する。

    Bitboard bb;

    for (int i = SQ_ZERO; i < SQ_NB; ++i)
        for (int j = SQ_ZERO; j < SQ_NB; ++j) {
            if (i >= j)
                continue;

            // 筋が同じ場合
            if (i / 5 == j / 5) {
                // i<jだから、iから見たjの方角は下である
                Bitboard bb1 = ROOK_EFFECT_BB[i][DIRECTION_DOWN];
                // std::cout << "bb1 = " << std::endl << bb1 << std::endl;
                // 逆に、jから見たiの方角は上である
                Bitboard bb2 = ROOK_EFFECT_BB[j][DIRECTION_UP];
                // std::cout << "bb2 = " << std::endl << bb2 << std::endl;
                bb = bb1 & bb2;
                // std::cout << "bb1 & bb2  = " << std::endl << bb << std::endl;
            }
            // 段が同じ場合
            else if (i % 5 == j % 5) {
                // i<jだから、iから見たjの方角は左である
                Bitboard bb1 = ROOK_EFFECT_BB[i][DIRECTION_LEFT];
                // 逆に、jから見たiの方角は右である
                Bitboard bb2 = ROOK_EFFECT_BB[j][DIRECTION_RIGHT];
                bb = bb1 & bb2;
            }
            // iが右下でjが左上の関係
            else if (i / 5 + i % 5 == j / 5 + j % 5) {
                // i<jだから、iから見たjの方角は左上である
                Bitboard bb1 = BISHOP_EFFECT_BB[i][DIRECTION_UPPER_LEFT];
                // 逆に、jから見たiの方角は右下である
                Bitboard bb2 = BISHOP_EFFECT_BB[j][DIRECTION_LOWER_RIGHT];
                bb = bb1 & bb2;
            }
            // iが右上でjが左下の関係
            else if (i / 5 - i % 5 == j / 5 - j % 5) {
                // i<jだから、iから見たjの方角は左下である
                Bitboard bb1 = BISHOP_EFFECT_BB[i][DIRECTION_LOWER_LEFT];
                // 逆に、jから見たiの方角は右上である
                Bitboard bb2 = BISHOP_EFFECT_BB[j][DIRECTION_UPPER_RIGHT];
                bb = bb1 & bb2;
            } else {
                // それ以外の場合は、iとjの間に駒が存在しない
                bb = ZERO_BB;
            }

            BETWEEN_INDEX[i][j] = between_index;
            BETWEEN_BB[between_index++] = bb;
        }

    // 対称性を利用する
    for (int i = SQ_ZERO; i < SQ_NB; ++i)
        for (int j = SQ_ZERO; j < SQ_NB; ++j)
            if (i > j)
                BETWEEN_INDEX[i][j] = BETWEEN_INDEX[j][i];

    // indexがサイズを超えてないか//ASSERTする
    // ASSERT(between_index == BETWEEN_INDEX_SIZE,
    //        "between_index == BETWEEN_INDEX_SIZE");

    // --------------------
    //   ハッシュキーの初期化
    // --------------------
    std::random_device rnd;
    std::mt19937 mt;
    // mt.seed(rnd());
    mt.seed(20231024);

    // Squareの乱数表の初期化
    for (Square sq = SQ_ZERO; sq < SQ_NB; ++sq) {
        uint64_t r = (((uint64_t)mt() << 32) | (uint64_t)mt());
        Bitboards::square[sq] = r;
    }

    // 駒の配置の乱数表の初期化
    for (Square sq = SQ_ZERO; sq < SQ_NB; ++sq) {
        uint64_t r = (((uint64_t)mt() << 32) | (uint64_t)mt());
        Bitboards::occ[sq] = r;
    }

    // --------------------
    //   ハッシュマップの初期化
    // --------------------

    std::unordered_map<Square, Bitboard> bishop_effect_hash;
    std::unordered_map<Square, Bitboard> rook_effect_hash;
    bishop_effect_hash.reserve(BISHOP_TABLE_SIZE);
    rook_effect_hash.reserve(ROOK_TABLE_SIZE);
}

HASH_KEY Bitboards::h(Square sq, Bitboard occ) {
    HASH_KEY key = 0;
    key ^= Bitboards::square[sq];
    while (occ.p != 0) {
        Square s = occ.pop();
        key ^= Bitboards::occ[s];
    }
    return key;
}

// ビットを逆順にする
uint32_t reverse_bits(uint32_t num) {
    num = ((num & 0xaaaaaaaa) >> 1) | ((num & 0x55555555) << 1);
    num = ((num & 0xcccccccc) >> 2) | ((num & 0x33333333) << 2);
    num = ((num & 0xf0f0f0f0) >> 4) | ((num & 0x0f0f0f0f) << 4);
    num = ((num & 0xff00ff00) >> 8) | ((num & 0x00ff00ff) << 8);
    return (num >> 16) | (num << 16);
}

// 下位7ビットが0になっているので右にシフトしたものを返す
Bitboard reverse_bitboard(uint32_t num) { return reverse_bits(num) >> 7; }