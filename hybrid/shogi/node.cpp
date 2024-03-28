#include "node.h"
#include <algorithm>
#include <cmath>
// #include <torch/cuda.h>
#include <vector>
#include <filesystem>

int node_cnt = 0;
torch::jit::script::Module Node::model;
bool Node::is_model_loaded = false;

Node::Node(Position pos, Move &move, int depth) {
    this->pos = pos;
    this->move = move;
    this->depth = depth;
    node_cnt++;

    // NONEはROOTノードの場合にのみ渡される。
    // ROOTノードは既に指し手を実行した後なのでこの処理をスキップする。
    if (!this->move.is_none()) {
        if (!is_safe_move(move, pos.side_to_move, pos)) {
            this->score = -INFTY;
            this->is_illegal = true;
        }
        this->pos.do_move(move);
        // もしこの指し手が一度訪れた盤面だったら千日手対策として違法手にする
        HASH_KEY hash_key = this->pos.get_hash_key();
        if (std::find(visited_hash_keys.begin(), visited_hash_keys.end(),
                      hash_key) != visited_hash_keys.end()) {
            this->score = -INFTY;
            this->is_illegal = true;
        }
    }
}

Node::~Node() {}

double Node::search(double beta) {
    std::vector<Move> move_list = generate_move_list(pos);
    if (move_list.size() == 0) {
        // 指し手がない（＝詰み）の場合は前の手番側の勝ち
        // より浅い詰みを選ぶために深さで割る
        this->score = INFTY / this->depth;
        return this->score;
    }

    // 深さの上限に達していたらこのノードの評価値を返す
    // 「『前の手番』から見たこのノードの評価値」を返すのが適切！！
    if (depth == MAX_DEPTH) {
        this->score = eval_pieces(~pos.side_to_move);
        // rootから見た子の評価値が正になるように符号調整
        if (depth % 2 == 0) {
            this->score -= 1;
        }
        return this->score;
    }
    sort_move_list(move_list, pos);

    // α：子ノードの評価値の最大値
    double alpha = -INFTY;
    double value = -1;
    for (auto move : move_list) {
        std::unique_ptr<Node> child =
            std::make_unique<Node>(pos, move, depth + 1);
        if (child->is_illegal) {
            continue;
        }
        // α更新
        value = child->search(-alpha);
        if (value > alpha) {
            alpha = value;
            // rootノードはbest_nodeを保持しない
            if (!this->move.is_none()) {
                this->best_child = std::move(child);
            }
        }
        // rootノードの場合は子ノードを保存
        if (this->move.is_none()) {
            children.push_back(std::move(child));
        }

        // βカット
        if (alpha > beta) {
            break;
        }
    }

    // このノードの評価値を「敵から見た評価値」に変換する
    this->score = -alpha;
    return this->score;
}

// 駒の価値を考慮した評価値を返す（0.0 ~ 1.0）
double Node::eval_pieces(const Color color) {
    double total_piece_value = 0;
    double piece_value[COLOR_NB] = {0, 0};
    for (Square sq = SQ_ZERO; sq < SQ_NB; ++sq) {
        Piece piece = pos.piece_board[sq];
        if (piece == NO_PIECE) {
            continue;
        }
        piece_value[color_of(piece)] += PIECE_VALUE_MAP.at(type_of(piece));
        total_piece_value += PIECE_VALUE_MAP.at(type_of(piece));
    }
    // 手札にある駒の枚数を数え上げる
    for (int c = BLACK; c < COLOR_NB; ++c) {
        for (Piece pr = RAW_PIECE_BEGIN; pr < RAW_PIECE_NB; ++pr) {
            int num = hand_count(pos.hands[c], (Piece)pr);
            ASSERT(0 <= num && num <= 2, "Number of hand is invalid");
            piece_value[c] += num * PIECE_VALUE_MAP.at(pr);
            total_piece_value += num * PIECE_VALUE_MAP.at(pr);
        }
    }
    // 0.0 ~ 1.0に正規化
    double piece_val = piece_value[color] / total_piece_value;
    ASSERT(0 <= piece_val && piece_val <= 1.0, "eval_pieces() is invalid");

    return piece_val;
}

bool Node::compare(const Node *a, const Node *b) { return b->score < a->score; }

bool Node::compare(const std::unique_ptr<Node> &a,
                   const std::unique_ptr<Node> &b) {
    return Node::compare(*a, *b);
}

bool Node::compare(const Node &a, const Node &b) { return a.score > b.score; }

std::ostream &operator<<(std::ostream &os, const Node &node) {
    os << "move: " << node.move << ", score: " << node.score << std::endl;
    return os;
}

