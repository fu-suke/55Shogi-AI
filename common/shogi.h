#pragma once

#include <cstdint> // uint8_t, uint16_tなどの型を使えるようにする
#include <iostream>
#include <map>
#include <string>

// ASSERTを使うためのマクロ
#include <cstdlib>

typedef uint64_t HASH_KEY;

#define ASSERT(cond, msg)                                                      \
    do {                                                                       \
        if (!(cond)) {                                                         \
            std::cerr << "-------------------------" << std::endl;             \
            std::cerr << "//ASSERTion failed: (" #cond ")" << std::endl        \
                      << "function: " << __func__ << std::endl                 \
                      << "file: " << __FILE__ << ", "                          \
                      << "line " << __LINE__ << std::endl                      \
                      << "Message: " << msg << std::endl;                      \
            std::cerr << "-------------------------" << std::endl;             \
            std::cerr << "bestmove resign" << std::endl;                       \
            std::exit(EXIT_FAILURE);                                           \
        }                                                                      \
    } while (false)

// 手番
enum Color : int32_t {
    BLACK = 0,
    WHITE = 1,
    COLOR_ALL = 2,
    COLOR_NB = 2,
    COLOR_ZERO = 0,
};

// インクリメント
inline Color operator++(Color &c) { return c = (Color)(c + 1); } // i++

// 反転用のオペレータ
inline Color operator~(Color c) { return (Color)(c ^ 1); }

inline std::ostream &operator<<(std::ostream &os, Color c) {
    return os << (c == BLACK ? "BLACK" : "WHITE");
}

// --------------------
//        升目
// --------------------

// 盤上の升目に対応する定数。
// 盤上右上(1aが0)、左下(5e)が24
// 方角を表現するときにマイナスの値を使うので符号型である必要がある
enum Square : int32_t {
    // 以下、盤面の右上から左下までの定数。
    // これを定義していなくとも問題ないのだが、デバッガでSquare型を見たときに
    // どの升であるかが表示されることに価値がある。
    SQ_1A,
    SQ_1B,
    SQ_1C,
    SQ_1D,
    SQ_1E,
    SQ_2A,
    SQ_2B,
    SQ_2C,
    SQ_2D,
    SQ_2E,
    SQ_3A,
    SQ_3B,
    SQ_3C,
    SQ_3D,
    SQ_3E,
    SQ_4A,
    SQ_4B,
    SQ_4C,
    SQ_4D,
    SQ_4E,
    SQ_5A,
    SQ_5B,
    SQ_5C,
    SQ_5D,
    SQ_5E,

    // ゼロと末尾
    SQ_ZERO = 0,
    SQ_NB = 25,
    SQ_NULL = 26, // マスが見つからなかった時に返す値

    // 方角に関する定数
    SQ_DOWN = +1,
    SQ_RIGHT = -5,
    SQ_UP = -1,
    SQ_LEFT = +5,

    // 斜めの方角などを意味する定数。
    SQ_UPPER_RIGHT = int(SQ_UP + SQ_RIGHT),
    SQ_UPPER_LEFT = int(SQ_UP + SQ_LEFT),
    SQ_LOWER_RIGHT = int(SQ_DOWN + SQ_RIGHT),
    SQ_LOWER_LEFT = int(SQ_DOWN + SQ_LEFT),
};

// Square型の++演算子のオーバーロード
inline Square operator++(Square &sq) { return sq = (Square)(sq + 1); } // i++

enum File : int32_t {
    FILE_ZERO = 0,
    FILE_1,
    FILE_2,
    FILE_3,
    FILE_4,
    FILE_5,
    FILE_NB,
};

// 飛車・角の利きの配列用の定数
enum Direction : uint32_t {

    // 角
    DIRECTION_UPPER_RIGHT = 0,
    DIRECTION_LOWER_RIGHT = 1,
    DIRECTION_LOWER_LEFT = 2,
    DIRECTION_UPPER_LEFT = 3,
    DIRECTION_X = 4,

    // 飛車
    DIRECTION_RIGHT = 0,
    DIRECTION_DOWN = 1,
    DIRECTION_LEFT = 2,
    DIRECTION_UP = 3,
    DIRECTION_CROSS = 4,
    DIRECTION_NB, // 利きの方向の数
};

