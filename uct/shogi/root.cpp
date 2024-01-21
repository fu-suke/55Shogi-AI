#include "root.h"
#include <algorithm>

Root::Root() { pos = Position(); }

Move Root::search() {
    Move m = Move(Move::RESIGN);
    Node *root = new Node(pos.side_to_move, pos, m, 0);
    std::vector<Move> move_list = generate_move_list(pos);
    int loop = 0;
    for (auto move : move_list) {
        if (move.is_drop()) {
            loop += UCT_PER_DROP;
        } else {
            loop += UCT_PER_MOVE;
        }
    }
    // このノードは既にプレイされているものとする
    root->play_cnt = 1;
    for (int i = 0; i < loop; ++i) {
        root->search();
    }

    // 子ノードがない場合は投了
    if (root->children.empty()) {
        return Move(Move::RESIGN);
    }

    // 評価値の高い順にソート
    std::sort(root->children.begin(), root->children.end(), Node::compare);

    // 最も評価の高いノードが違法手の場合（＝合法手がない場合）も投了
    Node *best_node = root->children[0];
    if (best_node->is_illegal) {
        return Move(Move::RESIGN);
    }
    Move best_move = best_node->move;

    // 表示
    // std::cout << "=====SCORE=====" << std::endl;
    // std::cout << root->children << std::endl;
    // std::cout << std::endl;
    // std::cout << "=====BEST'S CHILDREN=====" << std::endl;
    // std::sort(best_node->children.begin(), best_node->children.end(),
    //           Node::compare);
    // std::cout << best_node->children << std::endl;

    // std::cout << "max_depth: " << max_depth << std::endl;
    // std::cout << "node_cnt: " << node_cnt << std::endl;

    std::cout << "node_cnt: " << node_cnt << std::endl;

    delete root;

    return best_move;
}