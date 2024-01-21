#include "root.h"
#include <algorithm>

Root::Root() {
    pos = Position();
    srand(time(nullptr));
}

Move Root::search() {
    Move m = Move(Move::NONE);
    Node *root = new Node(pos, m);
    root->search(INFTY);

    // 合法手がない場合は投了
    if (root->children.size() == 0) {
        return Move(Move::RESIGN);
    }

    // 評価値の高い順にソート
    std::sort(root->children.begin(), root->children.end(), Node::compare);

    // 最も評価の高いノードが違法手の場合（＝合法手がない場合）も投了
    Node *best_node = root->children[0];
    if (best_node->is_illegal) {
        return Move(Move::RESIGN);
    }

    // best_nodeと評価値が同じやつを探す
    std::vector<Node *> candidates;
    for (Node *node : root->children) {
        if (node->score == best_node->score) {
            candidates.push_back(node);
        }
    }

    // その中からランダムに選ぶ
    int idx = rand() % candidates.size();
    // int idx = 0;
    best_node = candidates[idx];

    Move best_move = best_node->move;

    // std::cout << node_cnt << std::endl;

    delete root;

    return best_move;
}