// Square型を文字列に変換する
inline std::string sq_to_string(Square sq) {
    static const char *file_str = "12345"; // 筋
    static const char *rank_str = "abcde"; // 団

    std::string result = "";
    result += file_str[sq / 5];
    result += rank_str[sq % 5];

    return result;
}

// Squareがどの筋にあるかを返す
inline File sq_to_file(Square sq) {
    if (sq == SQ_NULL) {
        return FILE_ZERO;
    }
    return static_cast<File>(sq / 5 + 1);
}

// Square型の演算子のオーバーロード

// 右シフト
inline Square operator>>(Square sq, int n) {
    return static_cast<Square>(static_cast<int>(sq) >> n);
}

// 左シフト
inline Square operator<<(Square sq, int n) {
    return static_cast<Square>(static_cast<int>(sq) << n);
}

// ビットAND
inline Square operator&(Square sq, int mask) {
    return static_cast<Square>(static_cast<int>(sq) & mask);
}

// ビットAND代入
inline Square &operator&=(Square &sq, int mask) {
    sq = sq & mask;
    return sq;
}

enum Piece : int32_t {

    // 注意！！！！！！！！！！！！！！

    // ここの値をいじくるとどこかでエラーが発生する可能性があるので、
    // いじくる前にその状態を必ず保存すること。
    // いじくったら、makeしてプログラムの動作チェックを必ず行うこと。

    NO_PIECE,
    RAW_PIECE_BEGIN = 1,
    PIECE_BEGIN = 1,
    GOLD = 1,      // 金
    KING,          // 玉
    PAWN = 3,      // 歩
    SILVER,        // 銀
    BISHOP,        // 角
    ROOK,          // 飛
    RAW_PIECE_NB,  // 駒種の終端
    PRO_PAWN = 11, // と
    PRO_SILVER,    // 成銀
    HORSE,         // 馬
    DRAGON,        // 龍
    PIECE_NB,      // 駒種の終端
    // 以下、先後の区別のある駒(Bがついているのは先手、Wがついているのは後手)
    COLOR_PIECE_BEGIN = 1,
    B_GOLD = 1,
    B_KING,
    B_PAWN,
    B_SILVER,
    B_BISHOP,
    B_ROOK,
    B_PRO_PAWN = 11,
    B_PRO_SILVER,
    B_HORSE,
    B_DRAGON,
    W_GOLD = 17,
    W_KING,
    W_PAWN = 19,
    W_SILVER,
    W_BISHOP,
    W_ROOK,
    W_PRO_PAWN = 27,
    W_PRO_SILVER,
    W_HORSE,
    W_DRAGON,
    COLOR_PIECE_NB, // 先後の区別のある駒種の終端
    PIECE_ZERO = 0,

    PIECE_PROMOTE = 8, //
    // 成り駒と非成り駒との差(この定数を足すと成り駒になる)
    PIECE_WHITE = 16, // 後手の駒との差(この定数を足すと後手の駒になる)
};

constexpr Color color_of(Piece pc) {
    // ASSERT(pc != NO_PIECE, "pc is NO_PIECE !!!");
    return (Color)((pc & PIECE_WHITE) >> 4);
}
// 先後の区別のない駒種（成りは考慮する）を返す
constexpr Piece type_of(Piece pc) { return (Piece)(pc & ~PIECE_WHITE); }
constexpr bool is_promoted(Piece pc) { return (pc & PIECE_PROMOTE) >> 3; }
// Piece同士の加算
inline Piece operator+(Piece pc1, Piece pc2) {
    return (Piece)((int)pc1 + (int)pc2);
}
inline Piece operator|(Piece pc1, int n) { return (Piece)((int)pc1 | n); }
inline Piece operator&(Piece pc1, int n) { return (Piece)((int)pc1 & n); }
inline Piece operator++(Piece &pc) { return pc = (Piece)(pc + 1); } // i++
inline Piece operator++(Piece &pc, int) {
    Piece original = pc;
    pc = (Piece)(pc + 1);
    return original;
}

inline Piece to_promote(Piece pc) { return (Piece)(pc | PIECE_PROMOTE); }
inline Piece unpromote(Piece pc) { return (Piece)(pc & ~PIECE_PROMOTE); }
inline Piece to_raw(Piece pc) { return (Piece)(unpromote(pc) & ~PIECE_WHITE); }