std::ostream &operator<<(std::ostream &os, const std::vector<Node *> &nodes) {
    for (auto node : nodes) {
        os << *node;
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, const std::unique_ptr<Node> &node) {
    os << "move: " << node->move << ", score: " << node->score << std::endl;
    return os;
}

std::ostream &operator<<(std::ostream &os,
                         const std::vector<std::unique_ptr<Node>> &nodes) {
    for (auto &node : nodes) {
        os << *node;
    }
    return os;
}

double Node::playout(Color color) {
    while (true) {
        // 手番側の可能な指し手を1つランダムにとってくる
        std::vector<Move> mlist = generate_move_list(pos);
        // Move move = pos.select_weighted_random_move(mlist);
        Move move = pos.select_random_move(mlist);

        // 合法手がない場合は勝敗がついたということ
        if (move.is_none()) {
            // 自分の手番なら負け、相手の手番なら勝ち
            if (pos.side_to_move == color) {
                return PLAYER_LOSE;
            } else {
                return PLAYER_WIN;
            }
        }

        // 指し手を実行する
        pos.do_move(move);
    }
    // 勝負がついていなければ、駒の枚数に応じた評価値を返す
    // player優勢なら1に近く、opponent優勢なら0に近い値を返す
    return eval_pieces(color);
}

double Node::calc_playout_score(Color color) {
    const int LOOP = 1000;
    const double EPSILON = 1e-3;
    double total_score = 0;
    double total_loop = 0;
    double previous_score = INFTY;
    double current_score = 0;
    Position tmp;

    while (fabs(previous_score - current_score) >= EPSILON) {
        previous_score = current_score;
        for (int i = 0; i < LOOP; ++i) {
            tmp = pos; // 盤面を退避
            total_score += playout(color);
            pos = tmp; // 盤面を復元
        }
        total_loop += LOOP;
        current_score = total_score / total_loop;
        // std::cout << "loop: " << total_loop << ", score: " << current_score
        //           << std::endl;
    }

    return current_score;
}

torch::Tensor Node::create_tensor() {
    std::vector<double> data = create_board_array();
    for (int i = 0; i < scale.size(); ++i) {
        data[i] *= scale[i];
    }

    torch::Tensor tensor = torch::from_blob(data.data(), {38}, torch::kFloat64);
    tensor = tensor.toType(torch::kFloat);
    tensor = tensor.view({1, -1});
    // std::cout << tensor << std::endl;

    return tensor;
}

std::vector<double> Node::create_board_array() {
    std::vector<double> data = {};
    for (Square sq = SQ_ZERO; sq < SQ_NB; ++sq) {
        data.push_back(pos.piece_board[sq]);
    }
    for (Color c = COLOR_ZERO; c < COLOR_NB; ++c) {
        for (Piece pr = RAW_PIECE_BEGIN; pr < RAW_PIECE_NB; ++pr) {
            data.push_back(hand_count(pos.hands[c], pr));
        }
    }
    data.push_back(pos.side_to_move);

    return data;
}

double Node::eval_playout_score(Color color) {
    torch::Tensor t = create_tensor();
    // std::cout << t << std::endl;
    double score = 0;
    try {
        at::Tensor output = model.forward({t}).toTensor();
        score = output.item<double>();
        // std::cout << score << std::endl;
    } catch (const c10::Error &e) {
        std::cout << "An error occurred while running the model: " << e.what()
                  << std::endl;
        return -1;
    }
    // std::cout << score << std::endl;
    if (color == pos.side_to_move) {
        return score;
    } else {
        return 1 - score;
    }
}

Node *Node::get_best_child() {
    if (this->best_child == nullptr) {
        return this;
    } else {
        return this->best_child.get();
    }
}

const std::vector<double> Node::scale = {0.03333333333333333,
                                         0.03333333333333333,
                                         0.03333333333333333,
                                         0.03333333333333333,
                                         0.03333333333333333,
                                         0.03333333333333333,
                                         0.03333333333333333,
                                         0.03333333333333333,
                                         0.03333333333333333,
                                         0.03333333333333333,
                                         0.03333333333333333,
                                         0.03333333333333333,
                                         0.03333333333333333,
                                         0.03333333333333333,
                                         0.03333333333333333,
                                         0.03333333333333333,
                                         0.03333333333333333,
                                         0.03333333333333333,
                                         0.03333333333333333,
                                         0.03333333333333333,
                                         0.03333333333333333,
                                         0.03333333333333333,
                                         0.03333333333333333,
                                         0.03333333333333333,
                                         0.03333333333333333,
                                         0.5,
                                         1.0,
                                         0.5,
                                         0.5,
                                         0.5,
                                         0.5,
                                         0.5,
                                         1.0,
                                         0.5,
                                         0.5,
                                         0.5,
                                         0.5,
                                         1.0};

int Node::load_model() {
    // bool is_cuda_available = torch::cuda::is_available();
    // std::cout << "Is CUDA Available: " << is_cuda_available << std::endl;
    std::string path_to_model = "../shogi/";
    std::string model_name = "playout_model.pt";
    if (!is_model_loaded) {
        try {
            model = torch::jit::load(path_to_model + model_name);
            model.eval();
            is_model_loaded = true;
            std::cout << "Successfully loaded the model: " << model_name
                      << std::endl;
            return 0;
        } catch (const c10::Error &) {
            ;
        }
        std::cout<< "Current path: " << std::filesystem::current_path() << std::endl;
        std::cout << "An error occurred while loading the model: " << model_name
                  << std::endl;
        return -1;
    }
    return 0;
}