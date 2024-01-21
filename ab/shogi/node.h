#pragma once

#include "../common/position.h"
#include "params.h"
#include <vector>

extern int node_cnt;

class Node {
  public:
    Position pos;
    Move move; // このノードに来た時に実行する指し手
    bool is_illegal = false; // ↑が違法手かどうか
    int depth = 0;           // rootからの深さ
    std::vector<Node *> children = {};
    // このノードの勝率。ただし前の手番側から見た勝率である。
    double score = INFTY;

    Node(Position pos, Move &move, int depth = 0);
    ~Node();

    double search(double beta);
    double eval_pieces(Color color);
    static bool compare(const Node *a, const Node *b);

  private:
};

// Nodeの標準出力用
std::ostream &operator<<(std::ostream &os, const Node &node);
std::ostream &operator<<(std::ostream &os, const std::vector<Node *> &nodes);