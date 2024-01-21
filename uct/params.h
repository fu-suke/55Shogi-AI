#pragma once
#include "../common/shogi.h"
#include <map>

// const int UCT_LOOP_MAX = 3000; // UCTアルゴリズムの探索回数

const int PLAYOUT_CNT = 200; // UCBの1手あたりの探索回数
const double INFTY = 1e8;

// こっちは使わない。resultが-1~1で加算され続けると、
// result/cntが0に収束してしまう。
// const int SCORE_WIN = 1;
// const int SCORE_DRAW = 0;
// const int SCORE_LOSE = -1;

const double PLAYER_WIN = 1;
const double PLAYER_DRAW = 0.5;
const double PLAYER_LOSE = 0;

const int MAX_DEPTH = 12;
// プレイアウトを何手目まで進めるか（10~20くらいでいいかも）
const int PLAYOUT_LOOP_MAX = 6;
const int UCT_PER_MOVE = 1000; // 駒打ち以外の指し手1手あたりの探索回数
const int UCT_PER_DROP = 300; // 駒打ち1手あたりの探索回数

// ucbが、「node自体が持つ駒の価値」を重視する割合。
// この値を大きくすると、駒を捕る手に探索が集中する。
const double UCB_PIECE_WEIGHT = 0.2;
// 評価値が「node自体が持つ駒の価値」を重視する割合。
// α * (このノードの駒得具合) + (1-α) * (未来の勝率)
const double NODE_PIECE_WEGHT = 0.2;

// プレイアウト結果で駒の価値を重視する割合。基本的に1.0固定でいいと思う。
// と思っていたが、勝利より駒得を意識するような挙動になりがちなので少し割合を下げてもいいかも
const double PLAYOUT_PIECE_WEIGHT = 1.0;
// UCBのexplore項の係数。これを大きくすると探索が広がり、小さくすると探索が狭まる。
// 0.3くらいが、探索が広がりすぎず、かつ狭まりすぎず、ちょうどいい気がする。
const double C = 0.3;

const int PAWN_VALUE = 1;
const int SILVER_VALUE = 5;
const int GOLD_VALUE = 6;
const int BISHOP_VALUE = 8;
const int ROOK_VALUE = 10;
const int KING_VALUE = 0;
const int PRO_PAWN_VALUE = 4;
const int PRO_SILVER_VALUE = 6;
const int HORSE_VALUE = 11;
const int DRAGON_VALUE = 12;

inline std::map<Piece, int> PIECE_VALUE_MAP = {
    {PAWN, 1}, {SILVER, 5},   {GOLD, 6},       {BISHOP, 8}, {ROOK, 10},
    {KING, 0}, {PRO_PAWN, 4}, {PRO_SILVER, 6}, {HORSE, 11}, {DRAGON, 12},
};