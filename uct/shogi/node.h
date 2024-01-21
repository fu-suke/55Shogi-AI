#pragma once

#include "../common/movegen.h"
#include "../common/position.h"
#include "params.h"
#include <vector>

const int UCB_UNREACHED = 100000;
const int NODE_ILLEGAL = -99999;
const int SCORE_MAX = 1234567890;

extern int node_cnt;
extern double total_explore;
extern double total_exploit;
extern int max_depth;

class Node {
  public:
    Color player_color; // rootの手番の色
    Position pos;
    Move move; // このノードに来た時に実行する指し手
    std::vector<Node *> children = {};
    int play_cnt = 0;
    // このノードの勝率。ただし前の手番側から見た勝率である。
    double score = 0;
    // このノードが持つ駒の価値。ただし前の手番側から見た価値である。
    double node_piece_value = 0;
    int depth = 0;           // rootからの深さ
    bool is_illegal = false; // 違法手かどうか

    Node(Color player_color, Position pos, Move &move, int depth = 0);
    ~Node(); // デストラクタ

    double search();
    double do_playout();
    Node *select_child();
    double ucb(int parent_play_cnt);
    double rate() const;
    static bool compare(const Node *a, const Node *b);
    double playout();
    double eval_pieces(const Color color_us);

  private:
};

// Nodeの標準出力用
std::ostream &operator<<(std::ostream &os, const Node &node);
std::ostream &operator<<(std::ostream &os, const std::vector<Node *> &nodes);