#pragma once

#include "../common/position.h"
#include "params.h"
#include <vector>

extern int node_cnt;

class Node {
  public:
    Position pos;
    Move move;     // このノードに来た時に実行する指し手
    int depth = 1; // rootからの深さ
    std::vector<Node *> children = {};
    // このノードの勝率。ただし前の手番側から見た勝率である。
    double score = INFTY;
    // このノードが持つ駒の価値。ただし前の手番側から見た価値である。
    // double node_piece_value = 0;
    bool is_illegal = false; // 違法手かどうか

    Node(Position pos, Move &move, int depth = 0);
    ~Node(); // デストラクタ

    double search(double beta);
    double do_playout();
    Node *select_child();
    double ucb(int parent_play_cnt);
    double rate() const;
    double eval_pieces(Color color);
    static bool compare(const Node *a, const Node *b);

  private:
};

// Nodeの標準出力用
std::ostream &operator<<(std::ostream &os, const Node &node);
std::ostream &operator<<(std::ostream &os, const std::vector<Node *> &nodes);