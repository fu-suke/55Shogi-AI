#pragma once

#include "../common/position.h"
#include "params.h"
#include <filesystem>
#include <torch/script.h> // モデルの読み込みと実行に必要なヘッダー
#include <vector>

extern int node_cnt;

class Node {
  public:
    Position pos;
    Move move; // このノードに来た時に実行する指し手
    int depth = 1;
    double score = INFTY;
    bool is_illegal = false;
    std::vector<std::unique_ptr<Node>> children = {};
    std::unique_ptr<Node> best_child;

    Node(Position pos, Move &move, int depth = 0);
    ~Node();

    double playout(Color color);
    double search(double beta);
    static bool compare(const Node *a, const Node *b);
    static bool compare(const std::unique_ptr<Node> &a,
                        const std::unique_ptr<Node> &b);
    static bool Node::compare(const Node &a, const Node &b);
    Node *get_best_child();

    // 評価関数
    double calc_playout_score(
        Color color); // 実際にプレイアウトを行って評価値を計算する
    double eval_pieces(Color color); // 駒の価値をもとに評価値を計算する
    double eval_playout_score(
        Color color); // プレイアウトの機械学習モデルを使って評価値を計算する

    // モデルの読み込み関連
    static bool is_model_loaded;
    static torch::jit::script::Module model;
    std::vector<double> create_board_array();
    torch::Tensor create_tensor();
    static int load_model();
    static const std::vector<double> scale;

  private:
};

// Nodeの標準出力用
std::ostream &operator<<(std::ostream &os, const Node &node);
std::ostream &operator<<(std::ostream &os, const std::vector<Node *> &nodes);
std::ostream &operator<<(std::ostream &os, const std::unique_ptr<Node> &nodes);
std::ostream &operator<<(std::ostream &os,
                         const std::vector<std::unique_ptr<Node>> &nodes);