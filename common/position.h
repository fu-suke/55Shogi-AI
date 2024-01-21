#pragma once

#include "movegen.h"
// #include "shogi.h"
#include "zobrist.h"
#include <map>
#include <random>
#include <string>

struct Move;

extern std::vector<HASH_KEY> visited_hash_keys;

class Position {
  public:
    // 盤面情報
    // ここ、ゼロクリアしないとバグるので必ず右辺に初期値を書くこと
    Color side_to_move = BLACK;                    // 手番
    Piece piece_board[SQ_NB] = {};                 // 盤上の駒の配置
    Bitboard piece_bitboards[COLOR_PIECE_NB] = {}; // 盤上の駒の配置
    Hand hands[COLOR_NB] = {};                     // 持ち駒

    // コンストラクタ
    Position();
    Position(const Position &pos);

    // Moveを受取って盤面情報を更新する関数たち
    void do_move(const Move &move);
    void undo_move(const Move &move);
    void set_captured_piece(Move &move);

    // Squareを受取って盤面情報を更新する関数たち
    Piece clear_piece(Square sq);
    void set_piece(Square sq, Piece pc);
    Piece move_piece(Square from, Square to, bool is_promote);
    void unmove_piece(Square from, Square to, bool unpromote);

    // 利きを計算する関数
    Bitboard pawn_effect(Square sq, Color color);
    Bitboard silver_effect(Square sq, Color color);
    Bitboard gold_effect(Square sq, Color color);
    Bitboard bishop_effect(Square sq, Color color);
    Bitboard rook_effect(Square sq, Color color);
    Bitboard king_effect(Square sq, Color color);
    Bitboard horse_effect(Square sq, Color color);
    Bitboard dragon_effect(Square sq, Color color);
    Bitboard all_effect(Color color);

    // 表示系
    void display_piece_board();
    void display_bitboards();
    void debug_all_bitboards();
    inline void display_hands() { std::cout << hands << std::endl; }

    // 便利系
    bool is_check(Color color);
    Square king_square(Color color);
    Bitboard occupied_bb(Color color);
    HASH_KEY get_hash_key();

    // リストを渡されたら、その中からランダムに合法手を1つを選んで返す。
    // 非合法の手しかない場合、NONEを返す。
    Move select_random_move(std::vector<Move> &move_list);
    // 重みつきの手を選ぶ
    Move select_weighted_random_move(std::vector<Move> &move_list);
    Move select_by_weight(std::vector<Move> &mlist1, std::vector<Move> &mlist2,
                          int w1, int w2);
    Move select_by_weight(std::vector<Move> &mlist1, std::vector<Move> &mlist2,
                          std::vector<Move> &mlist3, int w1, int w2, int w3);
    Move select_by_weight(std::vector<Move> &mlist1, std::vector<Move> &mlist2,
                          std::vector<Move> &mlist3, std::vector<Move> &mlist4,
                          int w1, int w2, int w3, int w4);
};

// 関数ポインタを定義することで、関数名を変数名のように利用できる
/* 例
Bitboard someFunction(Square sq, Color col, Position &pos);
EffectFunc myFuncPointer = someFunction;
*/

// EffectFuncの定義
typedef Bitboard (Position::*EffectFunc)(Square sq, Color color);
inline std::map<Piece, EffectFunc> Piece_to_EffectFunc = {
    {PAWN, &Position::pawn_effect},     {SILVER, &Position::silver_effect},
    {GOLD, &Position::gold_effect},     {BISHOP, &Position::bishop_effect},
    {ROOK, &Position::rook_effect},     {KING, &Position::king_effect},
    {PRO_PAWN, &Position::gold_effect}, {PRO_SILVER, &Position::gold_effect},
    {HORSE, &Position::horse_effect},   {DRAGON, &Position::dragon_effect},
};