inline const std::map<Piece, char> PIECE_TO_CHAR = {
    {B_PAWN, 'P'},     {B_SILVER, 'S'},     {B_GOLD, 'G'},
    {B_BISHOP, 'B'},   {B_KING, 'K'},       {B_ROOK, 'R'},
    {B_PRO_PAWN, 'P'}, {B_PRO_SILVER, 'S'}, {B_HORSE, 'B'},
    {B_DRAGON, 'R'},   {W_PAWN, 'p'},       {W_SILVER, 's'},
    {W_GOLD, 'g'},     {W_BISHOP, 'b'},     {W_KING, 'k'},
    {W_ROOK, 'r'},     {W_PRO_PAWN, 'p'},   {W_PRO_SILVER, 's'},
    {W_HORSE, 'b'},    {W_DRAGON, 'r'},     {NO_PIECE, '-'}};

inline const std::map<char, Piece> CHAR_TO_RAW_PIECE = {
    {'P', PAWN},   {'S', SILVER}, {'G', GOLD},
    {'B', BISHOP}, {'K', KING},   {'R', ROOK}};

// Pieceの標準出力用オーバーロード
inline std::ostream &operator<<(std::ostream &os, Piece pc) {
    return os << PIECE_TO_CHAR.at(pc);
}

// --------------------
//       手駒
// --------------------

// 手駒
// プレイヤーの手駒を2bitで表現する。
enum Hand : uint16_t {
    HAND_ZERO = 0,
};

// 手駒のbit位置を定義するテーブル
// 玉は本来手駒にならないが、評価関数で用いる場合があるので、ここに含めておく。
inline const std::map<int, int> HAND_PIECE_BITS = {
    {PAWN, PAWN * 2}, {SILVER, SILVER * 2}, {BISHOP, BISHOP * 2},
    {ROOK, ROOK * 2}, {GOLD, GOLD * 2},     {KING, KING * 2}};

// Piece(歩,銀,金,角,飛)を手駒に変換するテーブル
inline const std::map<int, Hand> PIECE_TO_HAND = {
    {PAWN, (Hand)(1 << HAND_PIECE_BITS.at(PAWN))},
    {SILVER, (Hand)(1 << HAND_PIECE_BITS.at(SILVER))},
    {BISHOP, (Hand)(1 << HAND_PIECE_BITS.at(BISHOP))},
    {ROOK, (Hand)(1 << HAND_PIECE_BITS.at(ROOK))},
    {GOLD, (Hand)(1 << HAND_PIECE_BITS.at(GOLD))},
    {KING, (Hand)(1 << HAND_PIECE_BITS.at(KING))}};

// 持ち駒の枚数を表現するために必要なビット
// 全ての駒の枚数は0～2枚なので、2bitあれば十分。
constexpr int PIECE_BIT_MASK = 0b11;

// Handの中にあるprの枚数を返す。
inline int hand_count(Hand hand, Piece pr) {
    // handを右シフトして、最下位ビットにマスクをかければOK
    return (hand >> HAND_PIECE_BITS.at(pr) & PIECE_BIT_MASK);
}

// 手駒prを持っているかどうかを返す。
inline int hand_exists(Hand hand, Piece pr) {
    return hand & (PIECE_TO_HAND.at(pr) | PIECE_TO_HAND.at(pr) << 1);
}

// 手駒にprを1枚加える
inline void add_hand(Hand &hand, Piece pr) {
    hand = (Hand)(hand + PIECE_TO_HAND.at(pr));
}

// 手駒からprを1枚減らす
inline void sub_hand(Hand &hand, Piece pr) {
    // 送られて来たPieceが生駒であることを確認する
    // ASSERT(!is_promoted(pr), "Piece is promoted !!!");
    hand = (Hand)(hand - PIECE_TO_HAND.at(pr));
}

// 手駒を表示する(USI形式ではない) デバッグ用
inline std::ostream &operator<<(std::ostream &os, Hand hand) {
    std::cout << "hand: " << hand_count(hand, PAWN) << "P"
              << hand_count(hand, SILVER) << "S" << hand_count(hand, GOLD)
              << "G" << hand_count(hand, BISHOP) << "B"
              << hand_count(hand, ROOK) << "R" << std::endl;
    return os;
}

// 両サイドの手駒を表示する
inline std::ostream &operator<<(std::ostream &os, Hand hands[]) {
    std::cout << "BLACK: " << hands[BLACK] << std::endl;
    std::cout << "WHITE: " << hands[WHITE] << std::endl;
    return os;
}
