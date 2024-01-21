#include "root.h"
#include <algorithm>
#include <random>

Root::Root() {
    pos = Position();
    if (Node::is_model_loaded == false) {
        Node::load_model();
    }
    // srand(time(nullptr));
}

// Move Root::search() {
//     std::unique_ptr<Node> root = std::make_unique<Node>(pos,
//     Move(Move::NONE)); root->search(INFTY);
//     // 合法手がない場合は投了
//     if (root->children.size() == 0) {
//         return Move(Move::RESIGN);
//     }

//     std::sort(
//         root->children.begin(), root->children.end(),
//         [](const std::unique_ptr<Node> &a, const std::unique_ptr<Node> &b) {
//             return Node::compare(*a, *b);
//         });

//     if (root->children[0]->is_illegal) {
//         return Move(Move::RESIGN);
//     }
//     std::cout << "=== SCORE ===\n";
//     for (auto &child : root->children) {
//         std::cout << child->move << ": " << child->score << std::endl;
//     }

//     std::vector<std::unique_ptr<Node>> candidates;
//     double best_score = root->children[0]->score;
//     double DELTA = 0.01;
//     for (int i = 0; i < root->children.size(); ++i) {
//         if (root->children[i]->score >= best_score - DELTA) {
//             candidates.push_back(std::move(root->children[i]));
//         }
//     }
//     unsigned seed =
//     std::chrono::system_clock::now().time_since_epoch().count();
//     std::default_random_engine generator(seed);
//     std::uniform_int_distribution<int> distribution(0, candidates.size() -
//     1); int idx = distribution(generator); Move best_move =
//     candidates[idx]->move;

//     node_cnt = 0;

//     return best_move;
// }
