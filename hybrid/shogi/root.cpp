#include "root.h"
#include <algorithm>
#include <random>

Root::Root() {
    pos = Position();
    // ノードの評価にプレイアウトの機械学習モデルを使う場合、モデルを読み込む
    if (Node::is_model_loaded == false) {
        Node::load_model();
        Node::is_model_loaded = true;
    }
}

Move Root::search() {
    std::unique_ptr<Node> root = std::make_unique<Node>(pos, Move(Move::NONE));
    root->search(INFTY);

    if (root->children.size() == 0) {
        return Move(Move::RESIGN);
    }

    std::sort(
        root->children.begin(), root->children.end(),
        [](const std::unique_ptr<Node> &a, const std::unique_ptr<Node> &b) {
            return Node::compare(*a, *b);
        });

    if (root->children[0]->is_illegal) {
        return Move(Move::RESIGN);
    }

    std::vector<std::unique_ptr<Node>> candidates;
    double best_score = root->children[0]->score;
    double DELTA = 0.01;
    for (int i = 0; i < root->children.size(); ++i) {
        if (root->children[i]->score >= best_score - DELTA) {
            candidates.push_back(std::move(root->children[i]));
        }
    }

    std::cout << "=== SCORE ===\n";
    // それぞれのbest_childのプレイアウトスコアを計算して、元のスコアと平均する
    for (auto &child : candidates) {
        // 機械学習モデルで簡易的に評価したプレイアウトスコア
        double best_child_score =
            child->get_best_child()->eval_playout_score(pos.side_to_move);
        // 実際にプレイアウトを行って評価したプレイアウトスコア
        // double best_child_score =
        // child->get_best_child()->calc_playout_score(pos.side_to_move);
        Node *best_child = child->get_best_child();
        // 元のスコア
        std::cout << child->move << ": " << child->score;
        child->score = child->score * (1 - PLAYOUT_WEIGHT) +
                       best_child_score * PLAYOUT_WEIGHT;
        std::cout << " -> " << child->score << "\n";
    }

    std::sort(
        candidates.begin(), candidates.end(),
        [](const std::unique_ptr<Node> &a, const std::unique_ptr<Node> &b) {
            return Node::compare(*a, *b);
        });

    node_cnt = 0;

    Move best_move = candidates[0]->move;

    return best_move;
}
