#pragma once

#include "bitboard.h"
#include "position.h"
#include "shogi.h"
#include <cstdint> // uint8_t, uint16_tなどの型を使えるようにする
#include <iostream>
#include <string>
#include <utility> // std::swap
#include <vector>

class Position;

int sq_to_index(const std::string &sq);

// 可変長配列のインスタンスを生成

// 指し手
// bit0..4:移動先のSquare
// bit5..9:移動元のSquare or 駒打ちの場合は打つ駒種
// bit10..13:獲得する駒種（先後の区別なし！！！）
// bit14..駒打ちフラグ
// bit15..成りフラグ
struct Move {
    // 実際のMoveのデータを格納するメンバ
    uint16_t value;

    enum MoveType : uint16_t {
        NONE = 0,              // 無効な移動 SQ_1A -> SQ_1A
        RESIGN = (1 << 6) + 2, // 投了を意味する指し手 SQ_1C -> SQ_1C
        DROP = 1 << 14,        // 駒打ちフラグ
        PROMOTE = 1 << 15      // 駒成りフラグ
    };

    Move();
    Move(MoveType move_type);
    Move(const std::string &move);
    Move(Square from, Square to);
    Move(Piece pr, Square to);

    Square get_from() const { return static_cast<Square>((value >> 5) & 0x1F); }
    Square get_to() const { return static_cast<Square>(value & 0x1F); }

    void promote() { value |= PROMOTE; }
    bool is_promote() const { return value & PROMOTE; }
    bool is_drop() const { return value & DROP; }
    // DROPやPROMOTEと違い、フラグを立てているわけではないのでビット演算はダメ
    bool is_resign() const { return value == RESIGN; }
    bool is_none() const { return value == NONE; }
    // 
    bool is_check(Position &pos) const;
    // 自分の駒の利きがなく、かつ相手の駒の利きがあるような場所（＝タダ）に移動する指し手かどうかを判定する
    bool is_danger(Position &pos, Bitboard &danger_zone) const;
    
    Piece get_dropped_piece() const {
        return static_cast<Piece>((value >> 5) & 0x1F);
    }
    Piece set_dropped_piece(Piece pr) {
        // prが生駒であることを保証する
        ASSERT(!is_promoted(pr), "This piece is promoted !!!");
        value |= static_cast<uint16_t>(pr << 5);
        return pr;
    }
    // 注意！！！返ってくる駒は先後の区別がない！！！
    Piece get_captured_piece() const {
        return static_cast<Piece>((value >> 10) & 0x0F);
    }
    // 捕られる駒種（先後の区別なし）をセットする。引数は区別のある駒でOK。
    // ※成りは考慮する
    void set_captured_piece(Piece pc) {
        pc = static_cast<Piece>(pc & ~PIECE_WHITE);
        value |= static_cast<uint16_t>(pc << 10);
    }

    // その指し手が成れる指し手かどうかを判定する
    bool can_promote(Color color) const {
        // 駒打ちの場合は成れない
        if (is_drop()) {
            return false;
        }
        Square from = static_cast<Square>(
            (value >> 5) & 0x1F); // moveの上位5ビットは移動元の位置
        Square to = static_cast<Square>(
            value & 0x1F); // moveの下位5ビットは移動先の位置
        Bitboard from_bb = Bitboard(1U << from);
        Bitboard to_bb = Bitboard(1U << to);
        return ((from_bb | to_bb) & PROMOTE_ZONE[color]).p;
    }

    // 暗黙の変換を可能にする
    operator uint16_t() const { return value; }
};

// Move型を標準出力するためのオーバーロード
inline std::ostream &operator<<(std::ostream &os, Move move) {
    if (move.is_resign()) {
        os << "resign";
        return os;
    }

    if (move.is_none()) {
        os << "none";
        return os;
    }

    Square to = static_cast<Square>(move & 0x1F);

    // 駒打ちフラグ
    std::string drop = "";
    if (move.value & Move::DROP) {
        Piece pr = static_cast<Piece>((move >> 5) & 0x1F);
        char c = PIECE_TO_CHAR.at(pr);
        drop = "*";
        os << c << drop << sq_to_string(to);
        return os;
    }

    // 成りフラグ
    std::string promote = "";
    if (move.value & Move::PROMOTE) {
        promote = "+";
    }
    Square from = static_cast<Square>((move >> 5) & 0x1F);
    os << sq_to_string(from) << sq_to_string(to) << promote;
    return os;
}

std::vector<Move> generate_move_list(Position &pos);
std::vector<Move> generate_move_list(Position &pos,
                                     std::vector<Move> &move_list);
std::vector<Move> generate_capture_mlist(const std::vector<Move> &move_list);
void sort_move_list(std::vector<Move> &move_list, Position &pos);
void generate_moves(Color color, std::vector<Move> &move_list, Position &pos);
void generate_drop_moves(Color color, std::vector<Move> &move_list,
                         Position &pos);
void generate_drop_moves(Color color, Bitboard target,
                         std::vector<Move> &move_list, Position &pos);
void generate_block_moves(Color color, std::vector<Move> &move_list,
                          Position &pos);
Move generate_king_capture_move(Color color, std::vector<Move> &move_list,
                                Position &pos);
bool is_safe_move(Square from, Square to, Color color, Position &pos);
bool is_safe_move(Move move, Color color, Position &pos);

template <Piece piece>
void generate_piece_moves(Color color, std::vector<Move> &mlist, Position &pos);

template <Piece piece>
void generate_piece_moves(Color color, std::vector<Move> &mlist,
                          Bitboard target, Position &pos);

inline Move &operator|=(Move &m, Move value) {
    m.value |= value.value;
    return m;
}

inline std::ostream &operator<<(std::ostream &os, std::vector<Move> move_list) {
    for (auto m : move_list) {
        os << m << ' ';
    }
    return os;
}

inline bool invalid_pawn_drop(Square drop_to, Color color) {
    Bitboard to_bb = (1U << drop_to);
    return (to_bb & PROMOTE_ZONE[color]).p;
}
