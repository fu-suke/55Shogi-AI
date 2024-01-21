#include "root.h"
#include <algorithm>
#include <random>

Root::Root() {
    pos = Position();
    // srand(time(nullptr));
}

Move Root::search() {
    // Node *root = new Node(pos, Move(Move::NONE));
    std::unique_ptr<Node> root = std::make_unique<Node>(pos, Move(Move::NONE));
    root->search(INFTY);
    // 合法手がない場合は投了
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
    std::cout << "=== SCORE ===\n";
    for (auto &child : root->children) {
        std::cout << child->move << ": " << child->score << std::endl;
    }

    std::vector<std::unique_ptr<Node>> candidates;
    double best_score = root->children[0]->score;
    double DELTA = 0.01;
    for (int i = 0; i < root->children.size(); ++i) {
        if (root->children[i]->score >= best_score - DELTA) {
            candidates.push_back(std::move(root->children[i]));
        }
    }
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    std::uniform_int_distribution<int> distribution(0, candidates.size() - 1);
    int idx = distribution(generator);
    Move best_move = candidates[idx]->move;

    node_cnt = 0;

    // std::cout << " === SCORE === " << std::endl;
    // std::cout << root->children << std::endl;

    // std::vector<double> previous_score = {};
    // previous_score.reserve(root->children.size());

    // for (auto &child : root->children) {
    //     if (child != nullptr) {
    //         previous_score.push_back(child->score);
    //     }
    // }

    // Node *leaf;
    // for (auto &child : root->children) {
    //     leaf = child->get_best_child();
    //     if (leaf == nullptr) {
    //         std::cout << "leaf is nullptr" << std::endl;
    //         return Move(Move::RESIGN);
    //     }
    //     child->score = leaf->eval_playout_score(pos.side_to_move);
    // }

    // 評価値がどのように変化したかを表示
    // std::cout << "=== SCORE ===\n";
    // std::cout << std::fixed
    //           << std::setprecision(4); // 小数点以下の桁数を4に設定
    // for (int i = 0; i < root->children.size(); ++i) {
    //     std::cout << root->children[i]->move << " " << previous_score[i]
    //               << " -> " << root->children[i]->score << std::endl;
    // }

    // 評価値の高い順にソート
    // std::sort(root->children.begin(), root->children.end(), Node::compare);

    // std::cout << "=== SCORE ===\n";
    // std::cout << root->children << std::endl;

    return best_move;
}

// Move Root::search() {
//     std::vector<Move> move_list = generate_move_list(pos);
//     if (move_list.size() == 0) {
//         return Move(Move::RESIGN);
//     }
//     std::vector<std::unique_ptr<Node>> children;
//     for (auto move : move_list) {
//         std::unique_ptr<Node> child = std::make_unique<Node>(pos, move, 0);
//         if (child->is_illegal) {
//             continue;
//         }
//         children.push_back(std::move(child));
//     }
//     if (children.size() == 0) {
//         return Move(Move::RESIGN);
//     }
//     for (auto &child : children) {
//         child->score = child->eval_playout_score(pos.side_to_move);
//     }
//     std::sort(children.begin(), children.end(), Node::compare);
//     std::cout << "=== SCORE ===\n";
//     std::cout << std::fixed << std::setprecision(4);
//     for (int i = 0; i < children.size(); ++i) {
//         std::cout << children[i]->move << " " << children[i]->score
//                   << std::endl;
//     }
//     double best_score = children[0]->score;
//     std::vector<std::unique_ptr<Node>> candidates;
//     double DELTA = 0.00;
//     for (int i = 0; i < children.size(); ++i) {
//         if (children[i]->score >= best_score - DELTA) {
//             candidates.push_back(std::move(children[i]));
//         }
//     }
//     unsigned seed =
//     std::chrono::system_clock::now().time_since_epoch().count();
//     std::default_random_engine generator(seed);
//     std::uniform_int_distribution<int> distribution(0, candidates.size() -
//     1); int idx = distribution(generator); Move best_move =
//     candidates[idx]->move; node_cnt = 0;
//     // std::cout << "=== BEST MOVE ===\n";
//     // std::cout << best_move << std::endl;
//     return best_move;
// }