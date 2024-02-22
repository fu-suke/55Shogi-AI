#pragma once

#include "shogi.h"
#include <cstdint> // uint8_t, uint16_tなどの型を使えるようにする
#include <iostream>
#include <map>
#include <unordered_map> // ハッシュテーブル用

unsigned ctz(unsigned x);


struct Bitboard {
    union {
        uint32_t p;
    };

    Bitboard() = default;
    Bitboard(uint32_t p) : p(p) {}
    Bitboard(Square sq) : p(1U << sq) {}

    void set_bit(Square sq) { p |= (1U << sq); }
    void clear_bit(Square sq) { p &= ~(1U << sq); }
    bool check_bit(Square sq) const { return p & (1U << sq); }
    void clear() { p = 0; }

    // ビットボードのビットが1である最も低い位の位置を返し、Bitboardからそのビットをクリアする
    inline Square pop() {
        if (p == 0)
            return SQ_NULL; // ビットボードが空の場合

        int sq_int = pop_lsb(p); // 最も低い位の1のビットを取得＆クリア
        return static_cast<Square>(sq_int); // ビットの位置（マス）を返す
    }

    static inline int pop_lsb(uint32_t &bb) {
        // int lsb = __builtin_ctz(bb); // 最も低い位の1のビットの位置を取得
        int lsb = ctz(bb); // 最も低い位の1のビットの位置を取得
        bb &= (bb - 1);    // 最も低い位の1をクリア
        return lsb;
    }

    friend Bitboard operator~(const Bitboard &bb);
    friend Bitboard operator|(const Bitboard &bb1, const Bitboard &bb2);
    friend Bitboard operator&(const Bitboard &bb1, const Bitboard &bb2);
    friend Bitboard operator^(const Bitboard &bb1, const Bitboard &bb2);
    friend Bitboard &operator|=(Bitboard &bb1, const Bitboard &bb2);
    friend Bitboard &operator&=(Bitboard &bb1, const Bitboard &bb2);
    friend Bitboard &operator^=(Bitboard &bb1, const Bitboard &bb2);
    friend bool operator&&(const Bitboard &bb1, const Bitboard &bb2) {
        return (bb1 & bb2).p != 0;
    }
    friend bool operator&&(const Bitboard &bb1, bool b) {
        return b ? bb1.p != 0 : false;
    }
    friend bool operator&&(bool b, const Bitboard &bb2) {
        return b ? bb2.p != 0 : false;
    }
    friend bool operator||(const Bitboard &bb1, const Bitboard &bb2) {
        return (bb1 | bb2).p != 0;
    }
    friend bool operator||(const Bitboard &bb1, bool b) {
        return b ? true : bb1.p != 0;
    }
    friend bool operator||(bool b, const Bitboard &bb2) {
        return b ? true : bb2.p != 0;
    }
    // std::cout << Bitboard と渡せるようにする
    friend std::ostream &operator<<(std::ostream &os, const Bitboard &bb);
};

#define BISHOP_TABLE_SIZE 1024
#define ROOK_TABLE_SIZE 6400

namespace Bitboards {
void init();
HASH_KEY h(Square sq, Bitboard occ);
// sqに駒がある状態を表す
extern HASH_KEY occ[SQ_NB];
// sqについて利きを計算することを表す
extern HASH_KEY square[SQ_NB];
extern std::unordered_map<HASH_KEY, Bitboard> hash_rook_effect;
extern std::unordered_map<HASH_KEY, Bitboard> hash_bishop_effect;
}; // namespace Bitboards

extern Bitboard PAWN_EFFECT_BB[SQ_NB][COLOR_NB];       // 歩の利き
extern Bitboard GOLD_EFFECT_BB[SQ_NB][COLOR_NB];       // 金の利き
extern Bitboard SILVER_EFFECT_BB[SQ_NB][COLOR_NB];     // 銀の利き
extern Bitboard KING_EFFECT_BB[SQ_NB];                 // 玉の利き
extern Bitboard BISHOP_EFFECT_BB[SQ_NB][DIRECTION_NB]; // 角方向の利き
extern Bitboard ROOK_EFFECT_BB[SQ_NB][DIRECTION_NB];   // 飛車の利き
extern Bitboard CROSS_EFFECT_BB[SQ_NB];                // 十字の利き
extern Bitboard X_EFFECT_BB[SQ_NB];                    // Xの利き

extern Bitboard FILE_BB[FILE_NB]; // 各筋の全ビットが1のBitboard
extern Bitboard
    PROMOTE_ZONE[COLOR_NB]; // [BLACK]なら、先手が成れる場所のBitboard
const Bitboard ZERO_BB = Bitboard(0); // 0のBitboard

#define BETWEEN_INDEX_SIZE 301 // 25*24/2 + 1
// 2升に挟まれている升を返すためのテーブル(その2升は含まない)
// この配列には直接アクセスせずにbetween_bb()を使うこと！！
extern Bitboard BETWEEN_BB[BETWEEN_INDEX_SIZE];
extern uint16_t BETWEEN_INDEX[SQ_NB][SQ_NB];

// 2升に挟まれている升を表すBitboardを返す。sq1とsq2が縦横斜めの関係にないときはZERO_BBが返る。
inline const Bitboard get_between_bb(Square sq1, Square sq2) {
    return BETWEEN_BB[BETWEEN_INDEX[sq1][sq2]];
}

Bitboard reverse_bitboard(uint32_t num);

inline Bitboard operator~(const Bitboard &bb) {
    return Bitboard(~bb.p & 0x1FFFFFF);
}

inline Bitboard operator|(const Bitboard &bb1, const Bitboard &bb2) {
    Bitboard result;
    result.p = bb1.p | bb2.p;
    return result;
}

inline Bitboard operator&(const Bitboard &bb1, const Bitboard &bb2) {
    Bitboard result;
    result.p = bb1.p & bb2.p;
    return result;
}

inline Bitboard operator^(const Bitboard &bb1, const Bitboard &bb2) {
    Bitboard result;
    result.p = bb1.p ^ bb2.p;
    return result;
}

inline Bitboard &operator|=(Bitboard &bb1, const Bitboard &bb2) {
    bb1.p |= bb2.p;
    return bb1;
}

inline Bitboard &operator&=(Bitboard &bb1, const Bitboard &bb2) {
    bb1.p &= bb2.p;
    return bb1;
}

inline Bitboard &operator^=(Bitboard &bb1, const Bitboard &bb2) {
    bb1.p ^= bb2.p;
    return bb1;
}

inline bool operator==(const Bitboard &bb1, const Bitboard &bb2) {
    return bb1.p == bb2.p;
}
