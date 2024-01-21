#pragma once
#include "../common/shogi.h"
#include <map>

const double INFTY = 10000;

// 深さは原則偶数にすること
const int MAX_DEPTH = 6;
const double PLAYER_WIN = 1;
const double PLAYER_LOSE = 0;

// 各駒の価値を定義
inline std::map<Piece, int> PIECE_VALUE_MAP = {
    {PAWN, 1}, {SILVER, 5},   {GOLD, 6},       {BISHOP, 8}, {ROOK, 10},
    {KING, 0}, {PRO_PAWN, 4}, {PRO_SILVER, 6}, {HORSE, 11}, {DRAGON, 12},
};
